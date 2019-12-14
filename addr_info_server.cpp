//
// Created by maslov on 13.12.19.
//

#include "addr_info_server.h"

#include <cassert>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>

//#include <iostream> //DEBUG:


void addr_info_connection::sock_handle(int msk) {
    assert(msk & EPOLLIN);
    if (!alive)
        throw server_error("invoke dead connection");
    string arg;

    reload:
    while (pos < len) {
        if (buffer[pos] == '\n' || buffer[pos] == '\0') {
            ++pos;

            if (arg.empty()) {
                disconnect();
            }

            {
                std::lock_guard<mutex> lg(owner->work_in);
                owner->jobs.push(std::make_pair(this, arg));
                owner->cv.notify_one();
            }
            return;
        }
        arg.push_back(buffer[pos++]);
    }

    if (pos == len && len != 0) {
        len = sock.read_c(buffer, BUFSIZE);
        pos = 0;
        goto reload;
    }
}

addr_info_connection::addr_info_connection(addr_info_server *owner, uniq_fd &&fd)
        : owner(owner),
          stamp(time(NULL)),
          sock(std::move(fd), owner->executor, [this](int sig) { this->sock_handle(sig); }, EPOLLIN) {}

void addr_info_connection::disconnect() {
    alive = false;
    owner->deleted_connections.insert(this);
}

addr_info_connection::~addr_info_connection() {
    disconnect();
}

addr_info_server::addr_info_server(processor *executor, uint16_t port)
        : executor(executor),
          port(port),
          sock(uniq_fd(socket(AF_INET, SOCK_STREAM, 0)), executor,
               [this](int sig) { this->sock_handle(sig); }, EPOLLIN) {
    int fd = sock.provide_fd();
    if (fd < 0) {
        throw server_error("socket create");
    }
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = 0;
    local_addr.sin_port = htons(port);
    bind(fd, reinterpret_cast<sockaddr const *>(&local_addr), sizeof(local_addr));
    listen(fd, SOMAXCONN);

    struct itimerspec new_value{};
    int max_exp, timerfd;
    struct timespec now{};
    uint64_t exp, tot_exp;
    ssize_t s;

    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
        throw server_error("clock_gettime system error");

    new_value.it_value.tv_sec = now.tv_sec;
    new_value.it_value.tv_nsec = now.tv_nsec;
    new_value.it_interval.tv_nsec = 10000; /// 10 microseconds
    //new_value.it_interval.tv_sec = 1; //DEBUG:

    timerfd = timerfd_create(CLOCK_REALTIME, 0);
    if (timerfd == -1)
        throw server_error("timerfd_create system error");

    if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
        throw server_error("timerfd_settime system error");

    timer = std::make_unique<observed_socket>(uniq_fd(timerfd), executor, [this](int msk) {
        std::lock_guard<mutex> lg(work_out);
        //std::cerr << jobs.size() << ' ' << results.size() << std::endl; //DEBUG:
        clean_old_connections(msk);
        response_all(msk);
        cv.notify_all();
        return;
    }, EPOLLIN);

    for (size_t i = 0; i < WORKERS; ++i)
        workers[i] = std::make_unique<thread>([this]() -> void {
            while (WORKS.load()) {
                pair<addr_info_connection *, string> arg;
                {
                    std::unique_lock<mutex> lg(work_in);
                    cv.wait(lg, [this]() -> bool { return !jobs.empty() || !WORKS.load(); });
                    if (!WORKS.load())
                        return;
                    arg = jobs.front();
                    jobs.pop();
                }
                try {
                    arg.second = get_addr_info(arg.second);
                } catch (const an_error& e) {
                    arg.second = "Server error: " + e.get_reason();
                }
                {
                    std::lock_guard<mutex> lg(work_out);
                    results.push(arg);
                }
            }
        });
}

void addr_info_server::add_connection(int fd) {
    unique_ptr<addr_info_connection> connect = std::make_unique<addr_info_connection>(this, uniq_fd(fd));
    connections.insert(std::make_pair(connect.get(), std::move(connect)));
}

void addr_info_server::sock_handle(int msk) {
    assert(msk & EPOLLIN);
    int fd = sock.provide_fd();
    int conn_fd = accept(fd, nullptr, nullptr);
    add_connection(conn_fd);
}

void addr_info_server::clean_old_connections(int) {
    while (!deleted_connections.empty()) {
        addr_info_connection *t = *deleted_connections.begin();
        connections.erase(t);
        deleted_connections.erase(deleted_connections.begin());
    }
}

void addr_info_server::response_all(int) {
    while (!results.empty()) {
        addr_info_connection *t = results.front().first;
        if (connections.find(t) != connections.end()) {
            connections[t]->sock.write_c(results.front().second.c_str(), results.front().second.size());
        }
        results.pop();
    }
}

addr_info_server::~addr_info_server() {
    WORKS.store(false);
    {
        std::lock_guard<mutex> lg(work_in);
        for (size_t i = 0; i < WORKERS; ++i)
            kill(workers[i]->native_handle(), SIGTERM);
        cv.notify_all();
    }
    for (size_t i = 0; i < WORKERS; ++i)
        workers[i]->join();
}

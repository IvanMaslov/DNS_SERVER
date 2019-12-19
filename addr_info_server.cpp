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

//#include <iostream> //DEBUG:cerr

#include <functional>

using std::bind;

void addr_info_connection::sock_handle() {
    if (!alive)
        throw server_error("invoke dead connection");

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
                arg.clear();
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

    disconnect();
}

addr_info_connection::addr_info_connection(addr_info_server *owner, uniq_fd &&fd)
        : owner(owner),
          stamp(time(NULL)),
          sock(std::move(fd), owner->executor, [this]() { this->sock_handle(); }) {}

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
          sock(uniq_fd(bind(&fd_fabric::socket_fd, port)), executor,
               [this]() { this->sock_handle(); }) {

    timer = std::make_unique<observed_fd>(uniq_fd(fd_fabric::timer_fd()), executor, [this]() {
        std::lock_guard<mutex> lg(work_out);
        //std::cerr << jobs.size() << ' ' << results.size() << std::endl; //DEBUG:cerr
        clean_old_connections();
        response_all();
        cv.notify_all();
        return;
    });

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
                } catch (const an_error &e) {
                    arg.second = "Server Internal Error\n";
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

void addr_info_server::sock_handle() {
    int fd = sock.fd;
    int conn_fd = accept(fd, nullptr, nullptr);
    add_connection(conn_fd);
}

void addr_info_server::clean_old_connections() {
    //if(deleted_connections.size()) std::cerr << "clear: " << deleted_connections.size() << std::endl; //DEBUG:cerr
    while (!deleted_connections.empty()) {
        addr_info_connection *t = *deleted_connections.begin();
        connections.erase(t);
        deleted_connections.erase(deleted_connections.begin());
    }
}

void addr_info_server::response_all() {
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

//
// Created by maslov on 13.12.19.
//

#include "echo_server.h"

#include <cassert>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <netinet/in.h>

void echo_connection::sock_handle(int msk) {
    assert(msk & EPOLLIN);
    if (!alive)
        throw server_error("invoke dead connection");
    size_t readed = 0;
    bool acted = false;
    while ((readed = sock.read_c(buffer, BUFSIZE)) != 0) {
        sock.write_c(buffer, readed);
        acted = true;
    }
    if (!acted) {
        disconnect();
    }
}

echo_connection::echo_connection(echo_server *owner, uniq_fd &&fd)
        : owner(owner),
          stamp(time(NULL)),
          sock(std::move(fd), owner->executor, [this](int sig) { this->sock_handle(sig); }, EPOLLIN) {}

void echo_connection::disconnect() {
    alive = false;
    owner->deleted_connections.insert(this);
}

echo_connection::~echo_connection() {
    disconnect();
}

echo_server::echo_server(processor *executor, uint16_t port)
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
    new_value.it_interval.tv_nsec = 100000;

    timerfd = timerfd_create(CLOCK_REALTIME, 0);
    if (timerfd == -1)
        throw server_error("timerfd_create system error");

    if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
        throw server_error("timerfd_settime system error");

    timer = std::make_unique<observed_socket>(uniq_fd(timerfd), executor, [this](int msk) {
        clean_old_connections(msk);
        return;
    }, EPOLLIN);
}

void echo_server::add_connection(int fd) {
    unique_ptr<echo_connection> connect = std::make_unique<echo_connection>(this, uniq_fd(fd));
    connections.insert(std::make_pair(connect.get(), std::move(connect)));
}

void echo_server::sock_handle(int msk) {
    assert(msk & EPOLLIN);
    int fd = sock.provide_fd();
    int conn_fd = accept(fd, nullptr, nullptr);
    add_connection(conn_fd);
}

void echo_server::clean_old_connections(int) {
    while (!deleted_connections.empty()) {
        echo_connection *t = *deleted_connections.begin();
        connections.erase(t);
        deleted_connections.erase(deleted_connections.begin());
    }
}

echo_server::~echo_server() = default;

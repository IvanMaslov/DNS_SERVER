//
// Created by maslov on 13.12.19.
//

#include "echo_server.h"

#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>

void echo_connection::sock_handle(int msk) {
    assert(msk & EPOLLIN);
    size_t readed = 0;
    while ((readed = sock.read_c(buffer, BUFSIZE)) != 0) {
        sock.write_c(buffer, readed);
    }
}

echo_connection::echo_connection(echo_server *owner, uniq_fd &&fd)
        : owner(owner),
          sock(std::move(fd), owner->executor, [this](int sig) { this->sock_handle(sig); }, EPOLLIN) {}

echo_connection::~echo_connection() {
    alive = false;
    owner->deleted_connections.insert(this);
}

echo_server::echo_server(processor *executor, uint16_t port)
        : executor(executor),
          port(port),
          sock(uniq_fd(socket(AF_INET, SOCK_STREAM, 0)), executor,
               [this](int sig) { this->sock_handle(sig); }, EPOLLIN) {
    //TODO: address bind listen
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
    }
}

echo_server::~echo_server() = default;

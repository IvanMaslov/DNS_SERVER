//
// Created by maslov on 12.12.19.
//

#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "uniq_fd.h"

// typedef std::function<void()> callback_t;

class base_socket {
private:
    uniq_fd fd;
public:
    base_socket();
    explicit base_socket(uniq_fd&&);

    ~base_socket();

    base_socket(base_socket const&) = delete;
    base_socket operator=(base_socket const&) = delete;

    size_t read_c(void*, size_t);
    size_t write_c(const void*, size_t);
};

// template<> void std::swap<base_socket>(base_socket&, base_socket&) = delete;

#endif //SERVER_SOCKET_H

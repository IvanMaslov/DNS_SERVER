//
// Created by maslov on 12.12.19.
//

#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "uniq_fd.h"

class base_socket {
protected:
    uniq_fd fd;
public:
    base_socket();
    explicit base_socket(uniq_fd&&);
    base_socket(base_socket &&) noexcept;

    ~base_socket();

    base_socket(base_socket const&) = delete;
    base_socket operator=(base_socket const&) = delete;

    size_t read_c(void*, size_t);
    size_t write_c(const void*, size_t);

    int provide_fd() const noexcept;
};

// template<> void std::swap<base_socket>(base_socket&, base_socket&) = delete;

#endif //SERVER_SOCKET_H

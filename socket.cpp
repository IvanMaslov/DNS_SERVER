//
// Created by maslov on 12.12.19.
//

#include "socket.h"

#include <unistd.h>
#include <sys/socket.h>

base_socket::~base_socket() {}

base_socket::base_socket(uniq_fd && fd) : fd(fd.fd) {}

size_t base_socket::read_c(void * src, size_t len) {
    return read(fd.fd, src, len);
}

size_t base_socket::write_c(const void * src, size_t len) {
    return write(fd.fd, src, len);
}

base_socket::base_socket() = default;

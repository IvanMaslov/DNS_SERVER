//
// Created by maslov on 12.12.19.
//

#include "uniq_fd.h"

#include <unistd.h>

uniq_fd::uniq_fd() {}

uniq_fd::uniq_fd(int fd) : fd(fd) {}

uniq_fd::~uniq_fd() { close(fd); }

uniq_fd::uniq_fd(uniq_fd && arg) noexcept : fd(arg.fd) {
    arg.fd = -1;
}

size_t uniq_fd::write_c(const void *src, size_t len) {
    return write(fd, src, len);
}

size_t uniq_fd::read_c(void *src, size_t len) {
    return read(fd, src, len);
}

uniq_fd::uniq_fd(function<int(void)>&& creator) { fd = creator(); }

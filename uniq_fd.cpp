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

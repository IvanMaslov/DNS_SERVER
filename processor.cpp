
//
// Created by maslov on 13.12.19.
//

#include "processor.h"

#include <utility>

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


processor::processor() : polling_fd(fd_fabric::epoll_fd()) {

    breaker = std::make_unique<observed_fd>(uniq_fd(fd_fabric::signal_fd()), this, [this]() {
        struct signalfd_siginfo fdsi{};
        ssize_t s = this->breaker->read_c(&fdsi, sizeof(struct signalfd_siginfo));
        if (s != sizeof(struct signalfd_siginfo))
            return;
        if (fdsi.ssi_signo == SIGINT) {
            executing = false;
        } else if (fdsi.ssi_signo == SIGQUIT) {
            exit(EXIT_SUCCESS);
        } else {
            exit(EXIT_FAILURE);
        }
        return;
    });
}

void processor::execute() {
    executing = true;
    epoll_event pevents[EPOLL_PER_TIME];
    while (executing) {
        int ready = polling_fd.wait(pevents, EPOLL_PER_TIME, EPOLL_TIMEOUT);
        if (ready == -1) {
            if (executing)
                throw exec_error("executing ready fail");
            return;
        } else {
            for (int i = 0; i < ready; i++) {
                reinterpret_cast<observed_fd *> (pevents[i].data.ptr)->callback();
            }
        }
    }
}

processor::~processor() = default;

void processor::epoll_unq_fd::add(observed_fd *sock, uint32_t msk) const {
    epoll_event ev{};
    ev.events = msk;
    ev.data.fd = sock->fd;
    ev.data.ptr = reinterpret_cast<void *>(sock);
    if (epoll_ctl(fd, EPOLL_CTL_ADD, sock->fd, &ev) != 0)
        throw exec_error("epoll add");
}

void processor::epoll_unq_fd::remove(observed_fd *sock) const {
    if (epoll_ctl(fd, EPOLL_CTL_DEL, sock->fd, NULL) != 0) {
        throw exec_error("epoll remove");
    }
}

int processor::epoll_unq_fd::wait(epoll_event * events, size_t len, size_t timeout) const {
    return epoll_wait(fd, events, len, timeout);
}

//DEBUG: force_invoke
void processor::force_invoke(observed_fd *t) {
    t->callback();
}

observed_fd::observed_fd(uniq_fd &&fd, processor *proc, std::function<void(void)> callback, uint32_t msk)
        : uniq_fd(std::move(fd)), parent(proc), callback(std::move(callback)) {
    parent->polling_fd.add(this, msk);
}

observed_fd::~observed_fd() {
    parent->polling_fd.remove(this);
}

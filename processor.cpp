
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


processor::processor() : polling_fd(epoll_create(0xCAFE)) {
    if (polling_fd.fd < 0) {
        throw exec_error("creating processor");
    }

    sigset_t mask;
    int sfd;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        throw exec_error("breaker 1");

    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1)
        throw exec_error("breaker 2");

    breaker = std::make_unique<observed_fd>(uniq_fd(sfd), this, [this, sfd](int msk) {
        struct signalfd_siginfo fdsi{};
        ssize_t s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
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
    }, EPOLLIN);
}

void processor::execute() {
    executing = true;
    epoll_event pevents[EPOLL_PER_TIME];
    while (executing) {
        int ready = epoll_wait(polling_fd.fd, pevents, EPOLL_PER_TIME, EPOLL_TIMEOUT);
        if (ready == -1) {
            if (executing)
                throw exec_error("executing ready fail");
            return;
        } else {
            for (int i = 0; i < ready; i++) {
                reinterpret_cast<observed_fd *> (pevents[i].data.ptr)->callback(pevents[i].events);
            }
        }
    }
}

processor::~processor() = default;

void processor::add(observed_fd *sock) {
    epoll_event ev{};
    ev.events = sock->epoll_mask;
    ev.data.fd = sock->fd;
    ev.data.ptr = reinterpret_cast<void *>(sock);
    if (epoll_ctl(polling_fd.fd, EPOLL_CTL_ADD, sock->fd, &ev) != 0)
        throw exec_error("epoll add");
}

void processor::remove(observed_fd *sock) {
    if (epoll_ctl(polling_fd.fd, EPOLL_CTL_DEL, sock->fd, NULL) != 0) {
        throw exec_error("epoll remove");
    }

}

//DEBUG: force_invoke
void processor::force_invoke(observed_fd *t, int arg) {
    t->callback(arg);
}

observed_fd::observed_fd(uniq_fd &&fd, processor *proc, std::function<void(int)> callback, uint32_t msk)
        : uniq_fd(std::move(fd)), parent(proc), callback(std::move(callback)), epoll_mask(msk) {
    parent->add(this);
}

observed_fd::~observed_fd() {
    parent->remove(this);
}

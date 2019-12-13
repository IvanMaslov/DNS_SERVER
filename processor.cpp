
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

    breaker = std::make_unique<observed_socket>(uniq_fd(sfd), this, [this, sfd](int msk) {
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
    epoll_event pevents[EPOLL_PER_TIME];
    while (executing) {
        int ready = epoll_wait(polling_fd.fd, pevents, EPOLL_PER_TIME, EPOLL_TIMEOUT);
        if (ready == -1) {
            if (executing)
                throw exec_error("executing ready fail");
            return;
        } else {
            for (int i = 0; i < ready; i++) {
                reinterpret_cast<observed_socket *> (pevents[i].data.ptr)->callback(pevents[i].events);
            }
        }
    }
}

processor::~processor() = default;

void processor::add(observed_socket *sock) {
    epoll_event ev{};
    ev.events = sock->epoll_mask;
    ev.data.fd = sock->fd.fd;
    ev.data.ptr = reinterpret_cast<void *>(sock);
    if (epoll_ctl(polling_fd.fd, EPOLL_CTL_ADD, sock->fd.fd, &ev) != 0)
        throw exec_error("epoll add");
}

void processor::remove(observed_socket *sock) {
    if (epoll_ctl(polling_fd.fd, EPOLL_CTL_DEL, sock->fd.fd, NULL) != 0) {
        throw exec_error("epoll remove");
    }

}

//DEBUG: force_invoke
void processor::force_invoke(observed_socket *t, int arg) {
    t->callback(arg);
}

observed_socket::observed_socket(uniq_fd &&fd, processor *proc, std::function<void(int)> callback, uint32_t msk)
        : base_socket(std::move(fd)), parent(proc), callback(std::move(callback)), epoll_mask(msk) {
    parent->add(this);
}

observed_socket::~observed_socket() {
    parent->remove(this);
}

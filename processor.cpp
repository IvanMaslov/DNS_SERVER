//
// Created by maslov on 13.12.19.
//

#include "processor.h"


processor::processor() : polling_fd(epoll_create(0xCAFE)) {
    if (polling_fd.fd < 0) {
        throw exec_error("creating processor");
    }
    //TODO: add ctrl+c
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
                dynamic_cast<observed_socket *> (pevents[i].data.ptr)->invoke(pevents[i].events);
            }
        }
    }
}

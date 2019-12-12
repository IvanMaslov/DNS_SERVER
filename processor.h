//
// Created by maslov on 13.12.19.
//

#ifndef SERVER_PROCESSOR_H
#define SERVER_PROCESSOR_H

#include "socket.h"
#include "server_utils.h"


#include <sys/epoll.h>

class observed_socket;

class processor {
    friend class observed_socket;
private:
    const uniq_fd polling_fd;

    void add(observed_socket const&&, int);
    void remove(observed_socket const&&, int);

    static const size_t EPOLL_PER_TIME = 1;
    static const size_t EPOLL_TIMEOUT = 10;
    inline static volatile bool executing = true;
public:
    processor();
    ~processor();
    void execute();
};

class observed_socket : public base_socket {
private:
    processor* parent;
public:
    observed_socket(processor*, callback_t);
};



#endif //SERVER_PROCESSOR_H

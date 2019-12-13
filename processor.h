//
// Created by maslov on 13.12.19.
//

#ifndef SERVER_PROCESSOR_H
#define SERVER_PROCESSOR_H

#include "socket.h"
#include "server_utils.h"


#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <memory>

class observed_socket;
class processor;

using std::unique_ptr;

class processor {
    friend class observed_socket;
private:
    const uniq_fd polling_fd;

    void add(observed_socket*);
    void remove(observed_socket*);

    unique_ptr<observed_socket> breaker;

    static const size_t EPOLL_PER_TIME = 1;
    static const size_t EPOLL_TIMEOUT = 10;
    inline static volatile bool executing = true;
public:
    processor();
    ~processor();
    void execute();

    //DEBUG: force_invoke
    void force_invoke(observed_socket* t, int arg);
};

class observed_socket : public base_socket {
    friend class processor;
private:
    processor* parent;
    std::function<void(int)> callback;
    uint32_t epoll_mask;
public:
    observed_socket(uniq_fd &&, processor*, std::function<void(int)>, uint32_t);
    virtual ~observed_socket();
};



#endif //SERVER_PROCESSOR_H

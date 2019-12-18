//
// Created by maslov on 13.12.19.
//

#ifndef SERVER_PROCESSOR_H
#define SERVER_PROCESSOR_H

#include "uniq_fd.h"
#include "server_utils.h"


#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <memory>

class observed_fd;
class processor;

using std::unique_ptr;

class processor {
    friend class observed_fd;
private:
    const uniq_fd polling_fd;

    void add(observed_fd*);
    void remove(observed_fd*);

    unique_ptr<observed_fd> breaker;

    static const size_t EPOLL_PER_TIME = 1;
    static const size_t EPOLL_TIMEOUT = 10;
    inline static volatile bool executing = true;
public:
    processor();
    ~processor();
    void execute();

    //DEBUG: force_invoke
    void force_invoke(observed_fd* t, int arg);
};

class observed_fd : public uniq_fd {
    friend class processor;
private:
    processor* parent;
    std::function<void(int)> callback;
    uint32_t epoll_mask;
public:
    observed_fd(uniq_fd &&, processor*, std::function<void(int)>, uint32_t);
    virtual ~observed_fd();
};



#endif //SERVER_PROCESSOR_H

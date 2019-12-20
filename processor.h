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
using std::move;

class processor {
    friend class observed_fd;

private:
    class epoll_unq_fd : public uniq_fd {
    public:
        using uniq_fd::uniq_fd;

        void add(observed_fd *, uint32_t) const;

        void remove(observed_fd *) const;

        int wait(epoll_event *, size_t, size_t) const;
    };

    const epoll_unq_fd polling_fd;


    unique_ptr<observed_fd> breaker;

    static const size_t EPOLL_PER_TIME = 1;
    static const size_t EPOLL_TIMEOUT = 10;
    inline static volatile bool executing = true;
public:
    processor();

    ~processor();

    void execute();

    //DEBUG: force_invoke
    void force_invoke(observed_fd *t);
};

class observed_fd : public uniq_fd {
    friend class processor;

private:
    processor *parent;
    std::function<void(void)> callback;
public:
    observed_fd(uniq_fd &&, processor *, std::function<void(void)>, uint32_t msk = EPOLLIN);

    virtual ~observed_fd();
};


#endif //SERVER_PROCESSOR_H

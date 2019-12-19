//
// Created by maslov on 13.12.19.
//

#ifndef SERVER_ADDR_INFO_SERVER_H
#define SERVER_ADDR_INFO_SERVER_H

#include "processor.h"

#include <map>
#include <set>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

using std::map;
using std::set;
using std::queue;
using std::string;
using std::unique_ptr;
using std::pair;
using std::thread;
using std::mutex;
using std::condition_variable;

class addr_info_connection;

class addr_info_server {
    friend class addr_info_connection;
private:
    uint16_t port;
    processor* executor;
    observed_fd sock;
    void sock_handle();

    unique_ptr<observed_fd> timer;
public:
    explicit addr_info_server(processor*, uint16_t);
    ~addr_info_server();

private:
    map<addr_info_connection*, unique_ptr<addr_info_connection>> connections;
    set<addr_info_connection*> deleted_connections;
    void add_connection(int);
    void clean_old_connections(); /// use lock under timer
    void response_all(); /// use lock under timer

private:
    static inline const size_t WORKERS = 3;
    static inline volatile std::atomic_bool WORKS = true;

    queue<pair<addr_info_connection*, string>> jobs;
    queue<pair<addr_info_connection*, string>> results;
    unique_ptr<thread> workers[WORKERS];
    mutex work_in, work_out;
    condition_variable cv;
};

class addr_info_connection {
    friend class addr_info_server;
private:
    addr_info_server* owner;
    observed_fd sock;
    void sock_handle();

    const time_t stamp;
    volatile bool alive = true;
    void disconnect();

public:
    addr_info_connection(addr_info_server*, uniq_fd &&);
    ~addr_info_connection();
private:
    inline static const size_t BUFSIZE = 32;

    char buffer[BUFSIZE];
    size_t pos = -1;
    size_t len = -1;
};


#endif //SERVER_ADDR_INFO_SERVER_H

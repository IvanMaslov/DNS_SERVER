//
// Created by maslov on 13.12.19.
//

#ifndef SERVER_ECHO_SERVER_H
#define SERVER_ECHO_SERVER_H

#include "processor.h"

#include <map>
#include <set>
#include <memory>

using std::map;
using std::set;
using std::unique_ptr;

class echo_connection;
class echo_server;


class echo_server {
private:
    processor* executor;
    observed_socket sock;
    void sock_handle(int);

public:
    echo_server(processor*);
    ~echo_server();

private:
    map<echo_connection*, unique_ptr<echo_connection>> connections;
    set<echo_connection*> deleted_connections;
    void add_connection(int);
    void clean_old_connections(int);

};

class echo_connection {
private:
    echo_server* owner;
    observed_socket sock;
    void sock_handle(int);

public:
    echo_connection(echo_server*, uniq_fd const &&);
    ~echo_connection();

private:
    inline static const size_t BUFSIZE = 1024;

    char buffer[BUFSIZE];
    size_t pos = 0;
    size_t len = 0;
};


#endif //SERVER_ECHO_SERVER_H

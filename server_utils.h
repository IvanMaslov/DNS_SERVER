//
// Created by maslov on 12.12.19.
//

#ifndef SERVER_SERVER_UTILS_H
#define SERVER_SERVER_UTILS_H

#include <string>

using std::string;
using std::exception;

string get_addr_info_name(const string &);

string get_addr_info(const string &);

class an_error : exception {
public:
    an_error();

    explicit an_error(const string &);

    string get_reason() const;

private:
    string reason;
};

class get_addr_info_error : public an_error {
public:
    get_addr_info_error() : an_error() {}

    explicit get_addr_info_error(const string &arg) : an_error(arg) {}

    ~get_addr_info_error() override = default;
};

class server_error : public an_error {
public:
    server_error() : an_error() {}

    explicit server_error(const string &arg) : an_error(arg) {}

    ~server_error() override = default;
};

class exec_error : public an_error {
public:
    exec_error() : an_error() {}

    explicit exec_error(const string &arg) : an_error(arg) {}

    ~exec_error() override = default;
};


namespace fd_fabric {

    class fd_error : public an_error {
    public:
        enum Type {
            UNDEFINED, TIMER_FD, SOCKET_FD, EPOLL_FD, SIGNAL_FD
        };
        const Type type;
    public:
        fd_error() : an_error(), type(UNDEFINED) {}

        explicit fd_error(const string &arg) : an_error(arg), type(UNDEFINED) {}

        explicit fd_error(const string &arg, Type type) : an_error(arg), type(type) {}

        ~fd_error() override = default;
    };

    int timer_fd(__time_t second, __syscall_slong_t nano_second);

    int socket_fd(uint16_t port);

    int epoll_fd();

    int signal_fd();
}

#endif //SERVER_SERVER_UTILS_H

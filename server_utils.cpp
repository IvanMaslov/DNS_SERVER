//
// Created by maslov on 12.12.19.
//

#include "server_utils.h"

#include <netdb.h>
#include <arpa/inet.h>

#include <signal.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>

#include <set>

using std::set;


string get_addr_info_name(const string &h) {
    struct addrinfo *result;
    struct addrinfo *res;
    int error;
    set<string> answer;

    error = getaddrinfo(h.c_str(), NULL, NULL, &result);
    if (error != 0) {
        throw get_addr_info_error("getaddrinfo error code != 0\n");
    }

    for (res = result; res != NULL; res = res->ai_next) {
        char hostname[NI_MAXHOST];
        error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
        if (error != 0) {
            // ignore
            continue;
        }
        if (*hostname != '\0') {
            answer.insert(hostname);
        }
    }

    freeaddrinfo(result);

    string ans;
    for (const auto &i : answer)
        ans += i + "\n";

    return ans;
}


string get_addr_info(const string &h) {
    struct addrinfo *result;
    struct addrinfo *res;
    int error;
    set<string> answer;

    error = getaddrinfo(h.c_str(), NULL, NULL, &result);
    if (error != 0) {
        throw get_addr_info_error("getaddrinfo error code != 0\n");
    }

    for (res = result; res != NULL; res = res->ai_next) {
        unsigned int t = reinterpret_cast<sockaddr_in *>(res->ai_addr)->sin_addr.s_addr;
        answer.insert(inet_ntoa(in_addr{t}));
    }

    freeaddrinfo(result);

    string ans;
    for (const auto &i : answer)
        ans += i + "\n";
    ans += "\n";

    return ans;
}

an_error::an_error() = default;

an_error::an_error(const string &reason) : reason(reason) {}

string an_error::get_reason() const { return reason; }

int fd_fabric::timer_fd() {
    struct itimerspec new_value{};
    int max_exp, timerfd;
    struct timespec now{};
    uint64_t exp, tot_exp;
    ssize_t s;

    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
        throw server_error("clock_gettime system error");

    new_value.it_value.tv_sec = now.tv_sec;
    new_value.it_value.tv_nsec = now.tv_nsec;
    new_value.it_interval.tv_nsec = 10000; /// 10 microseconds
    //new_value.it_interval.tv_sec = 1; //DEBUG:timeout

    timerfd = timerfd_create(CLOCK_REALTIME, 0);
    if (timerfd == -1)
        throw server_error("timerfd_create system error");

    if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
        throw server_error("timerfd_settime system error");
    return timerfd;
}

int fd_fabric::socket_fd(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        throw server_error("socket create");
    }
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = 0;
    local_addr.sin_port = htons(port);
    if (bind(fd, reinterpret_cast<sockaddr const *>(&local_addr), sizeof(local_addr)) )
        throw server_error("socket bind");
    if (listen(fd, SOMAXCONN))
        throw server_error("socket listen");
    return fd;
}

int fd_fabric::epoll_fd() {
    int q = epoll_create(0xCAFE);
    if (q < 0) {
        throw exec_error("creating processor");
    }
    return q;
}

int fd_fabric::signal_fd() {
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

}
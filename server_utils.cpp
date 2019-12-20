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
            throw get_addr_info_error("getnameinfo error");
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

int fd_fabric::timer_fd(__time_t s, __syscall_slong_t ns) {
    struct itimerspec new_value{};
    int timerfd;
    struct timespec now{};

    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
        throw fd_error("clock_gettime system error", fd_error::TIMER_FD);

    new_value.it_value.tv_sec = now.tv_sec;
    new_value.it_value.tv_nsec = now.tv_nsec;

    new_value.it_interval.tv_sec = s;
    new_value.it_interval.tv_nsec = ns;

    timerfd = timerfd_create(CLOCK_REALTIME, 0);
    if (timerfd == -1)
        throw fd_error("timerfd_create system error", fd_error::TIMER_FD);

    if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
        throw fd_error("timerfd_settime system error", fd_error::TIMER_FD);
    return timerfd;
}

int fd_fabric::socket_fd(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        throw fd_error("socket create", fd_error::SOCKET_FD);
    }
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = 0;
    local_addr.sin_port = htons(port);
    if (bind(fd, reinterpret_cast<sockaddr const *>(&local_addr), sizeof(local_addr)))
        throw fd_error("socket bind", fd_error::SOCKET_FD);
    if (listen(fd, SOMAXCONN))
        throw fd_error("socket listen", fd_error::SOCKET_FD);
    return fd;
}

int fd_fabric::epoll_fd() {
    int q = epoll_create(0xCAFE);
    if (q < 0) {
        throw fd_error("creating processor", fd_error::EPOLL_FD);
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
        throw fd_error("sigprocmask", fd_error::SIGNAL_FD);

    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1)
        throw fd_error("signalfd", fd_error::SIGNAL_FD);

}
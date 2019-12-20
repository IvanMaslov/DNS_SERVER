//
// Created by maslov on 12.12.19.
//

#include "gtest/gtest.h"

#include <cstdio>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server_utils.h"
#include "uniq_fd.h"
#include "processor.h"
#include "addr_info_server.h"

namespace {
    using namespace std;

    const vector<string> domain_2 = {
            // "neerc.info.ru",
            "ipc.susu.ru",
            "vk.com",
    };

    const vector<string> result_2 = {
            // "expirepages-kiae-1.nic.ru\nexpirepages-kiae-2.nic.ru\n",
            "ipc.susu.ru\n",
            "93.186.225.208\nsrv158-137-240-87.vk.com\nsrv194-139-240-87.vk.com\nsrv67-190-240-87.vk.com\nsrv72-190-240-87.vk.com\nsrv78-190-240-87.vk.com\n"
    };


    const vector<string> domain = {
            "192.168.1.1",
            "vk.com",
            "ya.ru",
    };

    const vector<string> result = {
            "192.168.1.1\n\n",
            "87.240.137.158\n87.240.139.194\n87.240.190.67\n87.240.190.72\n87.240.190.78\n93.186.225.208\n\n",
            "0.0.0.0\n87.250.250.242\n\n",
    };

    const uint16_t PORT = 1476;
    bool started = false;

    TEST(UTIL_TEST, GET_ADDR_NAME_FUNCTIONAL) {
        for (size_t i = 0; i < domain_2.size(); ++i)
            EXPECT_EQ(get_addr_info_name(domain_2[i]), result_2[i]);
    }

    TEST(UTIL_TEST, GET_ADDR_FUNCTIONAL) {
        for (size_t i = 0; i < domain.size(); ++i)
            EXPECT_EQ(get_addr_info(domain[i]), result[i]);
    }

    TEST(UTIL_TEST, FABRICS) {
        fd_fabric::socket_fd(1234);
        fd_fabric::epoll_fd();
        fd_fabric::signal_fd();
        fd_fabric::timer_fd(0, 10000);
        try {
            fd_fabric::socket_fd(1234);
            FAIL() << "MULTI-SOCKET: created";
        } catch (const fd_fabric::fd_error &e) {
            EXPECT_EQ(e.get_reason(), "socket bind");
            EXPECT_EQ(e.type, fd_fabric::fd_error::SOCKET_FD);
        } catch (...) {
            FAIL() << "MULTI-SOCKET: unexpected exception";
        }
        fd_fabric::epoll_fd();
        fd_fabric::signal_fd();
        fd_fabric::timer_fd(1, 0);
    }

    TEST(UNIQ_FD, CONTRACT) {
        uniq_fd t;
        uniq_fd c(socket(0, 0, 0));
        uniq_fd d(socket(AF_INET, SOCK_STREAM, 0));
        uniq_fd e((uniq_fd()));
    }

    TEST(PROCESSOR, CONTRACT) {
        processor p;
        int q = 12;
        observed_fd s1(uniq_fd(eventfd(0, 0)), &p, [&q]() {
            q = 15;
            return;
        }, EPOLLIN);
        p.force_invoke(&s1);
        EXPECT_EQ(q, 15);
        int *e = new int(13);
        observed_fd s2(uniq_fd(eventfd(0, 0)), &p, [e]() {
            *e = 16;
            return;
        }, EPOLLIN);
        p.force_invoke(&s2);
        EXPECT_EQ(*e, 16);

        //CHECK: sanitizer
        delete e;
    }
#ifdef SERVER_ECHO_SERVER_H
    TEST(ECHO, CONTRACT) {
        processor p;
        echo_server s(&p, 15000);
        echo_server t(&p, 15001);
    }

    TEST(ECHO, CONNECT) {
        started = false;
        pthread_t f;
        pthread_create(&f, NULL, [](void *) -> void * {
            processor p;
            echo_server s(&p, PORT);
            started = true;
            p.execute();
            started = false;
        }, NULL);
        while (!started) {
            usleep(10);
        }

        for (int i = 0; i < 31; ++i) {
            int v = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in local_addr;
            local_addr.sin_family = AF_INET;
            local_addr.sin_addr.s_addr = 0;
            local_addr.sin_port = htons(PORT);

            usleep(20);
            connect(v, (struct sockaddr *) &local_addr, sizeof(local_addr));
            usleep(30);

            const string msg = "hello world!";
            char e[msg.size() + 1];
            e[msg.size()] = '\0';
            usleep(20);
            write(v, msg.c_str(), msg.size());
            usleep(100);
            read(v, e, msg.size() + 1);
            usleep(20);
            close(v);
            EXPECT_EQ(msg, string(e));
        }

        pthread_kill(f, SIGINT);
        while (started) {
            usleep(10);
        }
    }

    TEST(GET_ADDR_INFO, CONTRACT) {
        const uint16_t PORT2 = 14123;
        processor p;
        addr_info_server s(&p, PORT);

        started = false;
        pthread_t f;
        pthread_create(&f, NULL, [](void *) -> void * {
            processor p;
            addr_info_server s(&p, PORT2);
            started = true;
            p.execute();
            started = false;
        }, NULL);
        while (!started) {
            usleep(10);
        }

        pthread_kill(f, SIGINT);
        while (started) {
            usleep(10);
        }

    }
#endif
    TEST(GET_ADDR_INFO, CONNECT) {
        const uint16_t PORT2 = 14124;
        started = false;
        pthread_t f;
        pthread_create(&f, NULL, [](void *) -> void * {
            processor p;
            addr_info_server s(&p, PORT2);
            started = true;
            p.execute();
            started = false;
        }, NULL);
        while (!started) {
            usleep(10);
        }
#if 0
        for(int i = 0; i < 31; ++i) {
            int v = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in local_addr;
            local_addr.sin_family = AF_INET;
            local_addr.sin_addr.s_addr = 0;
            local_addr.sin_port = htons(PORT2);

            usleep(20);
            connect(v, (struct sockaddr *) &local_addr, sizeof(local_addr));
            usleep(30);

            const string msg = "192.168.1.1";
            const string ans = "192.168.1.1\n\n";
            char e[ans.size() + 1];
            e[ans.size()] = '\0';
            usleep(20);
            std::cerr << "write: " << msg << endl;
            write(v, msg.c_str(), msg.size());
            usleep(100);
            std::cerr << "read (expect): " << msg << endl;
            read(v, e, ans.size() + 1);
            std::cerr << "read (got): " << string(e) << endl;
            usleep(20);
            close(v);
            EXPECT_EQ(ans, string(e));
        }
#endif
        pthread_kill(f, SIGINT);
        while (started) {
            usleep(10);
        }
    }

}
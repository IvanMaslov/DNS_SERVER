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
#include "socket.h"

namespace {
    using namespace std;

    const vector<string> domain = {
            "neerc.info.ru",
            "ipc.susu.ru",
            "vk.com",
    };

    const vector<string> result = {
            "expirepages-kiae-1.nic.ru\nexpirepages-kiae-2.nic.ru\n",
            "ipc.susu.ru\n",
            "93.186.225.208\nsrv158-137-240-87.vk.com\nsrv194-139-240-87.vk.com\nsrv67-190-240-87.vk.com\nsrv72-190-240-87.vk.com\nsrv78-190-240-87.vk.com\n"
    };

    const uint16_t PORT = 14000;

    TEST(UTIL_TEST, GET_ADDR_FUNCTIONAL) {
        for (size_t i = 0; i < domain.size(); ++i)
            EXPECT_EQ(get_addr_info(domain[i]), result[i]);
    }

    TEST(UNIQ_FD, CONTRACT) {
        uniq_fd t;
        uniq_fd c(socket(0, 0, 0));
        uniq_fd d(socket(AF_INET, SOCK_STREAM, 0));
        uniq_fd e((uniq_fd()));
    }

    TEST(SOCKET, CONTRACT) {
        base_socket some(uniq_fd(socket(0, 0, 0)));
        some.invoke(0);
    }

    TEST(SKIP, SKIP1) {
        GTEST_SKIP();
        int u = socket(AF_INET, SOCK_STREAM, 0);
        int v = socket(AF_INET, SOCK_STREAM, 0);
        base_socket s((uniq_fd(u)));
        base_socket t((uniq_fd(v)));

        sockaddr_in local_addr;
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = 0;
        local_addr.sin_port = htons(PORT);

        bind(u, reinterpret_cast<sockaddr const *>(&local_addr), sizeof(local_addr));
        listen(u, SOMAXCONN);

        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = 0;
        local_addr.sin_port = htons(PORT);

        connect(v, (struct sockaddr *)&local_addr, sizeof(local_addr));

        const string msg = "hello world!";
        char e[msg.size()];
        t.write_c(msg.c_str(), msg.size());

        s.read_c(e, msg.size());
        EXPECT_EQ(msg, string(e));
    }

}
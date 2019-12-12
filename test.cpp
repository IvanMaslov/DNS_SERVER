//
// Created by maslov on 12.12.19.
//

#include "gtest/gtest.h"

#include <cstdio>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server_utils.h"

namespace {
    using namespace std;

    bool started = false;


    TEST(UTIL_TEST, SAMPLE) {
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
        for (size_t i = 0; i < domain.size(); ++i)
            EXPECT_EQ(get_addr_info(domain[i]), result[i]);

    }
}
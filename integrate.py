#!/bin/python

import socket
import random
import time

TEST_COUNT = 20001
VERBOSITY = 1
FAIL_RESULT = "Server Internal Error\n"

# VERBOSITY >= 1
VERBOSITY_TEST_SCALE = 1000

data = [
    ("vk.com\n", "87.240.137.158\n87.240.139.194\n87.240.190.67\n87.240.190.72\n87.240.190.78\n93.186.225.208\n\n"),
    ("ya.ru\n", "0.0.0.0\n87.250.250.242\n\n"),
    ("ok.ru\n", "217.20.147.1\n217.20.155.13\n5.61.23.11\n\n"),
    ("neerc.ifmo.ru\n", "77.234.215.132\n\n"),
]

host_ip, server_port = "127.0.0.1", 1212


def test_short():
    start_time = time.time()

    failed = 0

    for test_num in range(TEST_COUNT):
        e = random.randint(0, len(data) - 1)
        if (VERBOSITY > 0) and (test_num % VERBOSITY_TEST_SCALE == 0): print ("Test:    {}".format(test_num))
        try:
            tcp_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            tcp_client.connect((host_ip, server_port))
            tcp_client.sendall(data[e][0].encode())
            received = tcp_client.recv(1024)
        finally:
            tcp_client.close()

        if (VERBOSITY > 1): print ("Bytes Sent:     \n{}".format(data[e][0]))
        if (VERBOSITY > 1): print ("Bytes Received: \n{}".format(received.decode()))

        if not (data[e][1] == received):
            if (received == FAIL_RESULT):
                failed += 1
            else:
                print ("Error SHORT:  expected != received")
                print ("EXPECTED: {}".format(data[e][1]))
                print ("RECEIVED: {}".format(received))
                return

    print ("SUCCESS SHORT:    {} passed in {} seconds with {} errors".format(TEST_COUNT, (time.time() - start_time),
                                                                             failed))
    return


def test_long():
    start_time = time.time()

    failed = 0

    e = random.randint(0, len(data) - 1)
    try:
        tcp_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        tcp_client.connect((host_ip, server_port))

        for test_num in range(TEST_COUNT):
            tcp_client.sendall(data[e][0].encode())
            received = tcp_client.recv(1024)
            if (VERBOSITY > 0) and (test_num % VERBOSITY_TEST_SCALE == 0): print ("Test:    {}".format(test_num))
            if (VERBOSITY > 1): print ("Bytes Sent:     \n{}".format(data[e][0]))
            if (VERBOSITY > 1): print ("Bytes Received: \n{}".format(received.decode()))

            if not (data[e][1] == received):
                if (received == FAIL_RESULT):
                    failed += 1
                else:
                    print ("Error LONG:  expected != received")
                    print ("EXPECTED: {}".format(data[e][1]))
                    print ("RECEIVED: {}".format(received))
                    tcp_client.close()
                    return


    finally:
        tcp_client.close()

    print ("SUCCESS LONG:    {} passed in {} seconds with {} errors".format(TEST_COUNT, (time.time() - start_time),
                                                                            failed))
    return

test_short()
test_long()

test_short()
test_long()
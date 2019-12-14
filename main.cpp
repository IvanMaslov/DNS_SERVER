#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>

#include "socket.h"
#include "echo_server.h"
#include "addr_info_server.h"

int main() {
    try {
        processor p;
        echo_server echo1500(&p, 1500);
        echo_server echo15000(&p, 15000);
        addr_info_server get_addr_info(&p, 1212);
        p.execute();
        std::cerr << "gracefully stop" << std::endl;
    } catch (const an_error& e) {
        std::cerr << e.get_reason() << std::endl;
    }
    return 0;
}
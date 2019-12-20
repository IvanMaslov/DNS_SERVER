#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>

#include "addr_info_server.h"

int main() {
    try {
        processor p;
        addr_info_server get_addr_info(&p, 1212);
        addr_info_server get_addr_info2(&p, 1213);
        p.execute();
        std::cerr << "gracefully stop" << std::endl;
    } catch (const an_error &e) {
        std::cerr << e.get_reason() << std::endl;
    }
    return 0;
}
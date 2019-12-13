#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>

#include "socket.h"
#include "echo_server.h"

int main() {
    processor p;
    echo_server s(&p, 1500);
    p.execute();
    std::cerr << "gracefully stop" << std::endl;
    return 0;
}
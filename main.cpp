#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>

#include "socket.h";

int main() {
    int u = socket(AF_INET, SOCK_STREAM, 0);
    base_socket s((uniq_fd(u)));

    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = 0;
    local_addr.sin_port = htons(1500);

    bind(u, reinterpret_cast<sockaddr const *>(&local_addr), sizeof(local_addr));
    listen(u, SOMAXCONN);

    base_socket t(uniq_fd(accept(u, nullptr, nullptr)));

    const std::string msg = "hello world!";
    char e[msg.size()];

    t.write_c(msg.c_str(), msg.size());

    return 0;
}
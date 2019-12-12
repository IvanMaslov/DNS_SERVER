//
// Created by maslov on 12.12.19.
//

#include "server_utils.h"

#include <netdb.h>

#include <set>

using std::set;


string get_addr_info(const string& h) {
    struct addrinfo* result;
    struct addrinfo* res;
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
    for(const auto& i : answer)
        ans += i + "\n";

    return ans;
}

an_error::an_error() = default;
an_error::an_error(const string& reason) :reason(reason) {}
string an_error::get_reason() const { return reason; }

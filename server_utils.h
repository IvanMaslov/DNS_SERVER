//
// Created by maslov on 12.12.19.
//

#ifndef SERVER_SERVER_UTILS_H
#define SERVER_SERVER_UTILS_H

#include <string>

using std::string;
using std::exception;

string get_addr_info(const string&);

class an_error : exception {
public:
    an_error();
    explicit an_error(const string&);
    string get_reason() const;
private:
    string reason;
};

class get_addr_info_error : public an_error {
public:
    get_addr_info_error() : an_error() {}
    explicit get_addr_info_error(const string& arg) : an_error(arg) {}
    ~get_addr_info_error() override = default;
};

class server_error : public an_error {
public:
    server_error() : an_error() {}
    explicit server_error(const string& arg) : an_error(arg) {}
    ~server_error() override = default;
};



#endif //SERVER_SERVER_UTILS_H

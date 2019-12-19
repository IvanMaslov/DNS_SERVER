//
// Created by maslov on 12.12.19.
//

#ifndef SERVER_UNIQ_FD_H
#define SERVER_UNIQ_FD_H

#include <algorithm>
#include <functional>

using std::function;

struct uniq_fd {
    //FIXME:private
    int fd = -1;

    uniq_fd();
    explicit uniq_fd(function<int(void)>&&);
    explicit uniq_fd(int);
    uniq_fd(uniq_fd &&) noexcept;

    ~uniq_fd();

    uniq_fd(uniq_fd const&) = delete;
    uniq_fd operator=(uniq_fd const&) = delete;

    size_t read_c(void*, size_t);
    size_t write_c(const void*, size_t);
};

// template<> void std::swap<uniq_fd>(uniq_fd&, uniq_fd&) = delete;

#endif //SERVER_UNIQ_FD_H

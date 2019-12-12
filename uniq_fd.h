//
// Created by maslov on 12.12.19.
//

#ifndef SERVER_UNIQ_FD_H
#define SERVER_UNIQ_FD_H

#include <algorithm>

struct uniq_fd {
    const int fd = -1;

    uniq_fd();
    explicit uniq_fd(int);
    uniq_fd(uniq_fd const&&) noexcept;

    ~uniq_fd();

    uniq_fd(uniq_fd const&) = delete;
    uniq_fd operator=(uniq_fd const&) = delete;

};

// template<> void std::swap<uniq_fd>(uniq_fd&, uniq_fd&) = delete;

#endif //SERVER_UNIQ_FD_H

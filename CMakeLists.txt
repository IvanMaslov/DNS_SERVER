cmake_minimum_required(VERSION 3.14)
project(server)

set(CMAKE_CXX_STANDARD 17)

add_executable(server main.cpp server_utils.cpp server_utils.h uniq_fd.cpp uniq_fd.h socket.cpp socket.h)

add_subdirectory(test)
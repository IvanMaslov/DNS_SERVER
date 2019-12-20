cmake_minimum_required(VERSION 3.14)
project(server)

set(CMAKE_CXX_STANDARD 17)

add_executable(server main.cpp server_utils.cpp server_utils.h uniq_fd.cpp uniq_fd.h processor.cpp processor.h addr_info_server.cpp addr_info_server.h)

find_package(Threads)
target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(test)
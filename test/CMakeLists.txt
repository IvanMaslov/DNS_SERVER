# 'Google_test' is the subproject name
project(Google_tests)

# 'lib' is the folder with Google Test sources
add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# 'Google_Tests_run' is the target name
# 'test1.cpp tests2.cpp' are source files with tests
add_executable(Google_Tests_run ../test.cpp ../server_utils.cpp ../uniq_fd.cpp ../uniq_fd.h ../processor.cpp ../processor.h ../addr_info_server.cpp ../addr_info_server.h)
target_link_libraries(Google_Tests_run -fsanitize=address gtest gtest_main)
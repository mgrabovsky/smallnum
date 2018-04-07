project(smallnum)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
    -Wformat-security -Wcast-qual -Wuninitialized -fsanitize=undefined \
    -fsanitize=address")
set(SMALLNUM_SRC number.c)

add_library(smallnum STATIC ${SMALLNUM_SRC})
add_executable(sn_test ${SMALLNUM_SRC} test/test.c)

add_dependencies(sn_test smallnum)

# TODO: Improve directory structure and integrate tests according to
# <https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests>

# vim: set et sw=4:

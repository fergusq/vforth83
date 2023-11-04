cmake_minimum_required(VERSION 3.20)
project(vforth83)
set(CURSES_NEED_NCURSES true)
set(CURSES_NEED_WIDE true)
find_package(Curses REQUIRED)
add_executable(vforth83 builtins.c dos.c errors.c forth.c input_stream.c io.c memory.c stack.c util.c)
target_link_libraries(vforth83 ncursesw)

add_custom_command(
    OUTPUT test.fs
    COMMAND bash -c "cat ${CMAKE_CURRENT_SOURCE_DIR}/tests/tester.fr ${CMAKE_CURRENT_SOURCE_DIR}/tests/core.fr | sed -E 's/\\s/ /g' >test.fs"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/tester.fr ${CMAKE_CURRENT_SOURCE_DIR}/tests/core.fr
    VERBATIM)

add_custom_command(
    OUTPUT memory.bin
    COMMAND ./vforth83 -s ${CMAKE_CURRENT_SOURCE_DIR}/system.fs -M memory.bin
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/system.fs vforth83
    VERBATIM)

add_custom_target(
    test
    COMMAND ./vforth83 -m memory.bin -s test.fs
    DEPENDS vforth83 test.fs memory.bin
    VERBATIM)

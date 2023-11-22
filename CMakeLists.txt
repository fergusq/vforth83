cmake_minimum_required(VERSION 3.20)
project(vforth83)

if(EMSCRIPTEN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2 -s USE_SDL_TTF=2 -s ASYNCIFY")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
    set(CURSES_NEED_NCURSES true)
    set(CURSES_NEED_WIDE true)
    find_package(Curses REQUIRED)
endif()

find_package(SDL2 REQUIRED)

add_executable(vforth83 builtins.c dos.c errors.c forth.c input_stream.c io.c memory.c stack.c util.c)

if(EMSCRIPTEN)
    set_target_properties(vforth83 PROPERTIES LINK_FLAGS "--preload-file ./data@/")
    target_link_libraries(vforth83 SDL2 SDL2_ttf m freetype harfbuzz)
else()
    target_link_libraries(vforth83 ncursesw SDL2 SDL2_ttf m)
endif()

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

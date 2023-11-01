cmake_minimum_required(VERSION 3.20)
project(vforth83)
add_executable(vforth83 builtins.c dos.c errors.c forth.c input_stream.c memory.c stack.c util.c)

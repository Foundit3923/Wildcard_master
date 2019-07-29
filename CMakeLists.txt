cmake_minimum_required(VERSION 3.8)
project(Wildcard_master)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        benchmarking/krauss.h
        tests.c
        wildcard.h Experimental_wildcard.h)

add_executable(Wildcard_master ${SOURCE_FILES} Experimental_wildcard.h)
cmake_minimum_required(VERSION 3.14)
project(NewCard C)

set(CMAKE_C_STANDARD 99)

add_executable(NewCard main.c clean_experimental_wildcard.h wildcard_redesign.h hashtable.h arbitrary_length_wildcard.h arbitrary_location.h rabin_karp.h benchmarking/Allesaandro.h benchmarking/original_krauss.h benchmarking/shift-or.h KMP_arbirary_length_wildcard.h KMP_tester.h overlapping_windows.h benchmarking/dynamic.h benchmarking/KMP.h)
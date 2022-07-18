cmake_minimum_required(VERSION 3.19)

project(ste_unordered_bimap)
add_library(ste_unordered_bimap STATIC "unordered_bimap.hpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_target_properties(ste_unordered_bimap PROPERTIES LINKER_LANGUAGE CXX)

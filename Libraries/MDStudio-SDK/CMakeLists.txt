cmake_minimum_required(VERSION 3.12)
project(Studio)

include(CTest)

add_subdirectory(Source)
add_subdirectory(Tests)

enable_testing()

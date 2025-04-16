cmake_minimum_required(VERSION 3.12)
project(DistributedCompressor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(DC-Handler)
add_subdirectory(DC-Interface)
add_subdirectory(DC-Queue)

cmake_minimum_required(VERSION 3.12)
project(DistributedCompressor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.66 REQUIRED COMPONENTS filesystem system)
find_package(LibArchive REQUIRED)
find_package(amqpcpp REQUIRED)

add_subdirectory(include)
add_subdirectory(src)

add_subdirectory(DC-Handler)
add_subdirectory(DC-Interface)
#add_subdirectory(DC-Queue)

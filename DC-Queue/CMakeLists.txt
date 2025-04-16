cmake_minimum_required(VERSION 3.12)
project(DC-Queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.66 REQUIRED COMPONENTS filesystem system)

add_library(DCQueueSources
	src/Client.cxx
	src/ClientsManager.cxx
	src/QueueManager.cxx
	src/TaskQueue.cxx
	../src/ConfigLoader.cxx
        ../src/DC_QueueChannel.cxx
        ../src/DC_Queue_TaskQueueClient.cxx
        ../src/RabbitMQ_TaskQueueClient.cxx
        ../src/TaskPool.cxx
        ../src/TaskQueueClient.cxx
)

target_include_directories(DCQueueSources
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_executable(distributed_compressor_queue src/main.cxx)
target_link_libraries(distributed_compressor_queue
        PRIVATE
        DCQueueSources
        Boost::filesystem
        Boost::system
)


cmake_minimum_required(VERSION 3.12)
project(DC-Interface LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.66 REQUIRED COMPONENTS filesystem system)
find_package(amqpcpp REQUIRED)

add_library(DCInterfaceSources
	src/DC_Interface_ConfigLoader.cxx
	src/FileSeeker.cxx
	../src/ConfigLoader.cxx
	../src/DC_QueueChannel.cxx
	../src/DC_Queue_TaskQueueClient.cxx
	../src/RabbitMQ_TaskQueueClient.cxx
	../src/TaskPool.cxx
	../src/TaskQueueClient.cxx
)

target_include_directories(DCInterfaceSources
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_executable(distributed_compressor_interface src/main.cxx)
target_link_libraries(distributed_compressor_interface
	PRIVATE
	DCInterfaceSources
	Boost::filesystem
	Boost::system
	amqpcpp
)

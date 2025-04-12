project(DC-Interface LANGUAGES CXX)

add_library(DCInterfaceSources
	src/DC_Interface_ConfigLoader.cxx
	src/FileSeeker.cxx
)

target_link_libraries(DCInterfaceSources
	CommonHeaders
)

target_include_directories(DCInterfaceSources
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(distributed_compressor_interface src/main.cxx)
target_link_libraries(distributed_compressor_interface
	PRIVATE
	DCInterfaceSources
	CommonSources
	CommonHeaders
	Boost::filesystem
)

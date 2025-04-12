project(DC-Handler LANGUAGES CXX)

add_library(DCHandlerSources
        src/DC_Handler_ConfigLoader.cxx
        src/CompressHandler.cxx
)

target_link_libraries(DCHandlerSources
        CommonHeaders
)

target_include_directories(DCHandlerSources
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(distributed_compressor_handler src/main.cxx)
target_link_libraries(distributed_compressor_handler
        PRIVATE
	DCHandlerSources
        CommonSources
        CommonHeaders
        Boost::filesystem
)

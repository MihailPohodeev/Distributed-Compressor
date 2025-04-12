# Собираем общие исходники в библиотеку
add_library(CommonSources
	ConfigLoader.cxx
	RabbitMQ_TaskQueueClient.cxx
	TaskPool.cxx
	TaskQueueClient.cxx
)

target_link_libraries(CommonSources 
	PUBLIC 
	CommonHeaders
	Boost::filesystem
	Boost::system
	LibArchive::LibArchive
	amqpcpp
)

if(UNIX AND NOT APPLE)
	target_link_libraries(CommonSources PRIVATE pthread)
endif()

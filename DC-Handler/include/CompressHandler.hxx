#ifndef _COMPRESS_HANDLER_HXX_
#define _COMPRESS_HANDLER_HXX_

#include <TaskExecutor.hxx>
#include <functional>
#include <mutex>

class CompressorHandler : public TaskExecutor
{
	// compress file from 'fileData' and return new vector with compressed data.
	std::shared_ptr< std::vector<unsigned char> > create_zip_from_memory( const std::vector<unsigned char>& fileData, const std::string& filename );
public:
	void handle(const json& task, std::function<void(json)> callback = nullptr) override;
};

#endif

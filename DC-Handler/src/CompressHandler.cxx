#include <CompressHandler.hxx>
#include <boost/filesystem.hpp>
#include <JSON_FilePacker.hxx>
#include <vector>
#include <string>
#include <iostream>
#include <archive.h>
#include <archive_entry.h>

namespace fs = boost::filesystem;
using json = nlohmann::json;

void CompressorHandler::handle(const json& task, std::function<void(json)> callback)
{
	//lock_guard<std::mutex> zip_lock(zip_mutex_);
	try
	{
		std::string filepathStr = task.at("filepath");
		std::string content 	= task.at("content");
		
		std::cout << "filepath : " << filepathStr << "\ncontent : " << content << '\n';

		fs::path filepath( filepathStr );
		std::string filename = filepath.filename().string();
		std::string zipname  = filename + ".zip";

		std::vector<unsigned char> sourceFile  = JSON_FilePacker::base64_decode(content);
		std::vector<unsigned char> archiveFile = create_zip_from_memory(sourceFile, filename);
		
		std::string encodedZipFile = JSON_FilePacker::base64_encode(archiveFile);

		std::cout << "source file size : " << sourceFile.size() << "\narchive file size : " << archiveFile.size() << "\nencoded size : " << encodedZipFile.size() << '\n';

		json response;
		response["filepath"] = filepathStr + ".zip";
		response["content"] = encodedZipFile;

		std::cout << response.dump() << '\n';

		if (callback)
			callback(response);
	}
	catch (const json::type_error& e)
	{
		std::cerr << "Type Error!\nCan't handle task :" << task.dump() << '\n';
	}
	catch (const json::out_of_range& e)
	{
		std::cerr << "Out of Range Error!\nCan't handle task : " << task.dump() << '\n';
	}
}

std::vector<unsigned char> CompressorHandler::create_zip_from_memory( const std::vector<unsigned char>& fileData, const std::string& filename )
{
	struct archive* a = archive_write_new();
	archive_write_set_format_zip(a);

	std::vector<unsigned char> zipData;
	archive_write_open( a, &zipData, [](archive*, void* client_data) -> int { return ARCHIVE_OK; },
				[](archive*, void* client_data, const void* buff, size_t len) -> ssize_t
				{
					std::vector<unsigned char>* vec = static_cast<std::vector<unsigned char>*>(client_data);
					const unsigned char* bytes = static_cast<const unsigned char*>(buff);
					vec->insert(vec->end(), bytes, bytes + len);
					return len;
				},
				[](archive*, void* client_data) -> int { return ARCHIVE_OK; }
	);

	struct archive_entry* entry = archive_entry_new();
	archive_entry_set_pathname(entry, filename.c_str());
	archive_entry_set_size(entry, fileData.size());
	archive_entry_set_filetype(entry, AE_IFREG);
	archive_entry_set_perm(entry, 0644);

	archive_write_header(a, entry);
	archive_write_data(a, fileData.data(), fileData.size());

	archive_entry_free(entry);
	archive_write_close(a);
	archive_write_free(a);

	return zipData;
}

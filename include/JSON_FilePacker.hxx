#ifndef JSON_FILE_PACKER_HXX
#define JSON_FILE_PACKER_HXX

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>
#include <string>

namespace fs = boost::filesystem;
using json = nlohmann::json;

// JSON_FilePacker allows translate files to JSON format with all content.
class JSON_FilePacker
{
public:
	// encode file-content.
	static std::string base64_encode(const std::vector<unsigned char>& data) 
	{
		using namespace boost::archive::iterators;
		using Base64Encode = base64_from_binary< transform_width< std::vector<unsigned char>::const_iterator, 6, 8 > >;

		std::string encoded;
		encoded.reserve(((data.size() + 2) / 3) * 4);
        	
		std::copy( Base64Encode(data.begin()), Base64Encode(data.end()), std::back_inserter(encoded) );
		
		size_t pad = (3 - data.size() % 3) % 3;
		encoded.append(pad, '=');
		return encoded;
	}

	// decode file-content.
	static std::vector<unsigned char> base64_decode(const std::string& encoded)
	{
		using namespace boost::archive::iterators;
		using Base64Decode = transform_width< binary_from_base64<std::string::const_iterator>, 8, 6 >;
		
		std::string data = encoded;
		data.erase(std::remove(data.begin(), data.end(), '='), data.end());
		
		std::vector<unsigned char> decoded;
		decoded.reserve((data.size() * 3) / 4);
		
		std::copy( Base64Decode(data.begin()), Base64Decode(data.end()), std::back_inserter(decoded) );
		return decoded;
	}
	
	// save file in current directory.
	static void save_file(const std::string& filepath, const std::vector<unsigned char>& data) 
	{
		std::ofstream file( filepath, std::ios::binary | std::ios::trunc );
		if (!file) {
			throw std::runtime_error("Could not open file for writing");
		}
		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		file.close();
	}

	// convert json to file.
	static void json_to_file(const json& json_object)
	{
		std::string filename = json_object.at("filepath");
		std::string encoded_content = json_object.at("content");
		std::vector<unsigned char> decoded_data = base64_decode(encoded_content);
		save_file(filename, decoded_data);
	}

	// convert filt to json.
	static json file_to_json(const fs::path& filePath)
	{
		std::string filepath = filePath.string();
		// check file existance.
		if (!fs::exists(filePath)) {
			throw std::runtime_error( "File does not exist: " + filepath );
		}
		
		// check, if file isn't directory.
		if (!fs::is_regular_file(filePath)) {
			throw std::runtime_error( "Not a regular file: " + filepath );
		}

		std::ifstream file(filePath, std::ios::binary);
		if (!file) {
			throw std::runtime_error( "Cannot open file: " + filepath );
		}

		json result;
		result["filepath"] = filepath;
		
		std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), {});
		file.close();

		std::string encoded_content = base64_encode(buffer);
		result["content"] = encoded_content;

		return result;
	}
};

#endif

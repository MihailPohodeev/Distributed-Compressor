#ifndef CONFIG_LOADER_HXX
#define CONFIG_LOADER_HXX

#include <string>
#include <memory>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>

namespace fs = boost::filesystem;
using json = nlohmann::json;

class ConfigLoader
{
	std::string configName_;
	std::string appName_;

	void create_file(const std::string& filePath);
public:
	ConfigLoader(std::string appName, std::string configFileName = "config.json");

	// get path of config file.
	std::string get_config_file_path();
	std::shared_ptr<json> get_config();
	// analize params.
	json params_analizer(int argc, char** argv);
};

#endif

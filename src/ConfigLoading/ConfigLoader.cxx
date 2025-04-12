#include <ConfigLoading/ConfigLoader.hxx>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif


ConfigLoader::ConfigLoader(std::string appName, std::string configFileName) : configName_( configFileName ), appName_( appName ) {}

std::string ConfigLoader::get_config_file_path()
{
	std::string path;
#ifdef _WIN32
	char buffer[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, buffer))) {
		path = std::string(buffer) + "\\" + appName_ + "\\" + configName_;
	}
#else
	const char* homeDir = getenv("HOME");
	if (homeDir) {
		path = std::string(homeDir) + "/.config/" + appName_ + "/" + configName_;
	}
#endif	
	else {
		throw std::runtime_error("Can't get homedirectory for loading configs");
	}
		
	return path;
}

void ConfigLoader::create_file(const std::string& filePath)
{
	fs::path filePATH = fs::path(filePath);
	fs::path dir = filePATH.parent_path();
	std::string filename = filePATH.filename().string();

	if (!dir.empty() && !fs::exists(dir)) {
		fs::create_directories(dir);
	}

	std::ofstream file(filePath);
	if (file) {
		std::cout << "File " << filename << " created successful!" << std::endl;
	}
	else
	{
		std::cerr << "Can't create file " << filename << std::endl;
	}
}

std::shared_ptr<json> ConfigLoader::get_config()
{
	std::shared_ptr<json> resultJSON = nullptr;

	try
	{
		std::string path = get_config_file_path();
		std::fstream configFile( path );

		if (!configFile.is_open())
		{
			std::cerr << "Create new config file!" << std::endl;
			create_file(path);
			configFile = std::fstream( path );
			if (!configFile.is_open())
				throw std::runtime_error("Can't open " + configName_ + " - configuration file");
			
			configFile << "{}";
			configFile.seekg(0);
		}

		resultJSON = std::make_shared<json>();
		configFile >> *resultJSON;

		configFile.close();
	}
	catch(const json::parse_error& e)
	{
		std::cerr << "Parse Error!\nCan't handle config file!\nCheck config file : " << get_config_file_path() << "\nError message : " << e.what() << '\n';
		exit(-1);
	}

	return resultJSON;
}

json ConfigLoader::params_analizer(int argc, char** argv)
{
	json result;
	for (int i = 0; i < argc; i++)
	{
		// TODO
	}
	return result;
}

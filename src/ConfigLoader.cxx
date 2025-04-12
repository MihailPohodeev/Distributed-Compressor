#include <ConfigLoader.hxx>
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

namespace fs = boost::filesystem;

ConfigLoader::ConfigLoader(std::string appName, std::string configFileName) : configName_( configFileName ), appName_( appName ) {}

std::string ConfigLoader::get_config_file_path() const
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

// check, if string is filepath.
bool ConfigLoader::is_valid_file_path(const std::string& path) const
{
	try {
		fs::path p(path);
		return !p.empty();
	}
	catch (...) {
        	return false;
	}
}


// create new file using the specified path.
void ConfigLoader::create_file(const std::string& filePath) const
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

bool ConfigLoader::load_config(QueueParams& qp) const
{
	try
	{
		std::string path = get_config_file_path();
		std::fstream configFile( path );

		
		if (!configFile.is_open())
		{
			return false;
			/*
			std::cerr << "Create new config file!" << std::endl;
			create_file(path);
			configFile = std::fstream( path );
			if (!configFile.is_open())
				throw std::runtime_error("Can't open " + configName_ + " - configuration file");
			
			configFile << "{}";
			configFile.seekg(0);
			*/
		}

		json configJSON;
		configFile >> configJSON;

		json queueJSON = configJSON.at("queue");
		std::string queueType = queueJSON.at("queue-type");

		if (queueType == "RabbitMQ")
		{
			qp.username = queueJSON.at("username");
			qp.password = queueJSON.at("password");
			qp.host	    = queueJSON.at("hostname");
		}
		else if (queueType == "DC-Queue")
		{
			qp.host = queueJSON.at("hostname");
		}
		else
		{
			throw std::runtime_error("Can't handle config file");
		}

		configFile.close();
		return true;
	}
	catch(const json::parse_error& e)
	{
		std::cerr << "Parse Error!\nCan't handle config file!\nCheck config file : " << get_config_file_path() << "\nError message : " << e.what() << '\n';
	}
	return false;
}

// remove config-file.
void ConfigLoader::remove_config_file() const
{
	std::string path = get_config_file_path();

	try
	{
		fs::remove(path);
		std::cout << "Config file has been removed!\n";
	}
	catch(const fs::filesystem_error& e)
	{
		std::cerr << "Can't remove config-file!\n";
	}
}

// save configs from config-json in config-file.
void ConfigLoader::save_config(const QueueParams& qp) const
{
	std::string path = get_config_file_path();
	create_file(path);

	if (!fs::exists(path)) {
		std::cerr << "File was not created: " + path << '\n';
	}
	
	std::fstream configFile( path, std::ios::in | std::ios::out );
	if (!configFile.is_open())
	{
		std::cerr << "Can't open " + path + " - configuration file\n";
		return;
	}

	json configJSON;
	json queueJSON;
	if (qp.queueType == QueueType::RabbitMQ)
	{
		queueJSON["queue-type"] = "RabbitMQ";
		queueJSON["hostname"] 	= qp.host;
		queueJSON["username"] 	= qp.username;
		queueJSON["password"] 	= qp.password;
	}
	else if (qp.queueType == QueueType::DC_Queue)
	{
                queueJSON["queue-type"] = "DC-Queue";
		queueJSON["hostname"]	= qp.host;
	}
	configJSON["queue"] = queueJSON;

	configFile << configJSON.dump();
	configFile.close();

	std::cout << std::string(configName_) + " - config file created successful\n";
}

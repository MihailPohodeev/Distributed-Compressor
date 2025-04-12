#ifndef CONFIG_LOADER_HXX
#define CONFIG_LOADER_HXX

#include <string>
#include <nlohmann/json.hpp>
#include <QueueParams.hxx>

using json = nlohmann::json;

/*
 *	ConfigLoader is abstract class for different realization
 *	For DC-Interface we can receive parameters from json-file or from interactive poll -> implemented in DC_Interface_ConfigLoader
 *	For DC-Handler we can receive parameters from command line through flags (like --host localhost --name admin).
 */

class ConfigLoader
{
	std::string configName_;
	std::string appName_;

	// get path of config file for different operating systems.
	std::string get_config_file_path() const;

	// create new file using the specified path.
	void create_file(const std::string& filePath) const;

protected:
	// check, if string is filepath.
	bool is_valid_file_path(const std::string& path) const;
public:
	// constructor.
	ConfigLoader(std::string appName, std::string configFileName = "config.json");

	// analize params.
	virtual void params_analizer(int argc, char** argv, std::string& path, QueueParams* qp = nullptr) const = 0;

	// save configs from config-json in config-file.
	void save_config(const QueueParams& qp) const;

	// remove config-file.
	void remove_config_file() const;

	// load configs from config-file to 'qp'.
	bool load_config(QueueParams& qp) const;

	// input parametars from command line.
	virtual void input_parameters(QueueParams& qp) const = 0;
};

#endif

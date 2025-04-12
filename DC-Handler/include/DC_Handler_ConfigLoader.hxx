#ifndef DC_HANDLER_CONFIG_LOADER_HXX
#define DC_HANDLER_CONFIG_LOADER_HXX

#include <ConfigLoader.hxx>
#include <QueueParams.hxx>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
 *      Implementation of ConfigLoader for DC_Handler.
 */

class DC_Handler_ConfigLoader : public ConfigLoader
{
public:
        DC_Handler_ConfigLoader(std::string appName, std::string configFileName = "config.json");
        
        // analize params.
        void params_analizer(int argc, char** argv, std::string& path, QueueParams* qp = nullptr) const override;

        // input parametars from command line.
        void input_parameters(QueueParams& qp) const override;
};      

#endif

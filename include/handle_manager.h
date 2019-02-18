
#include "request_handler.h"
#include "config_parser.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "error_handler.h"
#include "status_handler.h"

class HandlerManager
{
public: 
    
    std::unique_ptr<RequestHandler> createByName(const std::string& name, const NginxConfig& config, const std::string& path);

};
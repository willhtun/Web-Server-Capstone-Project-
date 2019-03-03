#pragma once

#include <string>
#include <vector>
#include "request.h"
#include "response.h"
#include "request_handler.h"

class MemeHandler : public RequestHandler
{
public:     

    //create an instance of the handler
    static RequestHandler*  create(const NginxConfig& config, const std::string& path);

    //Handles the request and generates a response
    std::unique_ptr<Response> HandleRequest(const Request& request);

private:
    void MemeCreate(std::unique_ptr<Response> response);
    void MemeView();
    void MemeList();
    void MemeError(std::unique_ptr<Response> response);

    std::string root_;
    std::string uri_;
    std::string filedir_;
    std::string memepage_;
    std::string memebody_;
    bool errorflag = false;
};
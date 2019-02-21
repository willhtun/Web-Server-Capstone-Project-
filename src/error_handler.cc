#include <string>
#include <vector>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include "error_handler.h"

RequestHandler* ErrorHandler::create(const NginxConfig& config, const std::string& path)
{
    ErrorHandler* eh = new ErrorHandler();
    eh->root_ = path;
    eh->uri_ = config.GetAttribute("url");

    BOOST_LOG_TRIVIAL(trace) << "Error handler created";

    return eh;
};

std::unique_ptr<Response> ErrorHandler::HandleRequest(const Request& request)
{
    BOOST_LOG_TRIVIAL(trace) << "Error handler building response for request...";

    std::unique_ptr<Response> response(new Response());

    std::string error_msg = "404: File not found on path. Please provide correct path.";
    response->SetStatus(Response::NOT_FOUND);
    response->ReSetHeader("Content-Type", "text/plain");
    response->SetHeader("Content-Length", std::to_string(error_msg.length()));
    response->SetBody(error_msg);
    
    BOOST_LOG_TRIVIAL(trace) << "Response built by error handler...";

    return response;
};

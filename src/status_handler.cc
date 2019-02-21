#include <string>
#include <vector>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <tuple>
#include "status_handler.h"

RequestHandler* StatusHandler::create(const NginxConfig& config, const std::string& path)
{
    StatusHandler* sh = new StatusHandler();
    sh->root_ = path;
    sh->uri_ = config.GetAttribute("url");

    BOOST_LOG_TRIVIAL(trace) << "Status handler created";

    return sh;
};

std::unique_ptr<Response> StatusHandler::HandleRequest(const Request& request)
{
    // create fresh response obj
    BOOST_LOG_TRIVIAL(trace) << "Status handler building response for request...";
    std::unique_ptr<Response> response(new Response());

    // get status entry database
    std::vector<std::tuple<int,std::string,std::string>> status_entries = StatusObject::getStatusEntries();
    // build body string
    std::string body = "";
    body += "Status Page: git-r-done Server \n";

    // bring in handler details
    body += "\nRequest Handlers That Exist and Their Corresponding URL Prefixes:\n";
    std::vector<std::string> myReqHandlers = config_.getReqHandlers();
    for (std::vector<std::string>::const_iterator it = myReqHandlers.cbegin(); it != myReqHandlers.cend(); it++)
    {
        // TODO: specify handler and corresponding prefix
        body += *it + "\n";
    }

    std::cout << "Size of status entries: " << status_entries.size() << std::endl;
    //Get the total number of requests made
    int totalcalls = StatusObject::getStatusCalls();
    body += "Total Requests Made To git-r-done Server: " +std::to_string(totalcalls) + "\n";

    // bring in request url and response code pairs
    for (std::vector<std::tuple<int,std::string,std::string>>::const_iterator it = status_entries.begin();
         it != status_entries.end(); it++)
    {
        body += "Request URL: " + std::get<1>(*it) + " | Response Code: " + std::get<2>(*it) + " | Request Count: " + std::to_string(std::get<0>(*it)) + "\n";
    }

    // set response data
    response->SetStatus(Response::OK);
    response->ReSetHeader("Content-Type","text/plain");
    response->SetHeader("Content-Length", std::to_string(body.length()));
    response->SetBody(body);
    BOOST_LOG_TRIVIAL(trace) << "Response built by Status Handler...";

    return response;
};
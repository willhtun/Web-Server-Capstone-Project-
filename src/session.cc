#include <memory>
#include <iostream>
#include <streambuf>
#include <istream>
#include "session.h"
#include "request.h"
#include "response.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "server_object.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/system/system_error.hpp>

void session::start()
{
    BOOST_LOG_TRIVIAL(trace) << "Session has started...";

    // try-catch to help pass basic session->start() case
    try
    {
        BOOST_LOG_TRIVIAL(info) << "New connection from IP: " << socket_.remote_endpoint().address().to_string();
    }
    catch(boost::system::system_error const& e) {
        std::cout << e.what() << ": " << e.code() << " - " << e.code().message() << "\n";
    }

    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

// Ex GET request? GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred)
{
    if (error == boost::asio::error::eof)
    {

        BOOST_LOG_TRIVIAL(trace) << "EOF received...";
        // try-catch for passing test case
        try
        {
            BOOST_LOG_TRIVIAL(info) << "Dropped connection from IP: " << socket_.remote_endpoint().address().to_string() << "...";
        }
        catch(boost::system::system_error const& e)
        {
            std::cout << e.what() << ": " << e.code() << " - " << e.code().message() << "\n";
        }
        
        return;
    }
    if (!error)
    {
      // print original data
      std::cout << "My input: " <<  data_ << std::endl;
      
      // send data
      bool COMPLETE_ERROR = true;
      
      BOOST_LOG_TRIVIAL(trace) << "Sending data to request handler...";
      std::unique_ptr<Request> req = Request::request_handler(data_);
      if (req != nullptr)
      {
          // view data members
          BOOST_LOG_TRIVIAL(trace) << "Info about request{ "
                                   << "Method: " << req->method()
                                   << ", URI Path: " << req->uri_path()
                                   << ", HTTP Version: " << req->http_version()
                                   << "}";
          COMPLETE_ERROR = false;
      }

      //Writes back the response code and content type to the client
      std::string httpresponse;
      Response response_;
      
      if (!COMPLETE_ERROR)
      {
          //httpresponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(bytes_transferred) + "\r\n\r\n"; //The default response if request is complete
          //The default response if request is complete

         std::string full_uri = req->uri_path();
         int uri_length = 1;
         while (uri_length < full_uri.length() && full_uri[uri_length] != '/') {
             uri_length++;
         }
         std::string uri_type = full_uri.substr(1, uri_length - 1);

         int dir_id = ServerObject::findStaticDir(uri_type);
         if (dir_id != -1) {
            StaticHandler handler(ServerObject::staticfile_dir[dir_id]);
            handler.HandleRequest(*req, response_); 
            httpresponse = response_.Output();
         }
         else if ((req->uri_path()).substr(1, 4) == "echo") {
            EchoHandler handler;
            handler.HandleRequest(*req, response_); 
            httpresponse = response_.Output();
         }
      }
      else
      {
          std::string inc_req = "Incomplete request!\r\n\r\n";
          httpresponse = inc_req.c_str();
      }

      // combine response with original request
      //httpresponse = httpresponse + std::string(data_);
      
      boost::asio::async_write(socket_,
          boost::asio::buffer(httpresponse.c_str(), httpresponse.length()),
          boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
        
    }
    
    else
    {
        BOOST_LOG_TRIVIAL(error) << "async_read_some failed...";
        delete this;
    }
    
}

void session::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        try
        {
            BOOST_LOG_TRIVIAL(info) << "Writing response to IP: " << socket_.remote_endpoint().address().to_string() << "...";
        }
        catch(boost::system::system_error const& e) {
            std::cout << e.what() << ": " << e.code() << " - " << e.code().message() << "\n";
        }
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
}
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "session.h"
#include "config_parser.h"

using boost::asio::ip::tcp;

class server
{
  public:
    server(boost::asio::io_service& io_service, short port, NginxConfig* config)
        : io_service_(io_service),
          signals_(io_service),
          acceptor_(io_service),
          //new_connection_();
          thread_pool_size_(10)
        
    {
        signals_.add(SIGINT);
        signals_.add(SIGTERM);
        #if defined(SIGQUIT)
        signals_.add(SIGQUIT);
        #endif // defined(SIGQUIT)
        signals_.async_wait(boost::bind(&server::handle_stop, this));

        // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query("localhost", std::to_string(port));
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();

        
        config_ = config;
        start_accept();
    }

    void run();

    friend class Serv;

  private:
    void start_accept();
    void handle_accept(session* new_session,
        const boost::system::error_code& error);
    void handle_stop();
    
    std::size_t thread_pool_size_;
    boost::asio::io_service& io_service_;
    boost::asio::signal_set signals_;
    tcp::acceptor acceptor_;
    //connection_ptr new_connection_;
    NginxConfig* config_;
};


class Serv {
  public:
    void call_start_accept(server* serv) {
      serv->start_accept();
    }
    void call_handle_accept(server* serv, session* new_session, const boost::system::error_code& error) {
      serv->handle_accept(new_session, error);
    }
};
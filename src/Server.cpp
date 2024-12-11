#include "tp_www_beast/Server.h"
#include "tp_www_beast/Context.h"

#include "tp_www/Route.h"
#include "tp_www/Request.h"

#include "tp_utils/DebugUtils.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/signal_set.hpp>

#include <boost/bind/bind.hpp>

#include <thread>

#ifdef DELETE
  #undef DELETE
#endif

namespace tp_www_beast
{

namespace
{

//##################################################################################################
class HTTPConnection : public std::enable_shared_from_this<HTTPConnection>
{
  tp_www::Route* root;

  boost::asio::ip::tcp::socket socket;
  boost::beast::flat_buffer buffer{8192};
  boost::beast::http::request_parser<boost::beast::http::string_body> requestParser;
  std::string responseData;

#if BOOST_VERSION >= 107000
  boost::asio::steady_timer deadline{socket.get_executor(), std::chrono::seconds(60)};
#else
  boost::asio::steady_timer deadline{socket.get_io_context(), std::chrono::seconds(60)};
#endif

public:

  //################################################################################################
  HTTPConnection(boost::asio::ip::tcp::socket socket_, tp_www::Route* root_):
    root(root_),
    socket(std::move(socket_))
  {
    requestParser.body_limit(4096ull * 1024 * 1024);
  }

  //################################################################################################
  // Initiate the asynchronous operations associated with the connection.
  void start()
  {
    readRequest();
    checkDeadline();
  }

  //################################################################################################
  // Asynchronously receive a complete request message.
  void readRequest()
  {
    auto self = shared_from_this();

    boost::beast::http::async_read(socket, buffer, requestParser, [self](boost::beast::error_code ec, size_t bytes_transferred)
    {
      boost::ignore_unused(bytes_transferred);
      if(!ec)
        self->processRequest();
      else
      {
        tpWarning() << "Error reading request: " << ec.message();
        self->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
        self->deadline.cancel();
      }
    });
  }

  //################################################################################################
  tp_www::RequestType verbToRequestType(boost::beast::http::verb verb)
  {
    switch(verb)
    {
    case boost::beast::http::verb::get     : return tp_www::RequestType::GET    ;
    case boost::beast::http::verb::post    : return tp_www::RequestType::POST   ;
    case boost::beast::http::verb::head    : return tp_www::RequestType::HEAD   ;
    case boost::beast::http::verb::put     : return tp_www::RequestType::PUT    ;
    case boost::beast::http::verb::delete_ : return tp_www::RequestType::DELETE ;
    case boost::beast::http::verb::options : return tp_www::RequestType::OPTIONS;
    case boost::beast::http::verb::connect : return tp_www::RequestType::CONNECT;
    case boost::beast::http::verb::trace   : return tp_www::RequestType::TRACE  ;
    case boost::beast::http::verb::patch   : return tp_www::RequestType::PATCH  ;
    default: return tp_www::RequestType::GET;
    }
  }

  //################################################################################################
  // Determine what needs to be done with the request message.
  void processRequest()
  {
    auto self = shared_from_this();

    std::ostringstream out;
    std::ostringstream err;

    std::vector<std::string> parts;

#if BOOST_VERSION >= 108200
    tpSplit(parts, requestParser.get().target(), '?', TPSplitBehavior::KeepEmptyParts);
#else
    tpSplit(parts, requestParser.get().target().to_string(), '?', TPSplitBehavior::KeepEmptyParts);
#endif

    std::vector<std::string> route;
    if(!parts.empty())
      tpSplit(route, parts.front(), '/', TPSplitBehavior::SkipEmptyParts);

    std::unordered_map<std::string, std::string> getParams;
    if(parts.size() == 2)
    {
      std::vector<std::string> urlParts;
      tpSplit(urlParts, parts.at(1), '&', TPSplitBehavior::KeepEmptyParts);
      for(const auto& urlPart : urlParts)
      {
        std::vector<std::string> argParts;
        tpSplit(argParts, urlPart, '=', TPSplitBehavior::KeepEmptyParts);
        if(!argParts.empty())
        {
          std::string key = argParts.front();
          if(!key.empty())
          {
            std::string value;
            if(argParts.size()==2)
              value = argParts.at(1);

            getParams[key] = value;
          }
        }
      }
    }

    tp_www::RequestType requestType = verbToRequestType(requestParser.get().method());

    std::unordered_map<std::string, std::string> postParams;
    std::unordered_map<std::string, tp_www::MultipartFormData> multipartFormData;

    tp_www::Request wwwRequest(out, err, route, requestType, requestParser.get().body(), postParams, getParams, multipartFormData);

    if(!root->handleRequest(wwwRequest, 0))
      wwwRequest.sendBinary(404, "text/html", "404 Not Found");

#if BOOST_VERSION >= 108200
    std::string status =  boost::beast::http::obsolete_reason(boost::beast::http::int_to_status(unsigned(wwwRequest.httpStatus())));
#else
    std::string status =  boost::beast::http::obsolete_reason(boost::beast::http::int_to_status(unsigned(wwwRequest.httpStatus()))).to_string();
#endif
    responseData = "HTTP/1.1 " + std::to_string(wwwRequest.httpStatus()) + ' ' + status + "\r\n" + out.str();

    boost::asio::async_write(socket, boost::asio::buffer(responseData), [self](boost::beast::error_code ec, size_t)
    {
      self->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
      self->deadline.cancel();
    });
  }

  //################################################################################################
  // Check whether we have spent enough time on this connection.
  void checkDeadline()
  {
    auto self = shared_from_this();
    deadline.async_wait([self](boost::beast::error_code ec)
    {
      if(!ec)
      {
        // Close socket to cancel any outstanding operation.
        self->socket.close(ec);
      }
    });
  }
};
}

//##################################################################################################
struct Server::Private
{
  Context* context;
  tp_www::Route* root;
  uint16_t port;

  boost::asio::ip::address_v4 const address{boost::asio::ip::address_v4::any()};
  boost::asio::ip::tcp::acceptor acceptor{*context->ioc(), {address, port}};
  boost::asio::ip::tcp::socket socket{*context->ioc()};

  //################################################################################################
  Private(Context* context_, tp_www::Route* root_, uint16_t port_):
    context(context_),
    root(root_),
    port(port_)
  {
    try
    {
      httpServer();
    }
    catch(const std::exception& e)
    {
      tpWarning() << "Error: " << e.what();
    }
  }

  //################################################################################################
  // "Loop" forever accepting new connections.
  void httpServer()
  {
    acceptor.async_accept(socket, [this](boost::beast::error_code ec)
    {
      if(!ec)
        std::make_shared<HTTPConnection>(std::move(socket), root)->start();
      httpServer();
    });
  }
};

//##################################################################################################
Server::Server(Context* context, tp_www::Route* root, uint16_t port):
  d(new Private(context, root, port))
{

}

//##################################################################################################
Server::~Server()
{
  delete d;
}

}

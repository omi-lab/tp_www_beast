#ifndef tp_www_beast_Context_h
#define tp_www_beast_Context_h

#include "tp_www_beast/Globals.h"

#include "tp_utils/AbstractCrossThreadCallback.h"

namespace boost::asio
{
class io_context;
}

namespace tp_www
{
class Route;
}

namespace tp_www_beast
{

//##################################################################################################
class TP_WWW_BEAST_EXPORT Context
{
  TP_DQ;
public:
  //################################################################################################
  Context(bool runInThread);

  //################################################################################################
  ~Context();

  //################################################################################################
  void stop();

  //################################################################################################
  boost::asio::io_context* ioc();

  //################################################################################################
  boost::asio::io_context* iocMain();

  //################################################################################################
  tp_utils::AbstractCrossThreadCallbackFactory* crossThreadCallbackFactory() const;

  //################################################################################################
  void waitCtrlC();
};

}

#endif

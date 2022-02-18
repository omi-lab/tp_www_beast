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
class Context
{
public:
  //################################################################################################
  Context(bool runInThread);

  //################################################################################################
  ~Context();

  //################################################################################################
  boost::asio::io_context* ioc();

  //################################################################################################
  tp_utils::AbstractCrossThreadCallbackFactory* crossThreadCallbackFactory() const;

  //################################################################################################
  void waitCtrlC();

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif

#ifndef tp_www_beast_Server_h
#define tp_www_beast_Server_h

#include "tp_www_beast/Globals.h"

namespace tp_www
{
class Route;
}

namespace tp_www_beast
{
class Context;

//##################################################################################################
class TP_WWW_BEAST_EXPORT Server
{
  TP_DQ;
public:
  //################################################################################################
  Server(Context* context, tp_www::Route* root, uint16_t port);

  //################################################################################################
  ~Server();
};

}

#endif

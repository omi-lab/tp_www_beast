#ifndef tp_www_beast_Server_h
#define tp_www_beast_Server_h

#include "tp_www_beast/Globals.h"

namespace tp_www
{
class Route;
}

namespace tp_www_beast
{

//##################################################################################################
class Server
{
public:

  //################################################################################################
  Server(tp_www::Route* root);

  //################################################################################################
  ~Server();

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif

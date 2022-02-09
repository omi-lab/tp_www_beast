#ifndef tp_www_beast_ASIOCrossThreadCallbackFactory_h
#define tp_www_beast_ASIOCrossThreadCallbackFactory_h

#include "tp_www_beast/Globals.h"

#include "tp_utils/AbstractCrossThreadCallback.h"

namespace tp_www_beast
{
class Context;

//##################################################################################################
class ASIOCrossThreadCallbackFactory : public tp_utils::AbstractCrossThreadCallbackFactory
{
public:
  //################################################################################################
  ASIOCrossThreadCallbackFactory(Context* context);

  //################################################################################################
  [[nodiscard]] tp_utils::AbstractCrossThreadCallback* produce(const std::function<void()>& callback) const;

private:
  Context* m_context;
};

}

#endif

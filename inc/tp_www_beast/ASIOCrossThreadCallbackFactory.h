#ifndef tp_www_beast_ASIOCrossThreadCallbackFactory_h
#define tp_www_beast_ASIOCrossThreadCallbackFactory_h

#include "tp_www_beast/Globals.h"

#include "tp_utils/AbstractCrossThreadCallback.h"

namespace tp_www_beast
{
class Context;

//##################################################################################################
class TP_WWW_BEAST_EXPORT ASIOCrossThreadCallbackFactory : public tp_utils::AbstractCrossThreadCallbackFactory
{
public:
  //################################################################################################
  ASIOCrossThreadCallbackFactory(Context* context);

protected:
  //################################################################################################
  [[nodiscard]] tp_utils::AbstractCrossThreadCallback* produce(const std::function<void()>& callback) const override;

private:
  Context* m_context;
};

}

#endif

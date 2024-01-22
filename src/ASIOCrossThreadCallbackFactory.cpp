#include "tp_www_beast/ASIOCrossThreadCallbackFactory.h"
#include "tp_www_beast/Context.h"

#include <boost/asio/post.hpp>
#include <boost/asio/io_context.hpp>

namespace tp_www_beast
{

namespace
{
//##################################################################################################
class ASIOCrossThreadCallback final : public tp_utils::AbstractCrossThreadCallback
{
public:

  //################################################################################################
  ASIOCrossThreadCallback(const std::function<void()>& callback, Context* context):
    tp_utils::AbstractCrossThreadCallback(callback),
    m_context(context)
  {

  }

  //################################################################################################
  void call() override
  {
    std::weak_ptr<int> weak=m_exists;
    boost::asio::post(*m_context->ioc(), [&, weak]
    {
      if(!weak.expired())
        callback();
    });
  }

private:
  Context* m_context;
  std::shared_ptr<int> m_exists{std::make_shared<int>(0)};
};
}

//##################################################################################################
ASIOCrossThreadCallbackFactory::ASIOCrossThreadCallbackFactory(Context* context):
  m_context(context)
{

}

//##################################################################################################
tp_utils::AbstractCrossThreadCallback* ASIOCrossThreadCallbackFactory::produce(const std::function<void()>& callback) const
{
  return new ASIOCrossThreadCallback(callback, m_context);
}

}

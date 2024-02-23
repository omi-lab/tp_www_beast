#include "tp_www_beast/Context.h"
#include "tp_www_beast/ASIOCrossThreadCallbackFactory.h"

#include "tp_utils/DebugUtils.h"

#include <boost/asio/signal_set.hpp>

#include <boost/bind/bind.hpp>

#include <thread>

namespace tp_www_beast
{

//##################################################################################################
struct Context::Private
{
  bool runInThread;
  boost::asio::io_context* ioc{new boost::asio::io_context(1)};
  boost::asio::io_context* iocMain{new boost::asio::io_context(1)};
  std::thread thread;

  std::unique_ptr<ASIOCrossThreadCallbackFactory> crossThreadCallbackFactory;

  //################################################################################################
  Private(bool runInThread_):
    runInThread(runInThread_)
  {
  }

  //################################################################################################
  ~Private()
  {
    if(runInThread)
    {
      ioc->stop();
      thread.join();
    }
    delete ioc;
    delete iocMain;
  }

  //################################################################################################
  void run()
  {
    try
    {
      ioc->run();
    }
    catch(const std::exception& e)
    {
      tpWarning() << "Error: " << e.what();
    }
  }

  //################################################################################################
  void runMain()
  {
    try
    {
      iocMain->run();
    }
    catch(const std::exception& e)
    {
      tpWarning() << "Error: " << e.what();
    }
  }

  //################################################################################################
  void signalHandler(const boost::system::error_code&, int signalNumber)
  {
    tpWarning() << "\nSignal caught " << signalNumber;
    ioc->stop();
    iocMain->stop();
  }
};

//##################################################################################################
Context::Context(bool runInThread):
  d(new Private(runInThread))
{
  d->crossThreadCallbackFactory = std::make_unique<ASIOCrossThreadCallbackFactory>(this);
}

//##################################################################################################
Context::~Context()
{
  delete d;
}

//##################################################################################################
boost::asio::io_context* Context::ioc()
{
  return d->ioc;
}

//##################################################################################################
boost::asio::io_context* Context::iocMain()
{
  if(d->runInThread)
    return d->iocMain;
  else
    return d->ioc;
}

//##################################################################################################
tp_utils::AbstractCrossThreadCallbackFactory* Context::crossThreadCallbackFactory() const
{
  return d->crossThreadCallbackFactory.get();
}

//##################################################################################################
void Context::waitCtrlC()
{
  if(d->runInThread)
    d->thread = std::thread([&]{d->run();});

  auto ioc = iocMain();

#ifdef TP_WIN32_MSVC
  boost::asio::signal_set waitSignals(*ioc, SIGINT, SIGTERM);
#else
  boost::asio::signal_set waitSignals(*ioc, SIGINT, SIGTERM, SIGABRT);
#endif

  waitSignals.async_wait(boost::bind(&boost::asio::io_context::stop, ioc));
  d->runMain();
}

}

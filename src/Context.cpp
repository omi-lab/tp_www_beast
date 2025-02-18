#include "tp_www_beast/Context.h"
#include "tp_www_beast/ASIOCrossThreadCallbackFactory.h"

#include "tp_utils/DebugUtils.h"

#include <boost/asio/signal_set.hpp>

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
#ifdef TP_LINUX
    tpWarning() << "Signal caught: " << signalNumber << " (" << strsignal(signalNumber) << ')';
#else
    tpWarning() << "Signal caught: " << signalNumber;
#endif

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
void Context::stop()
{
  tpWarning() << "Context::stop() called.";
  d->ioc->stop();
  d->iocMain->stop();
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

  auto signalHandler = [this](const boost::system::error_code& ec, int signalNumber)
  {
    d->signalHandler(ec, signalNumber);
  };

#ifdef TP_WIN32_MSVC
  boost::asio::signal_set waitSignals(*ioc, SIGINT, SIGTERM);
#else
  boost::asio::signal_set waitSignals(*ioc, SIGINT, SIGTERM, SIGABRT);
#endif

  waitSignals.async_wait(signalHandler);
  d->runMain();

  if(d->runInThread)
  {
    d->ioc->stop();
    d->thread.join();
  }
}

}

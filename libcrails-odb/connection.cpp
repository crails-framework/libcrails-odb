#include "connection.hpp"
#include <crails/logger.hpp>

using namespace std;
using namespace Crails;

thread_local safe_ptr<Odb::Connection> Odb::Connection::instance;

Odb::Connection::Connection()
{
  if (instance)
    throw runtime_error("only one instance of Odb::Connection allowed per thread");
  instance = shared_ptr<Odb::Connection>(this, [](Odb::Connection*) {});
}

Odb::Connection::~Connection()
{
  rollback();
  instance.reset();
}

void Odb::Connection::commit()
{
  Crails::logger << Crails::Logger::Info << "Transaction commit. Database time: " << time << 's' << Crails::Logger::endl;
  Utils::Timer timer;
  transaction.commit();
  Crails::logger << Crails::Logger::Info << "Transaction committed in " << timer.GetElapsedSeconds() << Crails::Logger::endl;
  time = 0.f;
}

void Odb::Connection::rollback()
{
  transaction.rollback();
  time = 0.f;
}

#include "connection.hpp"
#include <crails/logger.hpp>

using namespace std;
using namespace Crails;

Odb::Connection::Connection() : transaction(Odb::Transaction::get())
{
}

Odb::Connection::~Connection()
{
  if (rollback_on_destruction)
    rollback();
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
  Crails::logger << Crails::Logger::Info << "Transaction rollback. Database time: " << time << 's' << Crails::Logger::endl;
  transaction.rollback();
  time = 0.f;
}

bool Odb::Connection::recoverable_action(std::function<bool()> action) const
{
  try
  {
    return action();
  }
  catch (const odb::recoverable& err)
  {
    logger << Logger::Warning << "Repeating recoverable database operation: " << err.what() << Logger::endl;
    return action();
  }
}

unsigned long Odb::Connection::recoverable_count(std::function<unsigned long()> action) const
{
  try
  {
    return action();
  }
  catch (const odb::recoverable& err)
  {
    logger << Logger::Warning << "Repeating recoverable database count operation: " << err.what() << Logger::endl;
    return action();
  }
}

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

bool Odb::Connection::execute(std::string_view query)
{
  return recoverable_action([this, query]() -> bool
  {
    unsigned long affected_rows;

    affected_rows = transaction.get_database().execute(query.data(), query.length());
    return affected_rows > 0;
  });
}

bool Odb::Connection::recoverable_action(std::function<bool()> action) const
{
  try
  {
    return action();
  }
  catch (const odb::connection_lost& err)
  {
    logger << Logger::Warning << "Connection was closed by the SQL server. Reconnecting and retrying." << Logger::endl;
    return action();
  }
  catch (const odb::recoverable& err)
  {
    logger << Logger::Warning << "Repeating recoverable database operation: " << err.what() << Logger::endl;
    return action();
  }
  catch (const std::exception& err)
  {
    logger << Logger::Error << "Catched non-recoverable exception from recoverable action: " << err.what() << Logger::endl;
    throw boost_ext::runtime_error(err.what());
  }
}

unsigned long Odb::Connection::recoverable_count(std::function<unsigned long()> action) const
{
  try
  {
    return action();
  }
  catch (const odb::connection_lost& err)
  {
    logger << Logger::Warning << "Connection was closed by the SQL server. Reconnecting and retrying." << Logger::endl;
    return action();
  }
  catch (const odb::recoverable& err)
  {
    logger << Logger::Warning << "Repeating recoverable database count operation: " << err.what() << Logger::endl;
    return action();
  }
  catch (const odb::exception& err)
  {
    logger << Logger::Error << "Catched non-recoverable exception from recoverable action: " << err.what() << Logger::endl;
    throw boost_ext::runtime_error(err.what());
  }
  return 0;
}

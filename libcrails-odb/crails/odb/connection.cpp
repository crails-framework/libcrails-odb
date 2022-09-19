#include "connection.hpp"
#include <crails/logger.hpp>

using namespace std;
using namespace Crails;

Odb::Connection::Connection() : transaction(Odb::Transaction::get())
{
}

Odb::Connection::~Connection()
{
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
  transaction.rollback();
  time = 0.f;
}

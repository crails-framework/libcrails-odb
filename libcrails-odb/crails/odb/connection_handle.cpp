#include "connection.hpp"
#include <odb/transaction.hxx>
#include <crails/logger.hpp>

using namespace std;
using namespace Crails;

Odb::ConnectionHandle::ConnectionHandle()
{
  rollback_on_destruction = false;
  if (odb::transaction::has_current())
  {
    logger << Logger::Debug << "Odb::ConnectionHandle was instantiated, but a transaction is dangling. Rollbacking dangling transaction" << Logger::endl;
    odb::transaction::current().rollback();
    odb::transaction::reset_current();
  }
}

Odb::ConnectionHandle::ConnectionHandle(Connection& target)
{
  rollback_on_destruction = false;
  active_transaction = &target.transaction();
}

Odb::ConnectionHandle::~ConnectionHandle()
{
  if (rollback_on_destruction)
    rollback();
  else if (active_transaction == &own_transaction)
    commit();
}

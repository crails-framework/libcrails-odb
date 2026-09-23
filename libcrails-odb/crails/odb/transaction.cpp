#include <odb/transaction.hxx>
#include <odb/session.hxx>
#include <odb/database.hxx>
#include <crails/databases.hpp>
#include "database.hpp"
#include "transaction.hpp"
#include "database_settings.hpp"
#include "exception.hpp"

using namespace std;
using namespace Crails;

extern const std::string default_configuration_name;
bool Odb::Transaction::use_session = false;

Odb::Transaction::Transaction() : odb_database(0)
{
}

Odb::Transaction::~Transaction()
{
}

odb::database& Odb::Transaction::get_database()
{
  if (!odb_database)
    throw Odb::Exception("asked for a database on an uninitialized Transaction");
  return *odb_database;
}

void Odb::Transaction::require(const std::string& name)
{
  if (database_name != name || !odb_database)
  {
    if (name == "default")
      start(name, CRAILS_DATABASE(Odb, default_configuration_name).get_agnostic_database());
    else
    {
      auto  database_settings = Odb::get_database_settings_for(name);
      auto& crails_database = CRAILS_DATABASE_FROM_SETTINGS(Odb, name, database_settings);

      start(name, crails_database.get_agnostic_database());
    }
  }
}

void Odb::Transaction::start(const std::string& name, odb::database& database)
{
  rollback();
  try
  {
    database_name   = name;
    odb_transaction = unique_ptr<odb::transaction>(
      new odb::transaction(database.begin())
    );
    odb_session.reset();
    if (use_session)
      odb_session   = unique_ptr<odb::session>(new odb::session);
    odb_database    = &database;
  }
  catch (const odb::exception& e)
  {
    throw Odb::Exception(e);
  }
}

void Odb::Transaction::commit()
{
  if (odb_transaction)
  {
    try
    {
      odb_transaction->commit();
      cleanup();
    }
    catch (const odb::exception& e)
    {
      throw Odb::Exception(e);
    }
  }
}

void Odb::Transaction::rollback()
{
  if (odb_transaction)
  {
    try
    {
      odb_transaction->rollback();
      cleanup();
    }
    catch (const odb::exception& e)
    {
      logger << Logger::Error << "Odb MAJOR FAILURE: rollback failed: " << e.what() << Logger::endl;
    }
  }
}

void Odb::Transaction::acquire_thread()
{
  if (active())
    odb::transaction::current(*odb_transaction);
}

void Odb::Transaction::release_thread()
{
  if (active())
    odb::transaction::reset_current();
}

void Odb::Transaction::cleanup()
{
  odb_database    = nullptr;
  odb_session     = nullptr;
  odb_transaction = nullptr;
}

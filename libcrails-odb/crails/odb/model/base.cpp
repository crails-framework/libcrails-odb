#include "base.hpp"

using namespace std;
using namespace Crails;

std::string Odb::ModelBase::get_database_name() const
{
  return "default";
}

void Odb::ModelBase::save(odb::database& db)
{
  if (!is_persistent())
    odb_persist(db);
  else
    odb_update(db);
  erased = false;
}

void Odb::ModelBase::destroy(odb::database& db)
{
  if (is_persistent())
    odb_erase(db);
  erased = true;
}

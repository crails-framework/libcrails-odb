#include "soft_delete.hpp"

using namespace std;
using namespace Crails;

void Odb::ModelSoftDelete::save(odb::database& db)
{
  deleted = false;
  BaseType::save(db);
}

void Odb::ModelSoftDelete::destroy(odb::database& db)
{
  if (with_soft_delete() && get_id() != 0)
  {
    deleted = true;
    BaseType::save(db);
  }
  else
    BaseType::destroy(db);
}

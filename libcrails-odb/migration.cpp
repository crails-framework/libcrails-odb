#include "migration.hpp"

using namespace Crails::Odb;
using namespace std;

bool Migrations::run_for_version(Database& db, odb::schema_version version) const
{
  for (auto migration : list)
  {
    if (migration.version == version)
    {
      cout << "+ " << migration.name << endl;
      if (!(migration.runner(db)))
      {
        cout << "/!\\ Migration failed to run" << endl;
        return false;
      }
    }
  }
  return true;
}

#ifndef  CRAILS_ODB_MIGRATION_HPP
# define CRAILS_ODB_MIGRATION_HPP

# include <odb/schema-catalog.hxx>
# include <list>
# include <functional>

# ifndef ODB_COMPILER
#  include "database.hpp"
# else
namespace Crails { namespace Odb { class Database; } }
# endif

namespace Crails
{
  namespace Odb
  {
    typedef std::function<bool (Odb::Database&, odb::schema_version)> MigrateFunction;

    struct Migration
    {
      std::string                          name;
      odb::schema_version                  version;
      std::function<bool (Odb::Database&)> runner;
    };

    struct Migrations
    {
      Migrations();

      bool run_for_version(Odb::Database& db, odb::schema_version version) const;
      std::list<Migration> list;

      operator MigrateFunction() const
      {
        return std::bind(&Odb::Migrations::run_for_version, this, std::placeholders::_1, std::placeholders::_2);
      }
    };
  }
}

# define ADD_MIGRATION(name, version, body) \
  Crails::Odb::Migration name = { #name, version, [](Odb::Database& database) -> bool body };

#endif

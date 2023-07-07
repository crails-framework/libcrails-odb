#ifndef  CRAILS_ODB_MIGRATE_HPP
# define CRAILS_ODB_MIGRATE_HPP

# include "odb.hpp"
# include <map>
# include <crails/logger.hpp>
# include <crails/utils/backtrace.hpp>
# include <odb/schema-catalog.hxx>

namespace Crails
{
  namespace Odb
  {
    class Database;

    template<typename T>
    void database_drop(T& db)
    {
      odb::transaction t(db.begin());

      odb::schema_catalog::drop_schema(db);
      t.commit();
    }

    template<typename T>
    void database_migrate(Odb::Database& self, T& database, std::function<bool (Odb::Database&, odb::schema_version)> callback)
    {
      using namespace Crails;
      odb::schema_version v(database.schema_version());

      logger << Logger::Info;
      if (v == 0)
      {
        odb::transaction t(database.begin());
        logger << ":: No schema version found\n";
        logger << ":: Initializing database from scratch" << Logger::endl;
        odb::schema_catalog::create_schema(database);
        t.commit();
      }
      else
      {
        odb::schema_version cv(odb::schema_catalog::current_version(database));

        if (v < cv)
        {
          logger << ":: Migration" << '\n';
          for (v = odb::schema_catalog::next_version(database, v) ;
              v <= cv ;
              v = odb::schema_catalog::next_version(database, v))
          {
            {
              odb::transaction t(database.begin());

              logger << ":: Running migration to version " << v << '\n';
              odb::schema_catalog::migrate_schema_pre(database, v);
              t.commit();
            }

            {
              odb::transaction t(database.begin());

              if (callback && !(callback(self, v)))
                throw boost_ext::runtime_error("database migration failed");
              t.commit();
            }

            {
              odb::transaction t(database.begin());

              odb::schema_catalog::migrate_schema_post(database, v);
              t.commit();
            }
          }
        }
        else
          logger << ":: Nothing to do" << Logger::endl;
      }
    }
  }
}

#endif

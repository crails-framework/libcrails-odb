#ifndef  CRAILS_ODB_DATABASE_LOADERS_HPP
# define CRAILS_ODB_DATABASE_LOADERS_HPP

# include "database.hpp"
# include "crails/any_cast.hpp"

namespace Crails
{
  namespace Odb
  {
    inline odb::database* initialize_for_mysql(const Crails::Databases::DatabaseSettings& settings)
    {
  #ifdef CRAILS_ODB_WITH_MYSQL
      return new odb::mysql::database(
        Crails::any_cast(settings.at("user")),
        Crails::any_cast(settings.at("password")),
        Crails::any_cast(settings.at("name")),
        defaults_to<const char*> (settings, "host", ""),
        defaults_to<unsigned int>(settings, "port", 0)
        0,
        defaults_to<const char*>(settings, "charset", "")
      );
  #else
      throw boost_ext::runtime_error("libcrails-odb was built without support for `mysql`");
      return nullptr;
  #endif
    }

    inline odb::database* initialize_for_postgresql(const Crails::Databases::DatabaseSettings& settings)
    {
  #ifdef CRAILS_ODB_WITH_PGSQL
      return new odb::pgsql::database(
        Crails::any_cast(settings.at("user")),
        Crails::any_cast(settings.at("password")),
        Crails::any_cast(settings.at("name")),
        defaults_to<const char*> (settings, "host",  ""),
        defaults_to<unsigned int>(settings, "port",  0),
        defaults_to<const char*> (settings, "extra", "")
      );
  #else
      throw boost_ext::runtime_error("libcrails-odb was built without support for `pgsql`");
      return nullptr;
  #endif
    }

    inline odb::database* initialize_for_sqlite(const Crails::Databases::DatabaseSettings& settings)
    {
  #ifdef CRAILS_ODB_WITH_SQLITE
      return new odb::sqlite::database(
        Crails::any_cast(settings.at("name")),
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
      );
  #else
      throw boost_ext::runtime_error("libcrails-odb was built without support for `sqlite`");
      return nullptr;
  #endif
    }

    inline odb::database* initialize_for_oracle(const Crails::Databases::DatabaseSettings& settings)
    {
  #ifdef CRAILS_ODB_WITH_ORACLE
      return new odb::oracle::database(
        Crails::any_cast(settings.at("user")),
        Crails::any_cast(settings.at("password")),
        Crails::any_cast(settings.at("name")),
        defaults_to<const char*> (settings, "host", ""),
        defaults_to<unsigned int>(settings, "port", 0)
      );
  #else
      throw boost_ext::runtime_error("libcrails-odb was built without support for `oracle`");
      return nullptr;
  #endif
    }
  }
}

#endif

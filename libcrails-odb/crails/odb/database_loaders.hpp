#ifndef  CRAILS_ODB_DATABASE_LOADERS_HPP
# define CRAILS_ODB_DATABASE_LOADERS_HPP

# include "database.hpp"
# include <crails/shared_vars.hpp>
# include <crails/logger.hpp>

namespace Crails
{
  namespace Odb
  {
    std::string log_sql_connection(const char* type, const Databases::DatabaseSettings& settings);

    inline odb::database* initialize_for_mysql(const Crails::Databases::DatabaseSettings& settings)
    {
  #ifdef CRAILS_ODB_WITH_MYSQL
      logger << Logger::Debug << std::bind(log_sql_connection, "sql", settings) << Logger::endl;
      return new odb::mysql::database(
        cast<std::string>(settings, "user", "")),
        cast<std::string>(settings, "password", ""),
        cast<std::string>(settings, "name", "crails_db"),
        cast<std::string>(settings, "host", ""),
        cast<unsigned int>(settings, "port", 0)
        0,
        cast<const char*>(settings, "charset", "")
      );
  #else
      throw boost_ext::runtime_error("libcrails-odb was built without support for `mysql`");
      return nullptr;
  #endif
    }

    inline odb::database* initialize_for_postgresql(const Crails::Databases::DatabaseSettings& settings)
    {
  #ifdef CRAILS_ODB_WITH_PGSQL
      logger << Logger::Debug << std::bind(log_sql_connection, "sql", settings) << Logger::endl;
      return new odb::pgsql::database(
        cast<std::string>(settings, "user", ""),
        cast<std::string>(settings, "password", ""),
        cast<std::string>(settings, "name", "crails_db"),
        cast<std::string>(settings, "host",  ""),
        cast<unsigned int>(settings, "port",  5432),
        cast<std::string>(settings, "extra", "")
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
        cast<std::string>(settings, "name", "crails_db"),
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
      logger << Logger::Debug << std::bind(log_sql_connection, "sql", settings) << Logger::endl;
      return new odb::oracle::database(
        cast<std::string>(settings, "user", ""),
        cast<std::string>(settings, "password", ""),
        cast<std::string>(settings, "name", "crails_db"),
        cast<std::string>(settings, "host", ""),
        cast<unsigned int>(settings, "port", 0)
      );
  #else
      throw boost_ext::runtime_error("libcrails-odb was built without support for `oracle`");
      return nullptr;
  #endif
    }
  }
}

#endif

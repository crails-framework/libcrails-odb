#ifndef  CRAILS_ODB_DATABASE_SETTINGS_HPP
# define CRAILS_ODB_DATABASE_SETTINGS_HPP

# include <crails/databases.hpp>

namespace Crails
{
  namespace Odb
  {
    extern const std::string default_configuration_name;
    Crails::Databases::DatabaseSettings get_database_settings_for(const std::string& name);
  }
}

#endif

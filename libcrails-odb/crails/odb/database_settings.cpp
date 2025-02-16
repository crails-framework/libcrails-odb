#include "database_settings.hpp"
#include <crails/environment.hpp>
#include <crails/shared_vars.hpp>

using namespace Crails;

namespace Crails
{
  namespace Odb
  {
    const std::string default_configuration_name = "odb";

    std::string log_sql_connection(const char* type, const Databases::DatabaseSettings& settings)
    {
      return std::string("odb/") + type + ": Connecting to "
        + Crails::cast<std::string>(settings, "name", "?")
        + " with user '"
        + Crails::cast<std::string>(settings, "user", "")
        + "' and password '"
        + Crails::cast<std::string>(settings, "password", "")
        + "'";
    }
  }
}

Databases::DatabaseSettings Odb::get_database_settings_for(const std::string& name)
{
  try {
    const Databases::DatabaseSettings& settings =
      Databases::Settings::singleton::require()
        .at(Crails::environment)
        .at(default_configuration_name);

    return {
      { "type",     Crails::cast<std::string>(settings, "type", "sqlite") },
      { "host",     Crails::cast<std::string>(settings, "host", "localhost") },
      { "name",     name },
      { "user",     Crails::cast<std::string>(settings, "user", "") },
      { "password", Crails::cast<std::string>(settings, "password", "") },
      { "port",     Crails::cast<unsigned int>(settings, "port", 5432) }
    };
  }
  catch (const std::exception& error) {
    throw Databases::Exception("Database not found '" + name + '\'');
  }
}

#include "database_settings.hpp"
#include <crails/environment.hpp>
#include <crails/any_cast.hpp>

using namespace Crails;

namespace Crails
{
  namespace Odb
  {
    const std::string default_configuration_name = "odb";

    std::string log_sql_connection(const char* type, const Databases::DatabaseSettings& settings)
    {
      return std::string("odb/") + type + ": Connecting to "
        + Crails::defaults_to<std::string>(settings, "name", "?")
        + " with user '"
        + Crails::defaults_to<std::string>(settings, "user", "")
        + "' and password '"
        + Crails::defaults_to<std::string>(settings, "password", "")
        + "'";
    }
  }
}

Databases::DatabaseSettings Odb::get_database_settings_for(const std::string& name)
{
  try {
    const Databases::DatabaseSettings& settings =
      Databases::settings
        .at(Crails::environment)
        .at(default_configuration_name);

    return {
      { "type",     Crails::defaults_to<std::string>(settings, "type", "sqlite") },
      { "host",     Crails::defaults_to<std::string>(settings, "host", "localhost") },
      { "name",     name },
      { "user",     Crails::defaults_to<std::string>(settings, "user", "") },
      { "password", Crails::defaults_to<std::string>(settings, "password", "") },
      { "port",     Crails::defaults_to<unsigned int>(settings, "port", 5432) }
    };
  }
  catch (const std::exception& error) {
    throw Databases::Exception("Database not found '" + name + '\'');
  }
}

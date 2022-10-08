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
        + Crails::any_cast("name")
        + " with user '"
        + Crails::any_cast(settings.at("user"))
        + "' and password '"
        + Crails::any_cast(settings.at("password"))
        + "'";
    }
  }
}

Databases::DatabaseSettings Odb::get_database_settings_for(const std::string& name)
{
  const auto& settings = Databases::settings
    .at(Crails::environment)
    .at(default_configuration_name);

  return {
    { "type",     settings.at("type") },
    { "host",     settings.at("host") },
    { "name",     name },
    { "user",     settings.at("user") },
    { "password", settings.at("password") },
    { "port",     settings.at("port") }
  };
}

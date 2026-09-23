#pragma once
#include <crails/databases.hpp>
#include <crails/odb/database.hpp>

class TestDatabases : public Crails::Databases::Settings
{
  SINGLETON_IMPLEMENTATION(TestDatabases, Crails::Databases::Settings)
  static const Crails::Databases::SettingsMap settings;
public:
  TestDatabases() : Crails::Databases::Settings(settings)
  {
  }
};

const Crails::Databases::SettingsMap TestDatabases::settings = {
  {
    Crails::Test, {
      {
        "odb", {
          { "type", "sqlite" },
          { "name", ":memory:" }
        }
      }
    }
  }
};

static void create_test_schema()
{
  Crails::Odb::Database& database = CRAILS_DATABASE(Crails::Odb, "odb");

  database.migrate();
}

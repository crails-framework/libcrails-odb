#include "database.hpp"
#include <crails/logger.hpp>
#include <crails/any_cast.hpp>
#include <boost/lexical_cast.hpp>
#include <list>
#include <sys/wait.h>

using namespace std;
using namespace boost;
using namespace Crails;
using namespace Crails::Odb;

#ifdef CRAILS_ODB_WITH_MYSQL

static string mysql_command_prefix(const Crails::Databases::DatabaseSettings& database_config, std::string user, std::string password)
{
  string command("mysql");

  command += " -h " + string(Any<string>::cast(database_config.at("host")));
  if (database_config.count("port"))
    command += " -P " + lexical_cast<string>(any_cast<unsigned int>(database_config.at("port")));
  if (user.length())
    command += " -u " + user;
  if (password.length())
    command += " -p\"" + password + "\"";
  command += " -e ";

  return command;
}

static void initialize_credentials(const Crails::Databases::DatabaseSettings& database_config, std::string& db_user, std::string& db_password, std::string& user, std::string& password)
{
  if (database_config.count("user"))
    db_user = Any<string>::cast(database_config.at("user"));
  if (database_config.count("password"))
    db_password = Any<string>::cast(database_config.at("password"));
  if (user == "")
  {
    user     = db_user;
    password = db_password;
  }
}

static bool mysql_run_queries(const Crails::Databases::DatabaseSettings& database_config, std::string user, std::string password, std::list<std::string> queries, bool ignore_failure = false)
{
  string db_user, db_password;
  string db_name = Any<string>::cast(database_config.at("name"));
  string command;

  initialize_credentials(database_config, db_user, db_password, user, password);
  command = mysql_command_prefix(database_config, user, password);
  for (string query : queries)
  {
    string full_command(command);
    int    status;

    full_command += '"' + query + '"';
    if (logger.get_log_level() > Logger::Info)
      full_command += " > /dev/null 2>&1";
    logger << Logger::Info << ":: running query " << query << Logger::endl;
    logger << Logger::Debug << ":: command " << full_command << Logger::endl;
    status = std::system(full_command.c_str());
    if (ignore_failure == false)
    {
      if (status < 0)
      {
        logger << Logger::Debug << ":: failed to run command" << Logger::endl;
        return false;
      }
      else
      {
        if (WIFEXITED(status))
        {
          if (WEXITSTATUS(status) != 0)
          {
            logger << Logger::Debug << ":: command returned with exit status " << WEXITSTATUS(status) << Logger::endl;
            return false;
          }
        }
        else
        {
          logger << Logger::Debug << ":: command didn't return" << Logger::endl;
          return false;
        }
      }
    }
  }
  return true;
}

bool mysql_create_from_settings(const Crails::Databases::DatabaseSettings& database_config, std::string user, std::string password)
{
  string db_user, db_password;
  string db_name = Any<string>::cast(database_config.at("name"));
  list<string> queries;

  initialize_credentials(database_config, db_user, db_password, user, password);
  if (db_user != user && db_user.length() > 0)
  {
    queries.push_back("CREATE USER IF NOT EXISTS '" + db_user + "'@'%' IDENTIFIED BY '" + db_password + "';");
    mysql_run_queries(database_config, user, password, queries, true);
    queries.clear();
  }
  queries.push_back("CREATE DATABASE IF NOT EXISTS `" + db_name + "`;");
  if (db_user != user && db_user.length() > 0)
  {
    queries.push_back("GRANT ALL PRIVILEGES ON `" + db_name + "`.* TO '" + db_user + "'@'%';");
    queries.push_back("FLUSH PRIVILEGES;");
  }
  return mysql_run_queries(database_config, user, password, queries);
}

bool mysql_drop_from_settings(const Crails::Databases::DatabaseSettings& database_config, std::string user, std::string password)
{
  string db_name = Any<string>::cast(database_config.at("name"));
  list<string> queries;

  queries.push_back("DROP DATABASE IF EXISTS `" + db_name + "`;");
  return mysql_run_queries(database_config, user, password, queries);
}

#endif

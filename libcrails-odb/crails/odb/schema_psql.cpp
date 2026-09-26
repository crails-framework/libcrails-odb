#include "schema.hpp"
#ifdef CRAILS_ODB_WITH_PGSQL
#include "connection.hpp"
#include <crails/logger.hpp>
#include <odb/database.hxx>
#include <odb/pgsql/database.hxx>
#include <odb/pgsql/connection.hxx>
#include <postgresql/libpq-fe.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace Crails::Odb::SchemaMigrator
{
  static string to_lower(string value)
  {
    transform(value.begin(), value.end(), value.begin(),
      [](unsigned char c) { return tolower(c); });
    return value;
  }

  // Converts SQL declared types into whatever Postgres turns them into
  static string normalize_type(const string& sql_type)
  {
    string type = to_lower(sql_type);

    if (type == "bigserial")   return "bigint";
    if (type == "serial")      return "integer";
    if (type == "smallserial") return "smallint";
    return type;
  }

  static PGconn* native_handle(odb::database& db)
  {
    odb::pgsql::database& pg_db = dynamic_cast<odb::pgsql::database&>(db);
    odb::connection_ptr   conn  = pg_db.connection();
    odb::pgsql::connection& pg_conn = dynamic_cast<odb::pgsql::connection&>(*conn);

    return pg_conn.handle();
  }

  static vector<vector<string>> select(PGconn* handle, const string& sql)
  {
    logger << Logger::Debug << "[SchemaMigrator] Executing: " << sql << Logger::endl;
    vector<vector<string>> rows;
    PGresult* result = PQexec(handle, sql.c_str());

    if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
      string error = PQresultErrorMessage(result);
      PQclear(result);
      logger << Logger::Debug << "[SchemaMigrator] Introspection query failed: " << error << Logger::endl;
      return rows;
    }

    int row_count = PQntuples(result), column_count = PQnfields(result);

    for (int r = 0 ; r < row_count ; ++r)
    {
      vector<string> row;

      for (int c = 0 ; c < column_count ; ++c)
        row.push_back(PQgetisnull(result, r, c) ? string() : string(PQgetvalue(result, r, c)));
      rows.push_back(row);
    }
    PQclear(result);
    return rows;
  }

  static bool table_exists(PGconn* handle, const string& table)
  {
    string sql =
      "SELECT 1 FROM information_schema.tables"
      " WHERE table_schema = 'public' AND table_name = '" + table + "';";

    return !select(handle, sql).empty();
  }

  // name -> postgres-reported data_type (lowercase, e.g. "bigint", "text")
  static map<string, string> existing_columns(PGconn* handle, const string& table)
  {
    map<string, string> columns;
    string sql =
      "SELECT column_name, data_type FROM information_schema.columns"
      " WHERE table_schema = 'public' AND table_name = '" + table + "';";

    for (const auto& row : select(handle, sql))
      columns[row[0]] = to_lower(row[1]);
    return columns;
  }

  static string build_create_table(const Table& table)
  {
    ostringstream sql;

    sql << "CREATE TABLE \"" << table.name << "\" (";
    for (size_t i = 0 ; i < table.columns.size() ; ++i)
    {
      const Column& column = table.columns[i];

      if (i > 0) sql << ", ";
      sql << '"' << column.name << "\" " << column.sql_type;
      if (column.name == table.primary_key)
        sql << " NOT NULL PRIMARY KEY";
      else
      {
        if (column.not_null)
          sql << " NOT NULL";
        if (!column.default_expr.empty())
          sql << " DEFAULT " << column.default_expr;
      }
    }
    sql << ");";
    return sql.str();
  }

  static string build_add_column(const string& table_name, const Column& column)
  {
    ostringstream sql;
    bool          not_null = column.not_null;

    sql << "ALTER TABLE \"" << table_name << "\" ADD COLUMN \""
        << column.name << "\" " << column.sql_type;
    if (not_null && column.default_expr.empty())
    {
      logger << Logger::Error
           << "[SchemaMigrator] warning: '" << table_name << "." << column.name
           << "' is declared NOT NULL with no default: adding it as nullable." << Logger::endl;
      not_null = false;
    }
    if (!column.default_expr.empty())
      sql << " DEFAULT " << column.default_expr;
    if (not_null)
      sql << " NOT NULL";
    sql << ";";
    return sql.str();
  }

  // Does not work on ADD CONSTRAINT, hence why execute_recoverable is needed
  static string make_index_idempotent(const string& sql)
  {
    static const regex create_index(
      R"(^(CREATE\s+(?:UNIQUE\s+)?INDEX\s+)(?!IF\s+NOT\s+EXISTS))", regex::icase
    );
    return regex_replace(sql, create_index, "$1IF NOT EXISTS ", regex_constants::format_first_only);
  }

  // Checks if an error is just an "already exists" and not worth stopping for
  static bool is_benign_already_exists(const string& what)
  {
    return what.find("42P07") != string::npos
        || what.find("42710") != string::npos
        || what.find("42701") != string::npos;
  }

  static string extract_constraint_name(const string& sql)
  {
    static const regex re(R"re(ADD\s+CONSTRAINT\s+"([^"]+)")re", regex::icase);
    smatch match;

    if (regex_search(sql, match, re))
      return match[1];
    return "";
  }

  static string extract_alter_table_name(const string& sql)
  {
    static const regex re(R"re(^\s*ALTER\s+TABLE\s+"([^"]+)")re", regex::icase);
    smatch match;

    if (regex_search(sql, match, re))
      return match[1];
    return "";
  }

  static bool constraint_exists(PGconn* handle, const string& table, const string& name)
  {
    string sql =
      "SELECT 1 FROM pg_constraint"
      " WHERE conname = '" + name + "'"
      " AND conrelid = '\"" + table + "\"'::regclass;";

    return !select(handle, sql).empty();
  }

  static void execute_recoverable(Crails::Odb::Connection& database, PGconn* handle, const string& sql)
  {
    PGresult* savepoint = PQexec(handle, "SAVEPOINT schema_migrator;");
    PQclear(savepoint);

    try
    {
      logger << Logger::Debug << "[SchemaMigrator] Executing: " << sql << Logger::endl;
      database.execute(sql);

      PGresult* release = PQexec(handle, "RELEASE SAVEPOINT schema_migrator;");
      PQclear(release);
    }
    catch (const exception& e)
    {
      PGresult* rollback = PQexec(handle, "ROLLBACK TO SAVEPOINT schema_migrator;");
      PQclear(rollback);
      PGresult* release = PQexec(handle, "RELEASE SAVEPOINT schema_migrator;");
      PQclear(release);

      if (is_benign_already_exists(e.what()))
        logger << Logger::Info << "[SchemaMigrator] already applied, skipping: " << sql << Logger::endl;
      else
        throw;
    }
  }

  void pgsql_sync(Crails::Odb::Connection& database, const vector<Table>& schema)
  {
    database.transaction().require("odb");

    odb::database& db     = database.transaction().get_database();
    PGconn*        handle = native_handle(db);

    // First pass: creates tables and columns
    for (const Table& table : schema)
    {
      if (!table_exists(handle, table.name))
        execute_recoverable(database, handle, build_create_table(table));
      else
      {
        map<string, string> live_columns = existing_columns(handle, table.name);

        for (const Column& column : table.columns)
        {
          auto found = live_columns.find(column.name);

          if (found == live_columns.end())
            execute_recoverable(database, handle, build_add_column(table.name, column));
          else
          {
            string live = found->second;
            string want = normalize_type(column.sql_type);

            if (live.find(want) == string::npos && want.find(live) == string::npos)
              logger << Logger::Error
                   << "[SchemaMigrator] warning: '" << table.name << "." << column.name
                   << "' is " << live << " in database but " << column.sql_type
                   << " in the schema. Update was not applied." << Logger::endl;
          }
        }
      }
    }

    // Second pass: indexes, constraints, alteratinons
    for (const Table& table : schema)
    {
      for (const string& statement : table.extra_statements)
      {
        string constraint_name = extract_constraint_name(statement);
        string target_table = extract_alter_table_name(statement);

        if (target_table.empty())
          target_table = table.name;
        if (!constraint_name.empty() && constraint_exists(handle, target_table, constraint_name))
        {
          logger << Logger::Info << "[SchemaMigrator] already applied, skipping: " << statement << Logger::endl;
          continue;
        }
        execute_recoverable(database, handle, make_index_idempotent(statement));
      }
    }
  }
}

#endif

#include "schema.hpp"
#ifdef CRAILS_ODB_WITH_PGSQL
#include "connection.hpp"
#include <crails/logger.hpp>
#include <odb/database.hxx>
#include <odb/pgsql/database.hxx>
#include <odb/pgsql/connection.hxx>
#include <libpq-fe.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
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

  static PGconn* native_handle(odb::database& db)
  {
    odb::pgsql::database& pg_db = dynamic_cast<odb::pgsql::database&>(db);
    odb::connection_ptr   conn  = pg_db.connection();
    odb::pgsql::connection& pg_conn = dynamic_cast<odb::pgsql::connection&>(*conn);

    return pg_conn.handle();
  }

  static vector<vector<string>> select(PGconn* handle, const string& sql)
  {
    vector<vector<string>> rows;
    PGresult* result = PQexec(handle, sql.c_str());

    if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
      string error = PQresultErrorMessage(result);
      PQclear(result);
      throw runtime_error("SchemaMigrator: introspection query failed: " + error);
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

  void pgsql_sync(Crails::Odb::Connection& database, const vector<Table>& schema)
  {
    database.transaction.require("odb");
    odb::database& db     = database.transaction.get_database();
    PGconn*        handle = native_handle(db);

    for (const Table& table : schema)
    {
      if (!table_exists(handle, table.name))
        database.execute(build_create_table(table));
      else
      {
        map<string, string> live_columns = existing_columns(handle, table.name);

        for (const Column& column : table.columns)
        {
          auto found = live_columns.find(column.name);

          if (found == live_columns.end())
            database.execute(build_add_column(table.name, column));
          else if (found->second.find(to_lower(column.sql_type)) == string::npos
                && to_lower(column.sql_type).find(found->second) == string::npos)
          {
            logger << Logger::Error
                 << "[SchemaMigrator] warning: '" << table.name << "." << column.name
                 << "' is " << found->second << " in database but " << column.sql_type
                 << " in the schema. Update was not applied." << Logger::endl;
          }
        }
      }

      for (const string& statement : table.extra_statements)
      {
        try { database.execute(statement); }
        catch (const exception& e)
        {
          logger << Logger::Error << "[SchemaMigrator] " << e.what() << Logger::endl;
        }
      }
    }
  }
}

#endif

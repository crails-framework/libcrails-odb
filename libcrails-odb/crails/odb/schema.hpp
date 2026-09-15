#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "backends.hpp"

namespace Crails
{
  namespace Odb
  {
    class Connection;

    namespace SchemaMigrator
    {
      struct Column
      {
        std::string name;
        std::string sql_type;
        bool        not_null   = true;
        std::string default_expr;  // leave empty for none
      };

      struct Table
      {
        std::string               name;
        std::string               primary_key = "id";
        std::vector<Column>       columns;
        std::vector<std::string>  extra_statements; // indexes, foreign keys, constraints, etc
        static Table              from_create_query(const std::string_view);
      };

#ifdef CRAILS_ODB_WITH_PGSQL
      void pgsql_sync(Crails::Odb::Connection& database, const std::vector<Table>& schema);
#endif
    }
  }
}

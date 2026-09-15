#include "schema.hpp"
#include <cctype>
#include <stdexcept>

using namespace std;

namespace Crails::Odb::SchemaMigrator
{
  static string_view trim(const string_view s)
  {
    size_t a = s.find_first_not_of(" \t\r\n");
 
    if (a == string_view::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
  }
 
  static bool case_insensitive_starts_with(const string_view s, const string_view keyword)
  {
    if (s.length() < keyword.length())
      return false;
    for (size_t i = 0 ; i < keyword.length() ; ++i)
      if (tolower((unsigned char)s[i]) != tolower((unsigned char)keyword[i]))
        return false;
    // "NULLABLE" shouldn't match "NULL"
    if (s.length() > keyword.length() && (isalnum((unsigned char)s[keyword.length()]) || s[keyword.length()] == '_'))
      return false;
    return true;
  }
 
  static size_t find_matching_parenthesis(const string_view s, size_t open_index)
  {
    int  depth     = 0;
    bool in_string = false;
 
    for (size_t i = open_index ; i < s.length() ; ++i)
    {
      char c = s[i];
 
      if (in_string)
      {
        if (c == '\'')
        {
          if (i + 1 < s.length() && s[i + 1] == '\'') { ++i; continue ; }
          in_string = false;
        }
        continue ;
      }
      if (c == '\'')      { in_string = true; continue ; }
      else if (c == '(')  ++depth;
      else if (c == ')')
      {
        --depth;
        if (depth == 0) return i;
      }
    }
    throw runtime_error("SchemaMigrator: unbalanced parentheses while parsing CREATE TABLE");
  }
 
  // Splits string on top-level occurrences of delimiter (outside parenthesis or quotes)
  static vector<string_view> split_top_level(const string_view s, char delim)
  {
    vector<string_view> parts;
    int                 depth     = 0;
    bool                in_string = false;
    size_t              start     = 0;
 
    for (size_t i = 0 ; i < s.length() ; ++i)
    {
      char c = s[i];
 
      if (in_string)
      {
        if (c == '\'')
        {
          if (i + 1 < s.length() && s[i + 1] == '\'') { ++i; continue ; }
          in_string = false;
        }
        continue;
      }
      if (c == '\'')            { in_string = true; continue ; }
      else if (c == '(')        ++depth;
      else if (c == ')')        --depth;
      else if (c == delim && depth == 0)
      {
        parts.push_back(s.substr(start, i - start));
        start = i + 1;
      }
    }
    parts.push_back(s.substr(start));
    return parts;
  }

  static bool parse_column_rest(Column& column, string_view rest, string& inline_primary_key)
  {
    static const vector<string> default_stop_words = {
      "NOT NULL", "NULL", "PRIMARY KEY", "UNIQUE", "REFERENCES", "CHECK"
    };

    while (!rest.empty())
    {
      if (case_insensitive_starts_with(rest, "NOT NULL"))
      {
        column.not_null = true;
        rest = trim(rest.substr(8));
      }
      else if (case_insensitive_starts_with(rest, "NULL"))
      {
        column.not_null = false;
        rest = trim(rest.substr(4));
      }
      else if (case_insensitive_starts_with(rest, "PRIMARY KEY"))
      {
        inline_primary_key = column.name;
        rest = trim(rest.substr(11));
      }
      else if (case_insensitive_starts_with(rest, "DEFAULT"))
      {
        int    depth     = 0;
        bool   in_string = false;
        size_t i         = 0;

        rest = trim(rest.substr(7));
        for ( ; i < rest.length() ; ++i)
        {
          char c = rest[i];

          if (in_string)
          {
            if (c == '\'')
            {
              if (i + 1 < rest.length() && rest[i + 1] == '\'') { ++i; continue ; }
              in_string = false;
            }
            continue ;
          }
          if (c == '\'')      { in_string = true; continue ; }
          else if (c == '(')  { ++depth; continue ; }
          else if (c == ')')  { --depth; continue ; }
          else if (depth == 0)
          {
            bool at_stop_word = false;

            for (const string& stop : default_stop_words)
              if (case_insensitive_starts_with(rest.substr(i), stop)) { at_stop_word = true; break ; }
            if (at_stop_word) break ;
          }
        }
        column.default_expr = trim(rest.substr(0, i));
        rest = trim(rest.substr(i));
      }
      else
      {
        return false;
      }
    }
    return true;
  }
 
  static Column parse_column(const string_view raw, string& inline_primary_key)
  {
    string_view def = trim(raw);
    string_view type_name, type_args, rest;
    size_t name_end;
    size_t type_len = 0;
    Column column;
 
    if (def.empty() || def[0] != '"')
      throw runtime_error("SchemaMigrator: expected a quoted column name in: " + string(raw));
    name_end = def.find('"', 1);
    if (name_end == string::npos)
      throw runtime_error("SchemaMigrator: unterminated quoted identifier in: " + string(raw));
    column.name     = string(def.substr(1, name_end - 1));
    column.not_null = false; // SQL default when neither NULL nor NOT NULL is stated
    rest            = trim(def.substr(name_end + 1));
 
    // Type name: leading identifier characters...
    while (type_len < rest.length() && (isalnum((unsigned char)rest[type_len]) || rest[type_len] == '_'))
      ++type_len;
    if (type_len == 0)
      throw runtime_error("SchemaMigrator: missing column type in: " + string(raw));
 
    type_name = rest.substr(0, type_len);
    rest = rest.substr(type_len);
 
    // ...optionally immediately followed by (args), e.g. NUMERIC(10,2)
    if (!rest.empty() && rest[0] == '(')
    {
      size_t close = find_matching_parenthesis(rest, 0);
      type_args = rest.substr(0, close + 1);
      rest = rest.substr(close + 1);
    }
    column.sql_type = string(type_name) + string(type_args);
    rest = trim(rest);
    if (!parse_column_rest(column, rest, inline_primary_key))
      throw runtime_error("SchemaMigrator: unrecognized column modifier near '" + string(rest) + "' in: " + string(raw));
    return column;
  }
 
  Table Table::from_create_query(const string_view sql)
  {
    Table table;
    string s = string(trim(sql));
    size_t name_end;
    string_view rest;
 
    while (!s.empty() && (s.back() == ';' || isspace((unsigned char)s.back())))
      s.pop_back();
    if (!case_insensitive_starts_with(s, "CREATE TABLE"))
      throw runtime_error("SchemaMigrator: expected a CREATE TABLE statement");
    s = trim(s.substr(12));
    if (case_insensitive_starts_with(s, "IF NOT EXISTS"))
      s = trim(s.substr(13));
    if (s.empty() || s[0] != '"')
      throw runtime_error("SchemaMigrator: expected a quoted table name");
    name_end = s.find('"', 1);
    if (name_end == string::npos)
      throw runtime_error("SchemaMigrator: unterminated quoted table name");
 
    table.name = s.substr(1, name_end - 1);
    table.primary_key = ""; // set below only if an inline PRIMARY KEY column is found
    rest = trim(string_view(s).substr(name_end + 1));
 
    if (rest.empty() || rest[0] != '(')
      throw runtime_error("SchemaMigrator: expected '(' after table name in CREATE TABLE \"" + table.name + "\"");

    size_t close         = find_matching_parenthesis(rest, 0);
    string_view body     = rest.substr(1, close - 1);
    string_view trailing = trim(rest.substr(close + 1));

    if (!trailing.empty())
      throw runtime_error("SchemaMigrator: unexpected content after CREATE TABLE body: " + string(trailing));

    for (const string_view raw_part : split_top_level(body, ','))
    {
      string_view part = trim(raw_part);
 
      if (part.empty())
        continue;
      if (part[0] == '"')
      {
        string inline_primary_key;
        Column column = parse_column(part, inline_primary_key);
 
        table.columns.push_back(column);
        if (!inline_primary_key.empty())
          table.primary_key = inline_primary_key;
      }
      else
      {
        table.extra_statements.push_back("ALTER TABLE \"" + table.name + "\" ADD " + string(part) + ";");
      }
    }
    if (table.columns.empty())
      throw runtime_error("SchemaMigrator: CREATE TABLE \"" + table.name + "\" has no columns");
    return table;
  }
}

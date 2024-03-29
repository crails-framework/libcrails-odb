#ifndef  CRAILS_ODB_ANY_HPP
# define CRAILS_ODB_ANY_HPP

# include <string>
# include <sstream>

namespace Crails
{
  namespace Odb
  {
    template<typename ARRAY>
    std::string array_to_string(const ARRAY& array, const std::string& sql_type = "")
    {
      std::stringstream stream;

      stream << "'{";
      for (auto it = array.begin() ; it != array.end() ; ++it)
      {
        if (it != array.begin())
          stream << ',';
        stream << *it;
      }
      stream << "}'";
      if (sql_type.length())
        stream << "::" << sql_type << "[]";
      return stream.str();
    }

    template<typename ARRAY>
    std::string any(const ARRAY& array, const std::string& sql_type = "")
    {
      return "ANY(" + array_to_string(array, sql_type) + ')';
    }
  }
}

#endif

#include <cstring> // std::memcpy
#include <vector>
#include <sstream>
#include <string>
#include <iomanip>

namespace odb
{
  namespace TRAITS_INCLUDE_SQL_BACKEND
  {
    template <>
    class value_traits<std::vector<std::string>, CRAILS_ODB_ID_STRING>
    {
    public:
      typedef std::vector<std::string> value_type;
      typedef value_type query_type;
      typedef details::buffer image_type;

      static void set_value(value_type& v,
                            const details::buffer& b,
                            std::size_t n,
                            bool is_null)
      {
        v.clear();
        if (!is_null)
        {
          std::string_view source(b.data(), n);

          if (source[0] == '{' && *source.rbegin() == '}')
          {
            std::istringstream ss(std::string(&source[1], source.length() - 1));
            std::string current_value;
            char end_char;

            while (ss.rdbuf()->in_avail())
            {
              ss >> std::quoted(current_value)
                 >> end_char;
              v.push_back(current_value);
              if (end_char != ',') break ;
            }
          }
        }
      }

      static void set_image (details::buffer& b,
                             std::size_t& n,
                             bool& is_null,
                             const value_type& v)
      {
        std::ostringstream os;

        is_null = false;
        os << '{';
        for (value_type::const_iterator i(v.begin()), e(v.end());
             i != e;)
        {
          os << std::quoted(*i);
          if (++i != e) os << ',';
        }
        os << '}';
        const std::string& s(os.str());
        n = s.size();
        if (n > b.capacity())
          b.capacity(n);
        std::memcpy(b.data(), s.c_str(), n);
      }
    };
  }
}

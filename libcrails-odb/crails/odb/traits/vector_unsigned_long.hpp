#include <cstring> // std::memcpy
#include <vector>
#include <sstream>

namespace odb
{
  namespace TRAITS_INCLUDE_SQL_BACKEND
  {
    template <>
    class value_traits<std::vector<unsigned long>, CRAILS_ODB_ID_STRING>
    {
    public:
      typedef std::vector<unsigned long> value_type;
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
          char c;
          std::string s(b.data(), n);
          std::istringstream is(s);
          is >> c; // '{'
          for (c = static_cast<char>(is.peek());
               c != '}';
               is >> c)
          {
            v.push_back(long ());
            is >> v.back();
          }
        }
      }

      static void set_image (details::buffer& b,
                             std::size_t& n,
                             bool& is_null,
                             const value_type& v)
      {
        is_null = false;
        std::ostringstream os;
        os << '{';
        for (value_type::const_iterator i(v.begin()), e(v.end());
             i != e;)
        {
          os << *i;
          if (++i != e)
            os << ',';
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


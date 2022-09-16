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
          bool done = false;
	  bool in_word = false;
          char c;
          std::string s(b.data(), n);
	  std::ostringstream word;
          std::istringstream is(s);
          is >> c; // '{'
          while (!done)
	  {
	    is >> c;
	    if (escaping)
              continue ;
	    switch (c)
	    {
            case '\\':
	      escaping = true;
	      break ;
            case '"':
	      if (in_word)
                v.push_back(word.str());
	      in_word = !in_word;
	      break ;
	    case '}':
              if (!in_word) done = true;
              break ;
	    default:
	      if (in_word) word << c;
	      break ;
	    }
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
          os << '"';
	  for (std::size_t ii = 0 ; ii != i->length() ; ++ii)
	  {
            char c = *i[ii];
	    if (c == '"' || c == '\\') os << '\\';
            os << *i[ii];
	  }
	  os << '"'
         if (++i != e) << ',';
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


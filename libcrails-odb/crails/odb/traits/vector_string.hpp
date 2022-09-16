#include <cstring> // std::memcpy
#include <vector>
#include <sstream>
#include <string>

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
          const std::string s(b.data(), n);
          for (std::size_t i = 0 ; i < s.length() ;)
          {
            switch (s[i])
            {
            case '"':
              v.push_back(output_escaped_word(&s[i], s.length() - i, i));
              break ;
            case '}':
              return ;
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
          input_escaped_word(os, i->c_str(), i->length());
          os << '"';
          if (++i != e) os << ',';
        }
        os << '}';
        const std::string& s(os.str());
        n = s.size();
        if (n > b.capacity())
          b.capacity(n);
        std::memcpy(b.data(), s.c_str(), n);
      }

    private:
      static void input_escaped_word(std::ostringstream& os, const char* word, std::size_t length)
      {
        for (std::size_t i = 0 ; i != length ; ++i)
        {
          char c = word[i];
          if (c == '"' || c == '\\') os << '\\';
          os << c;
        }
      }

      static std::string output_escaped_word(const char* buffer, std::size_t length, std::size_t& walked)
      {
        char c;
        bool escaping = false;
        std::ostringstream word;
        std::size_t i = 0;

        for (; i < length ; ++i)
        {
          char c = buffer[i];
          if (c == '\\' && !escaping)
            escaping = true;
          else if (c == '"' && !escaping)
            break ;
          else
          {
            word << c;
            escaping = false;
          }
        }
        walked += i + 1;
        return word.str();
      }
    };
  }
}

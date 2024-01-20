#ifndef  CRAILS_ODB_QUERY_STREAM_HPP
# define CRAILS_ODB_QUERY_STREAM_HPP

# include <sstream>
# include <iomanip>
# include <string>
# include <string_view>

namespace Crails
{
  namespace Odb
  {
    struct QueryStream
    {
      std::string_view next_fragment;
      std::stringstream stream;
      std::string output;

      void append_arg(const char* value) { stream << std::quoted(value); }
      void append_arg(const std::string value) { stream << std::quoted(value); }
      void append_arg(const std::string_view value) { stream << std::quoted(value); }
      void append_arg(const char value) { stream << '"' << value << '"'; }
      void append_arg(const bool value) { stream << (value ? '1' : '0'); }

      template<typename ARG>
      void append_arg(const ARG arg) { stream << arg; }

      template<typename ARG>
      void append(std::string_view query, ARG arg)
      {
        const std::string_view pattern("%%");
        std::size_t index = query.find(pattern);
        std::size_t end = index + pattern.length();

        if (index >= 0)
        {
          stream << std::string_view(query.data(), index);
          append_arg(arg);
          next_fragment = std::string_view(
            query.data() + sizeof(char) * end,
            query.length() - end
          );
        }
        else
          next_fragment = std::string_view();
      }

      template<typename ARG, typename... ARGS>
      void append(std::string_view query, ARG arg, ARGS... args)
      { 
        append(query, arg);
        if (next_fragment.length() > 0)
          append(next_fragment, args...);
      }


      template<typename... ARGS>
      void make(std::string_view query, ARGS... args)
      {
        append(query, args...);
        output = stream.str();
      }
    };
  }
}

#endif

#ifndef  CRAILS_ODB_EXCEPTION_HPP
# define CRAILS_ODB_EXCEPTION_HPP

# include <crails/utils/backtrace.hpp>
# include <odb/exception.hxx>
# include <sstream>
# include <iostream> 

namespace Crails
{
  namespace Odb
  {
    class Exception : public boost_ext::exception
    {
    public:
      explicit Exception(const odb::exception& exception) : message(exception.what())
      {
      }

      explicit Exception(std::string&& message) : message(std::move(message))
      {
      }

      const char* what() const noexcept override
      {
        return message.c_str();
      }

    private:
      std::string message;
    };

    template<typename MODEL>
    void throw_exception(const MODEL& model, const std::string& what)
    {
      std::ostringstream stream;

      stream << "object(" << model.get_id() << "): ";
      stream << what;
      throw Odb::Exception(stream.str().c_str());
    }
  }
}

#endif

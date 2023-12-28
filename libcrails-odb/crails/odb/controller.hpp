#ifndef  CRAILS_ODB_CONTROLLER_HPP
# define CRAILS_ODB_CONTROLLER_HPP

# include <crails/controller.hpp>
# include <crails/http_response.hpp>
# include <crails/logger.hpp>
# include "connection.hpp"

namespace Crails
{
  namespace Odb
  {
    template<class SUPER = Crails::Controller, class DATABASE = Odb::Connection>
    class Controller : public SUPER
    {
    protected:
      DATABASE database;

    public:
      Controller(Context& context) : SUPER(context)
      {
      }

      virtual void finalize() override
      {
        auto status = static_cast<int>(SUPER::response.get_status_code());

        if (status < 400)
        {
          logger << Logger::Debug << "Crails::Odb::finalize: status is " << status << ": committing changes." << Logger::endl;
          database.commit();
        }
      	else
          logger << Logger::Debug << "Crails::Odb::finalize: status is " << status << ": changes will rollback." << Logger::endl;
        SUPER::finalize();
      }
    };
  }
}

#endif

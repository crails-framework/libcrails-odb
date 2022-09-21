#ifndef  CRAILS_ODB_CONTROLLER_HPP
# define CRAILS_ODB_CONTROLLER_HPP

# include <crails/controller.hpp>
# include <crails/http_response.hpp>
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

      void finalize() override
      {
        SUPER::finalize();
        if (static_cast<int>(SUPER::response.get_status_code()) < 400)
          database.commit();
      }
    };
  }
}

#endif

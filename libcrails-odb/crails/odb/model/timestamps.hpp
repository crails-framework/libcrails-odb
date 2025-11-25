#ifndef  CRAILS_ODB_MODEL_TIMESTAMPS_HPP
# define CRAILS_ODB_MODEL_TIMESTAMPS_HPP

# include "soft_delete.hpp"
# include <ctime>

namespace Crails
{
  namespace Odb
  {
    #pragma db object abstract
    class ModelTimestamps : public ModelSoftDelete
    {
      friend class odb::access;
      typedef ModelSoftDelete BaseType;
    public:
      std::time_t get_created_at() const { return created_at; }
      std::time_t get_updated_at() const { return updated_at; }
      void set_created_at(std::time_t value) { created_at = value; }
      void set_updated_at(std::time_t value) { updated_at = value; }

#ifndef __COMET_CLIENT__
      void update_timestamps();

      void save(odb::database&);
      void destroy(odb::database&);
#endif

    private:
      #pragma db null
      std::time_t created_at;
      #pragma db null
      std::time_t updated_at;
    };
  }
}

#endif

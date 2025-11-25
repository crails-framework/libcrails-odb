#ifndef  CRAILS_ODB_MODEL_SOFT_DELETE_HPP
# define CRAILS_ODB_MODEL_SOFT_DELETE_HPP

# include "base.hpp"

namespace Crails
{
  namespace Odb
  {
#ifdef __COMET_CLIENT__
    typedef ModelBase ModelSoftDelete;
#else
    # pragma db object abstract
    class ModelSoftDelete : public ModelBase
    {
      friend class odb::access;
      typedef ModelBase BaseType;
    public:
      virtual bool with_soft_delete() const { return false; }
      bool is_deleted() const { return deleted; }

      void save(odb::database&);
      void destroy(odb::database&);

    private:
      #pragma db default(false)
      bool deleted;
    };
#endif
  }
}

#endif

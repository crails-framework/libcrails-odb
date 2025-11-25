#ifndef  CRAILS_ODB_MODEL_BASE_HPP
# define CRAILS_ODB_MODEL_BASE_HPP

# include <string>
# include "../id_type.hpp"

# ifndef __COMET_CLIENT__
#  include <odb/database.hxx>
#  define odb_instantiable() \
  friend class odb::access; \
  virtual void odb_persist(odb::database&); \
  virtual void odb_update(odb::database&); \
  virtual void odb_erase(odb::database&);
#  define odb_instantiable_impl(TYPE) \
  void TYPE::odb_persist(odb::database& db) { db.persist(*this); } \
  void TYPE::odb_update(odb::database& db)  { db.update(*this);  } \
  void TYPE::odb_erase(odb::database& db)   { db.erase(*this);   }
# else
#  define odb_instantiable()
#  define odb_instantiable_impl(TYPE)
# endif

namespace odb { class access; }

namespace Crails
{
  namespace Odb
  {
    # pragma db object abstract
    class ModelBase
    {
      friend class odb::access;
    public:
      ModelBase() : id(0) {}
      ModelBase(id_type id) : id(id) {}

      id_type get_id() const { return id; }
      void    set_id(id_type id) { this->id = id; }
      bool    has_been_erased() const { return erased; }
      bool    is_persistent() const { return id != ODB_NULL_ID && !erased; }

# ifndef __COMET_CLIENT__
      virtual std::string get_database_name() const;
      void save(odb::database&);
      void destroy(odb::database&);
# endif

      virtual void before_save()    {}
      virtual void after_save()     {}
      virtual void before_destroy() {}
      virtual void after_destroy()  {}

    protected:
# ifndef __COMET_CLIENT__
      virtual void odb_persist(odb::database&) = 0;
      virtual void odb_update(odb::database&)  = 0;
      virtual void odb_erase(odb::database&)   = 0;
# endif

      #pragma db id auto
      id_type id;

    private:
      #pragma db transient
      bool erased = false;
    };
  }
}

#endif

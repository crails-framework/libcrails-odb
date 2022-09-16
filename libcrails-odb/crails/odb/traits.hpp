#ifndef  CRAILS_ODB_TRAITS_HPP
# define CRAILS_ODB_TRAITS_HPP

# define CRAILS_ODB_ID_STRING id_string
# include "backends.hpp"
# ifdef CRAILS_ODB_WITH_PGSQL
#  include <odb/pgsql/traits.hxx>
#  define TRAITS_INCLUDE_SQL_BACKEND pgsql
#  include "traits/all.hpp"
#  undef TRAITS_INCLUDE_SQL_BACKEND
# endif
# ifdef CRAILS_ODB_WITH_SQLITE
#  include <odb/sqlite/traits.hxx>
#  define TRAITS_INCLUDE_SQL_BACKEND sqlite
#  undef CRAILS_ODB_ID_STRING
#  define CRAILS_ODB_ID_STRING id_text
#  include "traits/all.hpp"
#  undef TRAITS_INCLUDE_SQL_BACKEND
# endif
# ifdef CRAILS_ODB_WITH_MYSQL
#  include <odb/mysql/traits.hxx>
#  define TRAITS_INCLUDE_SQL_BACKEND mysql
#  include "traits/all.hpp"
#  undef TRAITS_INCLUDE_SQL_BACKEND
# endif
# ifdef CRAILS_ODB_WITH_ORACLE
#  include <odb/oracle/traits.hxx>
#  define TRAITS_INCLUDE_SQL_BACKEND oracle
#  include "traits/all.hpp"
#  undef TRAITS_INCLUDE_SQL_BACKEND
# endif

#endif

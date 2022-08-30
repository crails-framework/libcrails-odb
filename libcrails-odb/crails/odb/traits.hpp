#ifndef  CRAILS_ODB_TRAITS_HPP
# define CRAILS_ODB_TRAITS_HPP

# include "backends.hpp"
# ifdef CRAILS_ODB_WITH_PGSQL
#  define TRAITS_INCLUDE_SQL_BACKEND pgsql
#  include "traits/datatree.hpp"
#  include "traits/vector_id.hpp"
#  undef TRAITS_INCLUDE_SQL_BACKEND
# endif
# ifdef CRAILS_ODB_WITH_SQLITE
#  define TRAITS_INCLUDE_SQL_BACKEND sqlite
#  include "traits/datatree.hpp"
#  include "traits/vector_id.hpp"
#  undef TRAITS_INCLUDE_SQL_BACKEND
# endif
# ifdef CRAILS_ODB_WITH_MYSQL
#  define TRAITS_INCLUDE_SQL_BACKEND mysql
#  include "traits/datatree.hpp"
#  include "traits/vector_id.hpp"
#  undef TRAITS_INCLUDE_SQL_BACKEND
# endif
# ifdef CRAILS_ODB_WITH_ORACLE
#  define TRAITS_INCLUDE_SQL_BACKEND oracle
#  include "traits/datatree.hpp"
#  include "traits/vector_id.hpp"
#  undef TRAITS_INCLUDE_SQL_BACKEND
# endif

#endif

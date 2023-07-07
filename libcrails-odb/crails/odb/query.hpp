#ifndef  CRAILS_ODB_QUERY_HPP
# define CRAILS_ODB_QUERY_HPP

# ifndef TWILIDOC
#  include "backends.hpp"
#  ifdef CRAILS_ODB_WITH_PGSQL
#   include <odb/pgsql/query.hxx>
#  endif
#  ifdef CRAILS_ODB_WITH_SQLITE
#   include <odb/sqlite/query.hxx>
#  endif
#  ifdef CRAILS_ODB_WITH_MYSQL
#   include <odb/mysql/query.hxx>
#  endif
#  ifdef CRAILS_ODB_WITH_ORACLE
#   include <odb/oracle/query.hxx>
#  endif
# endif

#endif

#ifndef  CRAILS_ODB_IMPORTS_HPP
# define CRAILS_ODB_IMPORTS_HPP

# include <odb/database.hxx>

# ifndef TWILIDOC
# include "backends.hpp"
# endif

# ifdef CRAILS_ODB_WITH_MYSQL
#  include <odb/mysql/database.hxx>
# endif

# ifdef CRAILS_ODB_WITH_PGSQL
#  include <odb/pgsql/database.hxx>
# endif

# ifdef CRAILS_ODB_WITH_SQLITE
#  include <odb/sqlite/database.hxx>
# endif

# ifdef CRAILS_ODB_WITH_ORACLE
#  include <odb/oracle/database.hxx>
# endif

#endif

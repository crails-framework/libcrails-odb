#ifndef  CRAILS_ODB_BACKENDS_HPP
# define CRAILS_ODB_BACKENDS_HPP

# define true  1
# define false 0

# if true
#  define CRAILS_ODB_WITH_PGSQL
# endif
# if true
#  define CRAILS_ODB_WITH_SQLITE
# endif
# if false
#  define CRAILS_ODB_WITH_MYSQL
# endif
# if false
#  define CRAILS_ODB_WITH_ORACLE
# endif

# undef true
# undef false

#endif


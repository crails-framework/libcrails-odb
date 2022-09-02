#ifndef  CRAILS_ODB_TO_VECTOR_HPP
# define CRAILS_ODB_TO_VECTOR_HPP
# ifndef ODB_COMPILER
#  include <vector>
#  include <odb/result.hxx>
namespace Crails
{
  namespace Odb
  {
    template<typename MODEL, typename CONTAINER = MODEL>
    std::vector<MODEL> to_vector(odb::result<CONTAINER>& results)
    {
      std::vector<MODEL> models;

      //models.reserve(results.size()); // would be nice, but does not work for all backends: https://www.codesynthesis.com/pipermail/odb-users/2012-June/000591.html
      std::copy(results.begin(), results.end(), std::back_inserter(models));
      return models;
    }
  }
}
# endif
#endif

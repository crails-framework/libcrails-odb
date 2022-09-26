#ifndef  MODEL_HELPERS_HPP
# define MODEL_HELPERS_HPP

# include <crails/utils/arrays.hpp>
# include <crails/datatree.hpp>
# include <list>
# include <memory>
# include <algorithm>
# include "id_type.hpp"
# include "to_vector.hpp"
# ifndef ODB_COMPILER
#  include "connection.hpp"
# endif

template<typename MODELS>
std::vector<Crails::Odb::id_type> collect_ids_from(const MODELS& models)
{
  typedef typename MODELS::value_type Model;
  return Crails::collect(models, &Model::get_id);
}

# ifndef CRAILS_FRONT_HELPERS
template<typename MODEL>
bool update_id_list(
  std::vector<Crails::Odb::id_type>& model_list,
  Data model_ids)
{
#  ifndef ODB_COMPILER
  Crails::Odb::Connection database;
  auto ids = Crails::unique_list<std::vector<Crails::Odb::id_type> >(model_ids);

  for (auto it = model_list.begin() ; it != model_list.end() ;)
  {
    auto exists_in_new_list = std::find(ids.begin(), ids.end(), *it);

    if (exists_in_new_list == ids.end())
      it = model_list.erase(it);
    else
    {
      ids.erase(exists_in_new_list);
      it++;
    }
  }

  for (Crails::Odb::id_type id : ids)
  {
    std::shared_ptr<MODEL> model;

    if (!database.find_one(model, odb::query<MODEL>::id == id))
      return false;
    model_list.push_back(id);
  }
#  endif
  return true;
}

template<typename MODEL>
bool update_id_list(
  std::list<std::shared_ptr<MODEL> >& model_list,
  Data model_ids)
{
#  ifndef ODB_COMPILER
  Crails::Odb::Connection database;
  std::vector<Crails::Odb::id_type> ids = model_ids;
  {
    auto it = model_list.begin();

    while (it != model_list.end())
    {
      std::shared_ptr<MODEL> model(*it);
      auto exists_in_new_list = std::find(ids.begin(), ids.end(), model->get_id());

      if (exists_in_new_list == ids.end())
        it = model_list.erase(it); // if it isn't in the new list, remove the building
      else
      {
        ids.erase(exists_in_new_list); // if it is in both lists, ignore it
        it++;
      }
    }
  }

  for (Crails::Odb::id_type id : ids)
  {
    std::shared_ptr<MODEL> model;

    if (!database.find_one(model, odb::query<MODEL>::id == id))
      return false;
    model_list.push_back(model);
  }
#  endif
  return true;
}
# endif

#endif

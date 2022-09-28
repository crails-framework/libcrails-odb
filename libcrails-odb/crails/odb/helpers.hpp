#ifndef  MODEL_HELPERS_HPP
# define MODEL_HELPERS_HPP

# include <crails/utils/arrays.hpp>
# include <crails/datatree.hpp>
# include <list>
# include <memory>
# include <algorithm>
# include "id_type.hpp"
# include "to_vector.hpp"
# if !defined(ODB_COMPILER) && !defined(__COMET_CLIENT__)
#  include "connection.hpp"
# endif

template<typename MODELS>
std::vector<Crails::Odb::id_type> collect_ids_from(const MODELS& models)
{
  typedef typename MODELS::value_type Model;
  return Crails::collect(models, &Model::get_id);
}

template<typename MODEL>
bool update_id_list(
  std::vector<Crails::Odb::id_type>& model_list,
  Data model_ids)
{
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

# if !defined(ODB_COMPILER) && !defined(__COMET_CLIENT__)
  Crails::Odb::Connection database;

  for (Crails::Odb::id_type id : ids)
  {
    std::shared_ptr<MODEL> model;

    if (!database.find_one(model, odb::query<MODEL>::id == id))
      return false;
    model_list.push_back(id);
  }
# else
  for (Crails::Odb::id_type id : ids)
    model_list.push_back(id);
# endif
  return true;
}

template<typename MODEL>
bool update_id_list(
  std::list<std::shared_ptr<MODEL> >& model_list,
  Data model_ids)
{
  auto ids = update_id_list(
    Crails::collect(model_list, &MODEL::get_id),
    model_ids
  );

# if !defined(ODB_COMPILER) && !defined(__COMET_CLIENT__)
  Crails::Odb::Connection database;

  for (Crails::Odb::id_type id : ids)
  {
    std::shared_ptr<MODEL> model;

    if (!database.find_one(model, odb::query<MODEL>::id == id))
      return false;
    model_list.push_back(model);
  }
# elif defined(__COMET_CLIENT__)
  for (Crails::Odb::id_type id : ids)
  {
    std::shared_ptr<MODEL> model = std::make_shared<MODEL>();

    model->set_id(id);
#  ifdef COMET_MODELS_AUTOFETCH
    model->fetch();
#  endif
    model_list.push_back(model);
  }
# endif
  return true;
}

#endif

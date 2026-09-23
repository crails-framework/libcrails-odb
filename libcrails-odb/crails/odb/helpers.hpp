#ifndef  MODEL_HELPERS_HPP
# define MODEL_HELPERS_HPP

# include <crails/utils/type_traits.hpp>
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

template<typename MODELS, typename ELEMENT = typename MODELS::value_type, bool is_pointer = std::is_pointer<ELEMENT>::value || Crails::is_specialization<ELEMENT, std::shared_ptr>::value>
struct OdbHelperIdCollector
{
  static std::vector<Crails::Odb::id_type> run(const MODELS& models)
  {
    typedef typename MODELS::value_type Model;
    return Crails::collect(models, &Model::get_id);
  }
};

template<typename MODELS, typename ELEMENT>
struct OdbHelperIdCollector<MODELS, ELEMENT, true>
{
  static std::vector<Crails::Odb::id_type> run(const MODELS& models)
  {
    typedef typename MODELS::value_type ModelPointer;
    typedef typename std::pointer_traits<ModelPointer>::element_type Model;
    return Crails::collect(models, &Model::get_id);
  }
};

template<typename MODELS>
std::vector<Crails::Odb::id_type> collect_ids_from(const MODELS& models)
{
  return OdbHelperIdCollector<MODELS>::run(models);
}

/*
 * Updates a list of IDs using a id array stored in a DataTree
 */
template<typename MODEL, typename LIST, bool IS_AN_ID_LIST = std::is_same<typename LIST::value_type, Crails::Odb::id_type>::value>
class IdListDataUpdater
{
  IdListDataUpdater() = delete;
public:
# if !defined(ODB_COMPILER) && !defined(__COMET_CLIENT__)
  static bool update_id_list(LIST& id_list, Data model_ids)
  {
    return update_id_list(Crails::Odb::ConnectionHandle(), id_list, model_ids);
  }

  static bool update_id_list(Crails::Odb::ConnectionHandle& database, LIST& id_list, Data model_ids)
  {
    LIST ids = Crails::unique_list<LIST>(model_ids);

    wipe_removed_ids(ids, id_list);
    return aggregate_new_ids(database, ids, id_list);
  }
# else
  static bool update_id_list(LIST& id_list, Data model_ids)
  {
    LIST ids = Crails::unique_list<LIST>(model_ids);

    wipe_removed_ids(ids, id_list);
    return aggregate_new_ids(ids, id_list);
  }
# endif

private:
  static void wipe_removed_ids(LIST& input, LIST& output)
  {
    for (auto it = output.begin() ; it != output.end() ;)
    {
      auto exists_in_new_list = std::find(input.begin(), input.end(), *it);

      if (exists_in_new_list == input.end())
        it = output.erase(it);
      else
      {
        input.erase(exists_in_new_list);
        it++;
      }
    }
  }

  # if !defined(ODB_COMPILER) && !defined(__COMET_CLIENT__)
  static bool aggregate_new_ids(const LIST& input, LIST& output)
  {
    Crails::Odb::ConnectionHandle database;
    return aggregate_new_ids(database, input, output);
  }

  static bool aggregate_new_ids(Crails::Odb::ConnectionHandle& database, const LIST& input, LIST& output)
  {
    for (Crails::Odb::id_type id : input)
    {
      std::shared_ptr<MODEL> model;

      if (!database.find_one(model, odb::query<MODEL>::id == id))
        return false;
      output.push_back(id);
    }
    return true;
  }
  # else
  static bool aggregate_new_ids(const LIST& input, LIST& output)
  {
    for (Crails::Odb::id_type id : input)
      output.push_back(id);
    return true;
  }
  # endif
};

/*
 * Updates a list of models using an id array stored in a DataTree
 */
template<typename MODEL, typename LIST>
class IdListDataUpdater<MODEL, LIST, false> // list is assumed to be a list of pointers to models
{
  IdListDataUpdater() = delete;
public:
  # if !defined(ODB_COMPILER) && !defined(__COMET_CLIENT__)
  static bool update_id_list(LIST& model_list, Data model_ids)
  {
    Crails::Odb::ConnectionHandle database;
    return update_id_list(database, model_list, model_ids);
  }

  static bool update_id_list(Crails::Odb::ConnectionHandle& database, LIST& model_list, Data model_ids)
  {
    auto ids = Crails::unique_list<std::vector<Crails::Odb::id_type>>(model_ids);

    wipe_removed_ids(ids, model_list);
    return aggregate_new_ids(database, ids, model_list);
  }
  # else
  static bool update_id_list(LIST& model_list, Data model_ids)
  {
    auto ids = Crails::unique_list<std::vector<Crails::Odb::id_type>>(model_ids);

    wipe_removed_ids(ids, model_list);
    return aggregate_new_ids(ids, model_list);
  }
  # endif

private:
  static void wipe_removed_ids(std::vector<Crails::Odb::id_type>& input, LIST& output)
  {
    for (auto it = output.begin() ; it != output.end() ;)
    {
      auto exists_in_new_list = std::find(input.begin(), input.end(), (*it)->get_id());

      if (exists_in_new_list == input.end())
        it = output.erase(it);
      else
      {
        input.erase(exists_in_new_list);
        it++;
      }
    }
  }

  # if !defined(ODB_COMPILER) && !defined(__COMET_CLIENT__)
  static bool aggregate_new_ids(const std::vector<Crails::Odb::id_type>& input, LIST& output)
  {
    Crails::Odb::ConnectionHandle database;
    return aggregate_new_ids(database, input, output);
  }

  static bool aggregate_new_ids(Crails::Odb::ConnectionHandle& database, const std::vector<Crails::Odb::id_type>& input, LIST& output)
  {
    for (Crails::Odb::id_type id : input)
    {
      std::shared_ptr<MODEL> model;

      if (!database.find_one(model, odb::query<MODEL>::id == id))
        return false;
      output.push_back(model);
    }
    return true;
  }
  # else
  static bool aggregate_new_ids(const std::vector<Crails::Odb::id_type>& input, LIST& output)
  {
    for (Crails::Odb::id_type id : input)
      output.push_back(nullptr);
    return true;
  }
  # endif
};

/*
 * Applies a Data value to a relationship property, regardless of whether the relationship is joined (represented by a list of objects) or not (represented by an id list).
 */
template<typename MODEL, typename LIST>
bool update_id_list(LIST& model_list, Data model_ids)
{
  return IdListDataUpdater<MODEL, LIST>::update_id_list(model_list, model_ids);
}

#endif

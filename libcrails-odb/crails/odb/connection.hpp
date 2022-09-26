#ifndef  CRAILS_ODB_CONNECTION_HPP
# define CRAILS_ODB_CONNECTION_HPP

# include <crails/utils/timer.hpp>
# include <crails/safe_ptr.hpp>
# include <odb/result.hxx>
# include <odb/database.hxx>
# include "query.hpp"
# include "transaction.hpp"
# include "exception.hpp"
# include "id_type.hpp"

namespace Crails
{
  namespace Odb
  {
    class Connection
    {
    public:
      Connection();
      virtual ~Connection();

      float time = 0.f;

      template<typename MODEL>
      void start_transaction_for()
      {
        transaction.require(MODEL().get_database_name());
      }

      template<typename MODEL>
      void start_transaction_for(const MODEL& model)
      {
        transaction.require(model.get_database_name());
      }

      void commit();
      void rollback();

      template<typename MODEL>
      unsigned long count(odb::query<MODEL> query = odb::query<MODEL>(true))
      {
        Utils::TimeGuard timer(time);

        start_transaction_for<MODEL>();
        return transaction.get_database()
          .query_value<typename MODEL::Count>(query).value;
      }

      template<typename MODEL_PTR>
      bool find_one(MODEL_PTR& model, odb::query<typename MODEL_PTR::element_type> query = odb::query<typename MODEL_PTR::element_type>(true))
      {
        typedef typename MODEL_PTR::element_type MODEL;
        Utils::TimeGuard timer(time);

        start_transaction_for<MODEL>();
        model = transaction.get_database().query_one<MODEL>(query);
        return model.get() != 0;
      }

      template<typename MODEL_PTR>
      bool find_one(MODEL_PTR& model, Odb::id_type id)
      {
        return find_one(model, odb::query<typename MODEL_PTR::element_type>::id == id);
      }

      template<typename MODEL>
      bool find(odb::result<MODEL>& results, odb::query<MODEL> query = odb::query<MODEL>(true))
      {
        Utils::TimeGuard timer(time);

        start_transaction_for<MODEL>();
        results = transaction.get_database().query<MODEL>(query);
        return !results.empty();
      }

      template<typename MODEL>
      void save(MODEL& model)
      {
        Utils::TimeGuard timer(time);
        start_transaction_for(model);
        model.before_save();
        model.save(transaction.get_database());
        model.after_save();
      }

      template<typename MODEL>
      void destroy(MODEL& model)
      {
        Utils::TimeGuard timer(time);

        model.before_destroy();
        start_transaction_for(model);
        try
        {
          model.destroy(transaction.get_database());
          model.after_destroy();
        }
        catch (const odb::object_not_persistent& e)
        {
          Odb::throw_exception(model, e.what());
        }
      }

      template<typename MODEL>
      void destroy(odb::query<MODEL> query = odb::query<MODEL>(true))
      {
        odb::result<MODEL> results;

        if (find(results, query))
        {
          for (auto& model : results)
            destroy(model);
        }
      }

      Transaction& transaction;
    };
  }
}

#endif

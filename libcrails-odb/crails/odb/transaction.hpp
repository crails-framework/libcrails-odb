#ifndef  CRAILS_ODB_TRANSACTION_HPP
# define CRAILS_ODB_TRANSACTION_HPP

# include <string>
# include <memory>

namespace odb
{
  class transaction;
  class session;
  class database;
}

namespace Crails
{
  namespace Odb
  {
    struct Transaction
    {
      static bool use_session;

      Transaction();
      ~Transaction();

      std::string get_database_name() const { return database_name; }
      odb::database& get_database();

      static Transaction& get();

      void require(const std::string& name);
      void start(const std::string& name, odb::database&);
      void commit();
      void rollback();

    private:
      void cleanup();

      std::string                       database_name;
      std::unique_ptr<odb::transaction> odb_transaction;
      std::unique_ptr<odb::session>     odb_session;
      odb::database*                    odb_database;
    };
  }
}

#endif

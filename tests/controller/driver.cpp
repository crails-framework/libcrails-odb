#include <crails/odb/connection.hpp>
#include <crails/odb/controller.hpp>
#include <crails/controller/coroutine.hpp>
#include <crails/databases.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include "../test_databases.hpp"
#include "test_support.hpp"
#include "test_record.hpp"
#include "test_record-odb.hpp"

#undef NODEBUG
#include <cassert>

using namespace std;
using namespace std::chrono_literals;

typedef Crails::Odb::Controller<Crails::CoroutineController, Crails::Odb::Connection> ControllerBase;

struct ThreadHopController : public ControllerBase
{
  using ControllerBase::database;

  ThreadHopController(Crails::Context& context) : ControllerBase(context)
  {}

  Crails::Odb::id_type first_id = 0, second_id = 0;
  bool ran_to_completion = false;
  bool saw_first_record_after_hop = false;
  bool saw_second_record_after_second_hop = false;
  bool should_commit = true;

  // default uses strand, so no thread hopping would happen
  boost::asio::any_io_executor get_io_executor() override
  {
    return Crails::CoroutineBoundExecutor{
      boost::asio::make_strand(Crails::Server::get_io_context()),
      Crails::CoroutineController::coroutine_executor
    };
  }

  void start()
  {
    co_spawn([this]() -> boost::asio::awaitable<void>
    {
      // 1. Real database work, opening a transaction.
      TestRecord first("before first hop");
      database.save(first);
      first_id = first.get_id();
      assert(database.transaction().active());

      // 2. Force a genuine suspension. Which thread resumes this coroutine
      // is up to the scheduler -- that unpredictability is exactly the bug
      // this test exists to catch, so we don't try to control it, only
      // drive a real multi-threaded pool from main() so it's *possible*.
      auto timer = boost::asio::steady_timer(co_await boost::asio::this_coro::executor, 5ms);
      co_await timer.async_wait(boost::asio::use_awaitable);

      // 3. If the transaction didn't survive the hop, this either throws
      // ("operation can only be performed in transaction") or, worse,
      // silently queries against whatever else happens to be current on
      // this thread instead. Neither should happen.
      shared_ptr<TestRecord> reloaded;
      saw_first_record_after_hop = database.find_one(reloaded, first_id)
        && reloaded->get_label() == "before first hop";

      // 4. A second hop, and a second write on the *same* transaction --
      // makes sure this isn't a one-suspension fluke.
      auto timer2 = boost::asio::steady_timer(co_await boost::asio::this_coro::executor, 5ms);
      co_await timer2.async_wait(boost::asio::use_awaitable);

      TestRecord second("after second hop");
      database.save(second);
      second_id = second.get_id();

      shared_ptr<TestRecord> reloaded2;
      saw_second_record_after_second_hop = database.find_one(reloaded2, second_id)
        && reloaded2->get_label() == "after second hop";

      if (should_commit)
        database.commit();

      ran_to_completion = true;
      close();
      co_return;
    });
  }
};

void driver()
{
  Crails::environment = Crails::Test;
  SingletonInstantiator<Crails::Server> server;
  SingletonInstantiator<Crails::SessionStore::Factory> session_factory;
  SingletonInstantiator<TestDatabases> databases;
  auto&           io = server->get_io_context();
  atomic<bool>    stop{false};
  vector<thread>  pool;

  create_test_schema();
  io.restart();
  for (int i = 0 ; i < 4 ; ++i)
    pool.emplace_back([&]() { while (!stop) io.run_for(10ms); });

  // BEGIN transaction survives real cross-thread suspension, commits correctly
  {
    Support::Fixture fixture;
    auto controller = make_shared<ThreadHopController>(*fixture.context);

    bool done = false;
    Crails::ActionRoute<ThreadHopController>::attach(*controller, [&]{ done = true; });
    controller->start();

    bool finished = Support::pump_until([&]{ return done; });

    assert(finished);
    assert(controller->ran_to_completion);
    assert(controller->saw_first_record_after_hop);
    assert(controller->saw_second_record_after_second_hop);
    assert(!controller->database.transaction().active());

    // Confirm it's actually durable, via a completely independent Connection.
    Crails::Odb::Connection independent;
    shared_ptr<TestRecord> found_first, found_second;
    assert(independent.find_one(found_first, controller->first_id));
    assert(independent.find_one(found_second, controller->second_id));
  }

  // BEGIN rollback also survives a cross-thread suspension
  //
  // Same coroutine, but it never commits -- the Connection's destructor
  // should roll back correctly regardless of which thread happens to be
  // running when the controller (and its embedded Connection) is destroyed.
  {
    Crails::Odb::id_type first_id, second_id;

    {
      Support::Fixture fixture;
      auto controller = make_shared<ThreadHopController>(*fixture.context);
      controller->should_commit = false;

      bool done = false;
      Crails::ActionRoute<ThreadHopController>::attach(*controller, [&]{ done = true; });
      controller->start();

      bool finished = Support::pump_until([&]{ return done; });

      assert(finished);
      assert(controller->ran_to_completion);
      assert(controller->saw_first_record_after_hop);
      assert(controller->saw_second_record_after_second_hop);

      first_id  = controller->first_id;
      second_id = controller->second_id;
      // should rollback here
    }

    Crails::Odb::Connection independent;
    shared_ptr<TestRecord> found_first, found_second;
    assert(!independent.find_one(found_first, first_id));
    assert(!independent.find_one(found_second, second_id));
  }

  stop = true;
  for (auto& t : pool)
    t.join();
}

int main()
{
  driver();
  Crails::Server::cleanup();
  return 0;
}

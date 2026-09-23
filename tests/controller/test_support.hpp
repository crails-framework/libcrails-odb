#pragma once
#include <crails/server.hpp>
#include <crails/context.hpp>
#include <crails/params.hpp>
#include <crails/http_response.hpp>
#include <crails/session_store.hpp>
#include <crails/controller/action.hpp>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace Crails
{
  template<typename CONTROLLER, bool WITH_ASYNC = true>
  class ActionRoute
  {
  public:
    static void attach(CONTROLLER& controller, std::function<void()> callback) { controller.ActionController::callback = callback; }
    static bool closed(const CONTROLLER& controller) { return controller.ActionController::closing; }
  };
}

namespace Support
{
  using namespace std::chrono_literals;

  struct TestServer : public Crails::Server
  {
    SINGLETON_IMPLEMENTATION(TestServer, Crails::Server)
  public:
    TestServer() {}
  };

  struct MemorySessionStore : public Crails::SessionStore
  {
    SESSION_STORE_IMPLEMENTATION(MemorySessionStore)
  public:
    static inline DataTree cookie;
    static void reset() { cookie = DataTree(); }
    void        load(const Crails::HttpRequest&) override {}
    void        finalize(Crails::BuildingResponse&) override {}
    Data        to_data() override { return cookie.as_data(); }
    const Data  to_data() const override { return cookie.as_data(); }
  };

  struct Setup
  {
    SingletonInstantiator<TestServer>                  server;
    SingletonInstantiator<MemorySessionStore::Factory> sessions;
    Setup() { MemorySessionStore::reset(); }
  };

  struct Fixture
  {
    std::shared_ptr<Crails::Connection> connection;
    std::shared_ptr<Crails::Context>    context;

    Fixture(Crails::HttpVerb verb = Crails::HttpVerb::get, std::map<std::string,std::string> headers = {}, std::string target = "/")
    {
      Crails::HttpRequest request;

      request.method(verb);
      request.target(target);
      for (const auto& [name, value] : headers)
        request.set(name, value);
      connection = std::make_shared<Crails::Connection>(Crails::Server::singleton::require(), request);
      context    = std::make_shared<Crails::Context>(Crails::Server::singleton::require(), *connection);
    }

    Crails::Context& operator*() { return *context; }
    Crails::Params&  params()    { return context->params; }
    unsigned         status() const { return context->response.get_raw_response().result_int(); }
    std::string      body() const   { return context->response.get_raw_response().body(); }
    bool             has_header(const std::string& name) const { return context->response.get_raw_response().find(name) != context->response.get_raw_response().end(); }
    std::string      header(const std::string& name) const
    {
      auto it = context->response.get_raw_response().find(name);
      return it == context->response.get_raw_response().end() ? std::string() : std::string(it->value());
    }
    bool             finished() { return context->get_future().wait_for(0s) == std::future_status::ready; }
  };

  inline bool pump_until(std::function<bool()> condition)
  {
    auto&      io       = Crails::Server::get_io_context();
    const auto deadline = std::chrono::steady_clock::now() + 5s;

    io.restart();
    while (!condition() && std::chrono::steady_clock::now() < deadline)
      io.run_one_for(2ms);
    return condition();
  }
}

#pragma once
#include <string>
#include <functional>
#include <vector>
#include <chrono>
#include <optional>

namespace Validation::Tests
{

  struct Test
  {
    Test(std::string const& desc, std::function<void()>&& callback)
    : _callback(std::move(callback))
    {

    }

    void SetMaxTime(std::uint64_t max_time) { _max_time.emplace(max_time); };

    void Execute() const;

    std::function<void()> _callback;
    std::optional<std::chrono::duration<uint64_t, std::micro>> _max_time;
  };

  struct ModuleTest
  {
    explicit ModuleTest(std::string const& name, std::string desc = "")
    : _name(name)
    , _desc(std::move(desc))
    {};

    class Test& Test(std::string const& desc, std::function<void()>&& callback)
    {
      return _tests.emplace_back(desc, std::move(callback));
    }

  private:
    std::string _name;
    std::string _desc;
    std::vector<struct Test> _tests;
  };
}
#include <Utils/Meta/Reflection.hpp>
#include <Validation/Log.hpp>

struct Test
{
  int first;
  float second;
  unsigned third;

  void fourth() { LogDebug("fourth() called. "); }

  template<typename T>
  T fifth(T x) { LogDebug("fifth() called."); return x; }


};

struct TestNonReflectable {};

REFLECTION_DESCRIPTOR(Test, first, second, third, fourth);


int main()
{
  static_assert(!Utils::Meta::Reflection::Concepts::Reflectable<TestNonReflectable>);
  static_assert(Utils::Meta::Reflection::Concepts::Reflectable<Test>);

  constexpr bool has = Utils::Meta::Reflection::Reflect<Test>::HasMember<"first">();

  if constexpr (!has)
    return 1;

  auto ptr = Utils::Meta::Reflection::Reflect<Test>::GetMemberPtr<"first">();

  Test t{};
  t.*ptr = 10;

  auto& mem = Utils::Meta::Reflection::Reflect<Test>::GetMember<"first">(t);
  mem = 100;

  Utils::Meta::Reflection::Reflect<Test>::InvokeMemberFunc<"fourth">(t);

  LogDebug("first: %d", t.first);
  return 0;
}
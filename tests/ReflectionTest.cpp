#include <Utils/Meta/Reflection.hpp>
#include <Validation/Log.hpp>

struct Test
{
  int first;
  float second;
  unsigned third;
};

struct TestNonReflectable {};

REFLECTION_DESCRIPTOR(Test, first, second, third);

using namespace Utils::Meta::Reflection;

int main()
{
  static_assert(!Concepts::Reflectable<TestNonReflectable>);
  static_assert(Concepts::Reflectable<Test>);

  constexpr bool has = Reflect<Test>::HasMember<"first1">();

  if constexpr (!has)
    return 1;

  auto ptr = Reflect<Test>::GetMemberPtr<"first">();

  Test t{};
  t.*ptr = 10;

  LogDebug("first: %d", t.first);
  return 0;
}
#include <Utils/Meta/Reflection.hpp>
#include <Validation/Log.hpp>

#include <array>

struct Test
{
  int first;
  float second;
  unsigned third;

  void fourth() { LogDebug("fourth() called. "); }

  template<typename T>
  T fifth(T x) { LogDebug("fifth() called."); return x; }
};

struct TestNested
{
  int own;

  Test test_field;
  Test c_array[3];
  std::array<Test, 2> array;
  float matrix [4][4];

  union TestUnion
  {
    std::uint32_t first;
    float second;
  } union_mem;
};

struct TestNonReflectable {};

REFLECTION_DESCRIPTOR(Test, first, second, third, fourth);
REFLECTION_DESCRIPTOR(TestNested, own, test_field, c_array, array, matrix, union_mem);

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
  static_assert(Utils::Meta::Reflection::Reflect<Test>::IsMemberFunc<"fourth">());
  static_assert(!Utils::Meta::Reflection::Reflect<Test>::IsMemberVar<"fourth">());

  LogDebug("struct Test");
  LogDebug("{");
  {
    LogIndentScoped;
    Utils::Meta::Reflection::Reflect<Test>::ForEachMember([=](auto ptr, std::string_view name)
      {
        if constexpr (std::is_member_object_pointer_v<decltype(ptr)>)
        {
          LogDebug("%s %s = %s;", NAMEOF_SHORT_TYPE(Utils::Meta::Traits::TypeOfMemberObject_T<decltype(ptr)>), name, t.*ptr);
        }
      });
  }
  LogDebug("};");
  LogDebug("first: %d", t.first);


  TestNested t1{};
  Utils::Meta::Reflection::Reflect<TestNested>::PrettyPrint([](auto str, auto...args) { LogDebug(str, args...); }, t1);

  return 0;
}
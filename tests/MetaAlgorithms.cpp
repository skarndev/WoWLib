#include <Validation/Log.hpp>
#include <Utils/Meta/Algorithms.hpp>
#include <Validation/Contracts.hpp>

using namespace Utils::Meta::DataTypes;

struct Test
{
  template<typename...Ts>
  void test()
  {
    INLINE_FOR_EACH_TYPE(TypePack<Ts...>, T, i)
    {
      std::cout << typeid(T).name() << std::endl;
    };
  }

  template<auto... vars>
  void test_nttp()
  {
    INLINE_FOR_EACH_NTTP(ConstPack<vars...>, var, i)
    {
      std::cout << var << std::endl;
    };
  }

  template<auto... vars>
  void test_nttp_if()
  {
    LogDebug("Testing: nttp_if");
    constexpr auto pred = [](auto var, std::size_t i){ return std::is_same_v<decltype(var), int> && var < 10; };

    INLINE_FOR_EACH_NTTP_IF(ConstPack<vars...>, var, i, pred)
    {
      std::cout << var << std::endl;
    };
  }

  template<typename...Ts>
  void test_typepack_if()
  {
    LogDebug("Testing: typepack_if");
    constexpr auto pred = []<typename T>(std::type_identity<T> t, std::size_t i) -> bool { return std::is_integral_v<T>; };

    INLINE_FOR_EACH_TYPE_IF(TypePack<Ts...>, T, i, pred)
    {
      std::cout << typeid(T).name() << std::endl;
    };
  }
};

int main()
{

  int a = 100;
  int b = 0;
  INLINE_FOR(i, 0, 10)
  {
    std::cout << i + a << std::endl;
    b++;
  };
  Ensure(b == 10, "Expected to be 10.");

  Test t{};
  t.test<int, float, unsigned>();
  t.test_nttp<false, 1>();
  t.test_nttp_if<1, 2, 100, 'c'>();
  t.test_typepack_if<int, unsigned, float>();

  constexpr bool any_of_test = Utils::Meta::Algorithms::AnyOfNTTPs
    <ConstPack<'c', 1>, [](auto var, std::size_t i) { return std::is_same_v<char, decltype(var)>; }
    >();

  static_assert(any_of_test);

  constexpr bool any_of_test_types = Utils::Meta::Algorithms::AnyOfTypes
    <TypePack<int, float, unsigned>, []<typename T>(std::type_identity<T>, std::size_t i){ return std::is_same_v<int, T>; }
    >();

  static_assert(any_of_test_types);

  enum class T { A };
  constexpr bool all_of_test = Utils::Meta::Algorithms::AllOfNTTPs
    <ConstPack<'c', 'b', 'a'>, [](auto var, std::size_t i) { return std::is_same_v<char, decltype(var)>; }
    >();

  static_assert(all_of_test);

  constexpr bool all_of_test_types = Utils::Meta::Algorithms::AllOfTypes
    <TypePack<char, char, char>, []<typename T>(std::type_identity<T>, std::size_t i) { return std::is_same_v<char, T>; }
    >();

  static_assert(all_of_test_types);

  constexpr bool none_of_test = Utils::Meta::Algorithms::NoneOfNTTPs
    <ConstPack<1, 2, 3>, [](auto var, std::size_t i) { return std::is_same_v<char, decltype(var)>; }
    >();

  static_assert(none_of_test);

  constexpr bool none_of_test_types = Utils::Meta::Algorithms::NoneOfTypes
    <TypePack<int, int, int>,
      []<typename T>(std::type_identity<T>, std::size_t i) { return std::is_same_v<char, T>; }
    >();

  static_assert(none_of_test_types);

  constexpr auto test_find_nttp = Utils::Meta::Algorithms::FindNTTP
    <
    ConstPack<1, 2, 'c', 3>,
    [](auto var, std::size_t i) { return std::is_same_v<char, decltype(var)>; }
    >;

  static_assert(!std::is_same_v<decltype(decltype(test_find_nttp)::value), Utils::Meta::DataTypes::NoType const>);
  static_assert(decltype(test_find_nttp)::index == 2);

  constexpr auto test_find_type = Utils::Meta::Algorithms::FindType
    <
      TypePack<int, float, unsigned>,
      []<typename T>(std::type_identity<T>, std::size_t i) { return std::is_same_v<T, float>; }
    >;

  static_assert(decltype(test_find_type)::index == 1);

  return 0;

}
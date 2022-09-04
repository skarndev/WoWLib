#include <Validation/Log.hpp>
#include <Utils/Meta/Algorithms.hpp>
#include <Validation/Contracts.hpp>


struct Test
{
  template<typename...Ts>
  void test()
  {
    INLINE_FOR_EACH_TYPE(Ts, T, i)
    {
      std::cout << typeid(T).name() << std::endl;
    };
  }

  template<auto... vars>
  void test_nttp()
  {
    INLINE_FOR_EACH_NTTP(vars, var, i)
    {
      std::cout << var << std::endl;
    };
  }

  template<auto... vars>
  void test_nttp_if()
  {
    LogDebug("Testing: nttp_if");
    constexpr auto pred = [](auto var, std::size_t i){ return std::is_same_v<decltype(var), int> && var < 10; };

    INLINE_FOR_EACH_NTTP_IF(vars, var, i, pred)
    {
      std::cout << var << std::endl;
    };
  }

  template<typename...Ts>
  void test_typepack_if()
  {
    LogDebug("Testing: typepack_if");
    constexpr auto pred = []<typename T>(std::type_identity<T> t, std::size_t i) -> bool { return std::is_integral_v<T>; };

    INLINE_FOR_EACH_TYPE_IF(Ts, T, i, pred)
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

  constexpr bool any_of_test = Utils::Meta::Algorithms::AnyOf
                              <[](auto var, std::size_t i){ return std::is_same_v<char, decltype(var)>; }
                                , 'c', 1
                              >();

  static_assert(any_of_test);
  return 0;
}
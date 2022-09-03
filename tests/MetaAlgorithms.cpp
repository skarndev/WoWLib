#include <Validation/Log.hpp>
#include <Utils/Meta/Algorithms.hpp>

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
};

int main(){

  int a = 100;
  INLINE_FOR(i, 0, 10){
    std::cout << i + a << std::endl;
  };

  Test t{};
  t.test<int, float, unsigned>();
  t.test_nttp<false, 1>();
  t.test_nttp_if<1, 2, 100, 'c'>();

  return 0;
}
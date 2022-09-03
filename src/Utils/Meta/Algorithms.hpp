#pragma once
#include <Utils/Misc/ForceInline.hpp>

#include <type_traits>
#include <cstdint>


/**
 * Set of compilietime algorithms intended to reduce or eliminate the need for recursive template instantiations and
 * partial specializations.
 */
namespace Utils::Meta::Algorithms
{
  template<auto start, auto end>
  struct InlineFor
  {
    void operator|(auto f)
    {
      using T = decltype(start);
      [&]<T... Is>(std::integer_sequence<T, Is...>)
      {
        (f.template operator()<Is+start>(),...);
      }(std::make_integer_sequence<T, end-start>{});
    }
  };

  template<typename...Ts>
  struct InlineForEachType
  {
    void operator|(auto f)
    {
      [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>)
      {
        (f.template operator()<Ts, Is>(),...);
      }(std::make_integer_sequence<std::size_t, sizeof...(Ts)>{});
    }
  };

  template<auto... vars>
  struct InlineForEachNTTP
  {
    void operator|(auto f)
    {
      [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>)
      {
        (f.template operator()<vars, Is>(),...);
      }(std::make_integer_sequence<std::size_t, sizeof...(vars)>{});
    }
  };

  template<auto predicate, auto... vars>
  requires (std::is_invocable_r_v<bool, decltype(predicate), decltype(vars), std::size_t> && ...)
  struct InlineForEachNTTP_If
  {
    void operator|(auto f)
    {
      [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>)
      {
        auto conditional_invoke = [&]<auto var, std::size_t i>()
        {
          if (std::invoke(predicate, var, i))
          {
            f.template operator()<var, i>();
          }
        };

        (conditional_invoke.template operator()<vars, Is>() , ...);
      }(std::make_integer_sequence<std::size_t, sizeof...(vars)>{});
    }

  };

}

#define INLINE_FOR(var, beg, end) Utils::Meta::Algorithms::InlineFor<beg,end>{}|[=]<auto var>

#define INLINE_FOR_EACH_TYPE(TYPEPACK, CUR_TYPE, COUNTER) \
  Utils::Meta::Algorithms::InlineForEachType<TYPEPACK...>{} | [=]<typename CUR_TYPE, std::size_t COUNTER>

#define INLINE_FOR_EACH_NTTP(NTTP_PACK, CUR_VAR, COUNTER) \
  Utils::Meta::Algorithms::InlineForEachNTTP<NTTP_PACK...>{} | [=]<auto CUR_VAR, std::size_t COUNTER>

#define INLINE_FOR_EACH_NTTP_IF(NTTP_PACK, CUR_VAR, COUNTER, PREDICATE) \
  Utils::Meta::Algorithms::InlineForEachNTTP_If<PREDICATE, NTTP_PACK...>{} | [=]<auto CUR_VAR, std::size_t COUNTER>
#pragma once
#include <Utils/Misc/ForceInline.hpp>

#include <type_traits>
#include <cstdint>
#include <concepts>

/**
 * Set of compilietime algorithms intended to reduce or eliminate the need for recursive template instantiations and
 * partial specializations.
 */
namespace Utils::Meta::Algorithms
{
  namespace Concepts
  {
    /**
     * Checks if provided type is a valid NTTP predicate for a given NTTP.
     * @tparam Callable Any type (callable).
     * @tparam nttp Any NTTP.
     */
    template<typename Callable, auto nttp>
    concept NTTP_Predicate = std::is_invocable_r_v<bool, Callable, decltype(nttp), std::size_t>;

    /**
     * Checks if provided type is a valid type predicate for a given type.
     * @tparam Callable Any type (callable).
     * @tparam T Any type.
     */
    template<typename Callable, typename T>
    concept Type_Predicate = std::is_invocable_r_v<bool, Callable, std::type_identity<T>, std::size_t>;
  }

  template<auto start, auto end>
  struct InlineFor
  {
    FORCEINLINE_ATTR constexpr void operator|(auto f)
    {
      using T = decltype(start);
      [&]<T... Is>(std::integer_sequence<T, Is...>) FORCEINLINE_ATTR
      {
        (f.template operator()<Is+start>(),...);
      }(std::make_integer_sequence<T, end-start>{});
    }
  };

  template<typename...Ts>
  struct InlineForEachType
  {
    FORCEINLINE_ATTR constexpr void operator|(auto f)
    {
      [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) FORCEINLINE_ATTR
      {
        (f.template operator()<Ts, Is>(),...);
      }(std::make_integer_sequence<std::size_t, sizeof...(Ts)>{});
    }
  };

  template<auto predicate, typename... Ts>
  requires (Concepts::Type_Predicate<decltype(predicate), Ts> && ...)
  struct InlineForEachType_If
  {
    FORCEINLINE_ATTR constexpr void operator|(auto f)
    {
      [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) FORCEINLINE_ATTR
      {
        auto conditional_invoke = [&]<typename T, std::size_t i>() FORCEINLINE_ATTR
        {
          if (std::invoke(predicate, std::type_identity<T>{}, i))
          {
            f.template operator()<T, i>();
          }
        };

        (conditional_invoke.template operator()<Ts, Is>(), ...);
      }(std::make_integer_sequence<std::size_t, sizeof...(Ts)>{});
    }
  };

  template<auto... vars>
  struct InlineForEachNTTP
  {
    FORCEINLINE_ATTR constexpr void operator|(auto f)
    {
      [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) FORCEINLINE_ATTR
      {
        (f.template operator()<vars, Is>(),...);
      }(std::make_integer_sequence<std::size_t, sizeof...(vars)>{});
    }
  };

  template<auto predicate, auto... vars>
  requires (Concepts::NTTP_Predicate<decltype(predicate), vars> && ...)
  struct InlineForEachNTTP_If
  {
    FORCEINLINE_ATTR constexpr void operator|(auto f)
    {
      [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) FORCEINLINE_ATTR
      {
        auto conditional_invoke = [&]<auto var, std::size_t i>() FORCEINLINE_ATTR
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

  namespace details
  {
    template<auto predicate, std::size_t index, auto var, auto...rem_vars>
    FORCEINLINE_ATTR constexpr bool AnyOfImpl()
    {
      if constexpr(std::invoke(predicate, var, index))
      {
        return true;
      }
      else
      {
        if constexpr (sizeof...(rem_vars) > 0)
        {
          if constexpr (AnyOfImpl<predicate, index + 1, rem_vars...>())
            return true;
        }
        else
        {
          return false;
        }
      }

      return false;
    }

  }

  template<auto predicate, auto...vars>
  requires (Concepts::NTTP_Predicate<decltype(predicate), vars> && ...)
  [[nodiscard]]
  FORCEINLINE_ATTR constexpr bool AnyOf()
  {
    return details::AnyOfImpl<predicate, 0, vars...>();
  }
}

#define INLINE_FOR(var, beg, end) Utils::Meta::Algorithms::InlineFor<beg,end>{} \
  |[&]<auto var>() FORCEINLINE_ATTR

#define INLINE_FOR_EACH_TYPE(TYPEPACK, CUR_TYPE, COUNTER) \
  Utils::Meta::Algorithms::InlineForEachType<TYPEPACK...>{} \
  | [&]<typename CUR_TYPE, std::size_t COUNTER>() FORCEINLINE_ATTR

#define INLINE_FOR_EACH_NTTP(NTTP_PACK, CUR_VAR, COUNTER) \
  Utils::Meta::Algorithms::InlineForEachNTTP<NTTP_PACK...>{} \
  | [&]<auto CUR_VAR, std::size_t COUNTER>() FORCEINLINE_ATTR

#define INLINE_FOR_EACH_NTTP_IF(NTTP_PACK, CUR_VAR, COUNTER, PREDICATE) \
  Utils::Meta::Algorithms::InlineForEachNTTP_If<PREDICATE, NTTP_PACK...>{} \
  | [&]<auto CUR_VAR, std::size_t COUNTER>() FORCEINLINE_ATTR

#define INLINE_FOR_EACH_TYPE_IF(TYPEPACK, CUR_TYPE, COUNTER, PREDICATE) \
  Utils::Meta::Algorithms::InlineForEachType_If<PREDICATE, TYPEPACK...>{} \
  | [&]<typename CUR_TYPE, std::size_t COUNTER>() FORCEINLINE_ATTR
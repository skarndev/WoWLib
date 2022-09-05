#pragma once
#include <Utils/Misc/ForceInline.hpp>
#include <Utils/Meta/DataTypes.hpp>

#include <type_traits>
#include <cstdint>
#include <concepts>
#include <limits>

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
    concept NTTP_Predicate = std::is_invocable_r_v<bool, Callable
      , std::integral_constant<decltype(nttp), nttp>
      , std::integral_constant<std::size_t, 0>>;

    /**
     * Checks if provided type is a valid type predicate for a given type.
     * @tparam Callable Any type (callable).
     * @tparam T Any type.
     */
    template<typename Callable, typename T>
    concept Type_Predicate = std::is_invocable_r_v<bool, Callable, std::type_identity<T>
      , std::integral_constant<std::size_t, 0>>;
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

  template<Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack> T>
  struct InlineForEachType;

  template<typename...Ts>
  struct InlineForEachType<Utils::Meta::DataTypes::TypePack<Ts...>>
  {
    FORCEINLINE_ATTR constexpr void operator|(auto f)
    {
      [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) FORCEINLINE_ATTR
      {
        (f.template operator()<Ts, Is>(),...);
      }(std::make_integer_sequence<std::size_t, sizeof...(Ts)>{});
    }
  };

  template<Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack> T, auto predicate>
  struct InlineForEachType_If;

  template<auto predicate, typename... Ts>
  requires (Concepts::Type_Predicate<decltype(predicate), Ts> && ...)
  struct InlineForEachType_If<Utils::Meta::DataTypes::TypePack<Ts...>, predicate>
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

  template<Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack> T>
  struct InlineForEachNTTP;

  template<auto... vars>
  struct InlineForEachNTTP<Utils::Meta::DataTypes::ConstPack<vars...>>
  {
    FORCEINLINE_ATTR constexpr void operator|(auto f)
    {
      [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) FORCEINLINE_ATTR
      {
        (f.template operator()<vars, Is>(),...);
      }(std::make_integer_sequence<std::size_t, sizeof...(vars)>{});
    }
  };

  template<Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack> T, auto predicate>
  struct InlineForEachNTTP_If;

  template<auto predicate, auto... vars>
  requires (Concepts::NTTP_Predicate<decltype(predicate), vars> && ...)
  struct InlineForEachNTTP_If<Utils::Meta::DataTypes::ConstPack<vars...>, predicate>
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

  // AnyOf

  namespace details
  {
    template
    <
      auto predicate
      , std::size_t index
      , Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack> T
    >
    struct AnyOfNTTPsImpl;

    template<auto predicate, std::size_t index, auto var, auto...rem_vars>
    requires (Concepts::NTTP_Predicate<decltype(predicate), var>
              && (Concepts::NTTP_Predicate<decltype(predicate), rem_vars> && ...))
    struct AnyOfNTTPsImpl<predicate, index, Utils::Meta::DataTypes::ConstPack<var, rem_vars...>>
    {
      FORCEINLINE_ATTR static constexpr bool Impl()
      {
        if constexpr (std::invoke(predicate, var, index))
          return true;
        else if constexpr (sizeof...(rem_vars) > 0)
          return AnyOfNTTPsImpl<predicate, index + 1, Utils::Meta::DataTypes::ConstPack<rem_vars...>>::Impl();
        else
          return false;
      }
    };

  }

  template<Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack> T, auto predicate>
  [[nodiscard]]
  FORCEINLINE_ATTR constexpr bool AnyOfNTTPs()
  {
    return details::AnyOfNTTPsImpl<predicate, 0, T>::Impl();
  }

  namespace details
  {
    template<auto predicate, std::size_t index, Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack> T>
    struct AnyOfTypesImpl;

    template<auto predicate, std::size_t index, typename T, typename... RemTs>
    requires (Utils::Meta::Algorithms::Concepts::Type_Predicate<decltype(predicate), T>
      && (Utils::Meta::Algorithms::Concepts::Type_Predicate<decltype(predicate), RemTs> && ...))
    struct AnyOfTypesImpl<predicate, index, Utils::Meta::DataTypes::TypePack<T, RemTs...>>
    {
      FORCEINLINE_ATTR static constexpr bool Impl()
      {
        if constexpr (std::invoke(predicate, std::type_identity<T>{}, index))
          return true;
        else if constexpr (sizeof...(RemTs) > 0)
          return AnyOfTypesImpl<predicate, index + 1, Utils::Meta::DataTypes::TypePack<RemTs...>>::Impl();
        else
          return false;
      }
    };
  }

  template<Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack> Pack, auto predicate>
  [[nodiscard]]
  FORCEINLINE_ATTR constexpr bool AnyOfTypes()
  {
    return details::AnyOfTypesImpl<predicate, 0, Pack>::Impl();
  }

  // AllOf

  namespace details
  {
    template<auto predicate, std::size_t index, Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack> T>
    struct AllOfTypesImpl;

    template<auto predicate, std::size_t index, typename T, typename... RemTs>
    requires (Utils::Meta::Algorithms::Concepts::Type_Predicate<decltype(predicate), T>
              && (Utils::Meta::Algorithms::Concepts::Type_Predicate<decltype(predicate), RemTs> && ...))
    struct AllOfTypesImpl<predicate, index, Utils::Meta::DataTypes::TypePack<T, RemTs...>>
    {
      FORCEINLINE_ATTR static constexpr bool Impl()
      {
        if constexpr (!std::invoke(predicate, std::type_identity<T>{}, index))
          return false;
        else if constexpr (sizeof...(RemTs) > 0)
          return AllOfTypesImpl<predicate, index + 1, Utils::Meta::DataTypes::TypePack<RemTs...>>::Impl();
        else
          return true;
      }
    };
  }

  template<Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack> Pack, auto predicate>
  [[nodiscard]]
  FORCEINLINE_ATTR constexpr bool AllOfTypes()
  {
    return details::AllOfTypesImpl<predicate, 0, Pack>::Impl();
  }

  namespace details
  {
    template
    <
      auto predicate
      , std::size_t index
      , Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack> T
    >
    struct AllOfNTTPsImpl;

    template<auto predicate, std::size_t index, auto var, auto...rem_vars>
    requires (Concepts::NTTP_Predicate<decltype(predicate), var>
              && (Concepts::NTTP_Predicate<decltype(predicate), rem_vars> && ...))
    struct AllOfNTTPsImpl<predicate, index, Utils::Meta::DataTypes::ConstPack<var, rem_vars...>>
    {
      FORCEINLINE_ATTR static constexpr bool Impl()
      {
        if constexpr (!std::invoke(predicate, var, index))
          return false;
        else if constexpr (sizeof...(rem_vars) > 0)
          return AllOfNTTPsImpl<predicate, index + 1, Utils::Meta::DataTypes::ConstPack<rem_vars...>>::Impl();
        else
          return true;
      }
    };

  }

  template<Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack> T, auto predicate>
  [[nodiscard]]
  FORCEINLINE_ATTR constexpr bool AllOfNTTPs()
  {
    return details::AllOfNTTPsImpl<predicate, 0, T>::Impl();
  }

  // NoneOf

  namespace details
  {
    template<auto predicate, std::size_t index, Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack> T>
    struct NoneOfTypesImpl;

    template<auto predicate, std::size_t index, typename T, typename... RemTs>
    requires (Utils::Meta::Algorithms::Concepts::Type_Predicate<decltype(predicate), T>
              && (Utils::Meta::Algorithms::Concepts::Type_Predicate<decltype(predicate), RemTs> && ...))
    struct NoneOfTypesImpl<predicate, index, Utils::Meta::DataTypes::TypePack<T, RemTs...>>
    {
      FORCEINLINE_ATTR static constexpr bool Impl()
      {
        if constexpr (std::invoke(predicate, std::type_identity<T>{}, index))
          return false;
        else if constexpr (sizeof...(RemTs) > 0)
          return NoneOfTypesImpl<predicate, index + 1, Utils::Meta::DataTypes::TypePack<RemTs...>>::Impl();
        else
          return true;
      }
    };
  }

  template<Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack> Pack, auto predicate>
  [[nodiscard]]
  FORCEINLINE_ATTR constexpr bool NoneOfTypes()
  {
    return details::NoneOfTypesImpl<predicate, 0, Pack>::Impl();
  }

  namespace details
  {
    template
    <
      auto predicate
      , std::size_t index
      , Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack> T
    >
    struct NoneOfNTTPsImpl;

    template<auto predicate, std::size_t index, auto var, auto...rem_vars>
    requires (Concepts::NTTP_Predicate<decltype(predicate), var>
              && (Concepts::NTTP_Predicate<decltype(predicate), rem_vars> && ...))
    struct NoneOfNTTPsImpl<predicate, index, Utils::Meta::DataTypes::ConstPack<var, rem_vars...>>
    {
      FORCEINLINE_ATTR static constexpr bool Impl()
      {
        if constexpr (std::invoke(predicate, var, index))
          return false;
        else if constexpr (sizeof...(rem_vars) > 0)
          return NoneOfNTTPsImpl<predicate, index + 1, Utils::Meta::DataTypes::ConstPack<rem_vars...>>::Impl();
        else
          return true;
      }
    };
  }

  template<Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack> T, auto predicate>
  [[nodiscard]]
  FORCEINLINE_ATTR constexpr bool NoneOfNTTPs()
  {
    return details::NoneOfNTTPsImpl<predicate, 0, T>::Impl();
  }

  // Find
  namespace details
  {
    template<auto predicate, std::size_t i, Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack>>
    struct FindNTTPImpl;

    template<auto predicate, std::size_t i, auto var, auto... rem_vars>
    requires (Concepts::NTTP_Predicate<decltype(predicate), var>
              && (Concepts::NTTP_Predicate<decltype(predicate), rem_vars> && ...))
    struct FindNTTPImpl<predicate, i, Utils::Meta::DataTypes::ConstPack<var, rem_vars...>>
    {
      static constexpr auto Impl()
      {
        if constexpr (std::invoke(predicate, std::integral_constant<decltype(var), var>{}
                                  , std::integral_constant<std::size_t, i>{}))
          return Utils::Meta::DataTypes::NTTPIndex<var, i>{};
        else if constexpr (sizeof...(rem_vars) > 0)
            return FindNTTPImpl<predicate, i + 1, Utils::Meta::DataTypes::ConstPack<rem_vars...>>::Impl();
        else
          return Utils::Meta::DataTypes::NTTPIndex<Utils::Meta::DataTypes::NoType{}
            , std::numeric_limits<std::size_t>::max()>{};
      }
    };
  }

  template<Utils::Meta::Concepts::InstanceOf_NTTP<Utils::Meta::DataTypes::ConstPack> Pack, auto predicate>
  constexpr auto FindNTTP = details::FindNTTPImpl<predicate, 0, Pack>::Impl();

  namespace details
  {
    template<auto predicate, std::size_t i, Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack>>
    struct FindTypeImpl;

    template<auto predicate, std::size_t i, typename T, typename... Ts>
    requires (Concepts::Type_Predicate<decltype(predicate), T>
              && (Concepts::Type_Predicate<decltype(predicate), Ts> && ...))
    struct FindTypeImpl<predicate, i, Utils::Meta::DataTypes::TypePack<T, Ts...>>
    {
      static constexpr auto Impl()
      {
        if constexpr (std::invoke(predicate, std::type_identity<T>{}, i))
          return Utils::Meta::DataTypes::TypeIndex<T, i>{};
        else if constexpr (sizeof...(Ts) > 0)
            return FindTypeImpl<predicate, i + 1, Utils::Meta::DataTypes::TypePack<Ts...>>::Impl();
        else
          return Utils::Meta::DataTypes::TypeIndex<Utils::Meta::DataTypes::NoType
            , std::numeric_limits<std::size_t>::max()>{};
      }
    };
  }

  template<Utils::Meta::Concepts::InstanceOf<Utils::Meta::DataTypes::TypePack> Pack, auto predicate>
  constexpr auto FindType = details::FindTypeImpl<predicate, 0, Pack>::Impl();

}

#define INLINE_FOR(var, beg, end) Utils::Meta::Algorithms::InlineFor<beg,end>{} \
  |[&]<auto var>() FORCEINLINE_ATTR

#define INLINE_FOR_EACH_TYPE(TYPEPACK, CUR_TYPE, COUNTER) \
  Utils::Meta::Algorithms::InlineForEachType<TYPEPACK>{} \
  | [&]<typename CUR_TYPE, std::size_t COUNTER>() FORCEINLINE_ATTR

#define INLINE_FOR_EACH_NTTP(NTTP_PACK, CUR_VAR, COUNTER) \
  Utils::Meta::Algorithms::InlineForEachNTTP<NTTP_PACK>{} \
  | [&]<auto CUR_VAR, std::size_t COUNTER>() FORCEINLINE_ATTR

#define INLINE_FOR_EACH_NTTP_IF(NTTP_PACK, CUR_VAR, COUNTER, PREDICATE) \
  Utils::Meta::Algorithms::InlineForEachNTTP_If<NTTP_PACK, PREDICATE>{} \
  | [&]<auto CUR_VAR, std::size_t COUNTER>() FORCEINLINE_ATTR

#define INLINE_FOR_EACH_TYPE_IF(TYPEPACK, CUR_TYPE, COUNTER, PREDICATE) \
  Utils::Meta::Algorithms::InlineForEachType_If<TYPEPACK, PREDICATE>{} \
  | [&]<typename CUR_TYPE, std::size_t COUNTER>() FORCEINLINE_ATTR

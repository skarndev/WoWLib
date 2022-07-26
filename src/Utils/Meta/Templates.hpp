#pragma once
#include <array>
#include <type_traits>
#include <algorithm>

namespace Utils::Meta::Templates
{
  namespace details
  {
    /**
     * Used for generating a unique empty type paramterized for T. Service struct for OptionalBase and alike.
     * @tparam T Any type.
     */
    template<typename T>
    struct Empty{};
  }

  /**
   * Utility to provide an optional base for types.
   * @tparam T Type to use as a base for another type.
   * @tparam use constexpr boolean or expression convertible to boolean to determine whether to use this base or not.
   */
  template<typename T, bool use>
  using OptionalBase = typename std::conditional_t<use, T, details::Empty<T>>;

  /**
   * Make std::array given a homogenous list of literals.
   * @tparam Type std::array underlying type.
   * @tparam T Literals.
   * @param t Homogenous list of literals.
   * @return std::array<Type, sizeof(T)>
   */
  template<typename Type, typename ... T>
  constexpr auto MakeArray(T&&... t) -> std::array<Type, sizeof...(T)>
  {
    return { {std::forward<T>(t)...} };
  }

  /**
   * Utility structure to allow passing string literals as non-type template parameters.
   * @tparam n Number of characters in string (normally deduced).
   */
  template<std::size_t n>
  struct StringLiteral
  {
    constexpr StringLiteral(const char(&str)[n])
    {
      std::copy_n(str, n - 1, value);
    }

    char value[n - 1];
  };


  /**
   * Compute if two values are equal to each other and pass the provided type.
   * To be used with std::disjunction / std::conjunction and alike.
   * @tparam v1 First value.
   * @tparam v2 Second value.
   * @tparam T Type to store.
   * @tparam do_compute Enables computation of expression. If false, expression is always false.
   */
  template<auto v1, decltype(v1) v2, typename T, bool do_compute = true>
  struct ValuesEqualT : std::bool_constant<do_compute && v1 == v2>
  {
    using type = T;
  };

  /**
   * DefaultType<T>::value is always true.
   * To be used with std::disjunction / std::conjunction and alike.
   * @tparam T Type to store.
   */
  template <typename T>
  struct DefaultType : std::true_type
  {
    using type = T;
  };


}
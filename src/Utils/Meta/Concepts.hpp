#pragma once
#include <type_traits>
#include <concepts>

namespace Utils::Meta::Concepts
{
  namespace details
  {
    template<typename T>
    concept ImplicitLifetimeTypeSingular = std::is_scalar_v<T> || std::is_class_v<T>
      && std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>;
  }

  /**
   * Checks if type T satisfies the criteria of an implicit lifetime type.
   * @tparam T Any type.
   */
  template<typename T>
  concept ImplicitLifetimeType = details::ImplicitLifetimeTypeSingular<T>
    || std::is_array_v<T> && details::ImplicitLifetimeTypeSingular<std::remove_all_extents_t<T>>;

  /**
   * Checks if type T satisfies the criteria of POD (plain old data).
   * @tparam T Any type.
   */
  template<typename T>
  concept PODType = std::is_standard_layout_v<T> && std::is_trivial_v<T>;

  /**
   * Checks if type T is one of the provided pack of types.
   * @tparam T Candidate type.
   * @tparam Ts Pack of types.
   */
  template<typename T, typename... Ts>
  concept AnyOf = (std::same_as<T, Ts> || ...);
}

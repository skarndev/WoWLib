#pragma once
#include <Utils/Meta/Traits.hpp>

#include <type_traits>
#include <concepts>

namespace Utils::Meta::Concepts
{
  namespace details
  {
    template<typename T>
    concept ImplicitLifetimeTypeSingular = (std::is_scalar_v<T> || std::is_class_v<T>)
      && std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>;
  }

  /**
   * Checks if type T satisfies the criteria of an implicit lifetime type.
   * @tparam T Any type.
   */
  template<typename T>
  concept ImplicitLifetimeType = details::ImplicitLifetimeTypeSingular<T>
    || (std::is_array_v<T> && details::ImplicitLifetimeTypeSingular<std::remove_all_extents_t<T>>);

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

  /**
   * Checks if type T is a member pointer.
   * @tparam T Any type.
   */
  template<typename T>
  concept MemberPointer = std::is_member_pointer_v<T>;

  /**
   * Checks if type T is a member pointer of Class.
   * @tparam Any type.
   * @tparam Any class.
   */
   template<typename T, typename Class>
   concept IsMemberOf = MemberPointer<T> && std::is_same_v<Traits::ClassOfMember_T<T>, Class>;

   /**
   * Checks if type is an instance of provided template (type only).
   * @tparam T Any type.
   * @tparam Template Any template accepting only type parameters.
   */
   template<typename T, template<typename...> typename Template>
   concept InstanceOf = Utils::Meta::Traits::IsInstanceOf_V<T, Template>;

  /**
   * Checks if type is an instance of provided template (NTTP only).
   * @tparam T Any type.
   * @tparam Template Any template accepting only non-type parameters.
   */
   template<typename T, template<auto...> typename Template>
   concept InstanceOf_NTTP = Utils::Meta::Traits::IsInstanceOf_NTTP_V<T, Template>;

}

#pragma once
#include <type_traits>
#include <algorithm>

namespace Utils::Meta::Traits
{
  /**
   * Extracts class from a member function pointer. Use ::type to access class.
   * @tparam T Member function pointer
   */
  template<typename T>
  requires (std::is_member_function_pointer_v<T>)
  struct ClassOfMemberFunction {};

  template<typename Return, typename Class>
  struct ClassOfMemberFunction<Return (Class::*)>
  {
    using type = Class;
  };

  /**
   * Extracts class from a member function pointer..
   * @tparam T Member object pointer
   */
  template<typename T>
  requires (std::is_member_function_pointer_v<T>)
  using ClassOfMemberFunction_T = typename ClassOfMemberFunction<T>::type;

  /**
   * Extracts class from a member object pointer. Use ::type to access class.
   * @tparam T Member object pointer
   */
  template<typename T>
  requires (std::is_member_object_pointer_v<T>)
  struct ClassOfMemberObject;

  template<typename Class, typename Value>
  struct ClassOfMemberObject<Value Class::*>
  {
    using type = Class;
  };

  /**
   * Extracts class from a member object pointer.
   * @tparam T Member object pointer
   */
  template<typename T>
  requires (std::is_member_object_pointer_v<T>)
  using ClassOfMemberObject_T = typename ClassOfMemberObject<T>::type;

  /**
   * Extracts type of member object pointer. Use ::type to access type.
   * @tparam T Member object pointer
   */
  template<typename T>
  requires (std::is_member_object_pointer_v<T>)
  struct TypeOfMemberObject;

  template<typename Class, typename Value>
  struct TypeOfMemberObject<Value Class::*>
  {
    using type = Value;
  };

  /**
   * Extracts type of member object pointer.
   * @tparam T Member object pointer
   */
  template<typename T>
  requires (std::is_member_object_pointer_v<T>)
  using TypeOfMemberObject_T = typename TypeOfMemberObject<T>::type;

  /**
   * Extracts class of any member pointer (function or object).
   * @tparam T Member pointer.
   */
  template<typename T>
  struct ClassOfMember;

  template<typename T>
  requires (std::is_member_function_pointer_v<T>)
  struct ClassOfMember<T> : public ClassOfMemberFunction<T> {};

  template<typename T>
  requires (std::is_member_object_pointer_v<T>)
  struct ClassOfMember<T> : public ClassOfMemberObject<T> {};

  /**
   * Extracts class of any member pointer (function or object).
   * @tparam T Member pointer.
   */
  template<typename T>
  requires (std::is_member_pointer_v<T>)
  using ClassOfMember_T = typename ClassOfMember<T>::type;

  /**
   * Checks if type is an instance of provided template (type only).
   * @tparam T Any type.
   * @tparam Template Any template accepting only type parameters.
   */
  template<typename T, template<typename...> typename Template>
  struct IsInstanceOf : public std::false_type {};

  template<template<typename...> typename Template, typename... Args>
  struct IsInstanceOf<Template<Args...>, Template> : public std::true_type {};

  /**
   * Checks if type is an instance of provided template (type only).
   * @tparam T Any type.
   * @tparam Template Any template accepting only type parameters.
   */
  template<typename T, template<typename...> typename Template>
  constexpr bool IsInstanceOf_V = IsInstanceOf<T, Template>::value;

  /**
   * Checks if type is an instance of provided template (NTTP only).
   * @tparam T Any type.
   * @tparam Template Any template accepting only non-type parameters.
   */
  template<typename T, template<auto...> typename Template>
  struct IsInstanceOf_NTTP : public std::false_type {};

  template<template<auto...> typename Template, auto... Args>
  struct IsInstanceOf_NTTP<Template<Args...>, Template> : public std::true_type {};

  /**
   * Checks if type is an instance of provided template (NTTP only).
   * @tparam T Any type.
   * @tparam Template Any template accepting only non-type parameters.
   */
  template<typename T, template<auto...> typename Template>
  constexpr bool IsInstanceOf_NTTP_V = IsInstanceOf_NTTP<T, Template>::value;

  /**
   * Checks if type is iterable.
   * @tparam T Any type.
   */
  template<typename T>
  struct IsIterable : public std::false_type {};

  template<typename T>
  requires (requires (T& t)
  {
    { std::begin(t) };
    { std::end(t) };
  })
  struct IsIterable<T> : public std::true_type {};

  /**
   * Checks if type is iterable.
   * @tparam T Any type.
   */
  template<typename T>
  constexpr bool IsIterable_V = IsIterable<T>::value;
}


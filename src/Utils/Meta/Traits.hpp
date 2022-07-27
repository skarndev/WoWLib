#pragma once
#include <type_traits>

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
    typedef Class type;
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
  struct TypeOfMemberObject;

  template<typename Class, typename Value>
  struct TypeOfMemberObject<Value Class::*>
  {
    typedef Value type;
  };

  /**
   * Extracts type of member object pointer.
   * @tparam T Member object pointer
   */
  template<typename T>
  requires (std::is_member_object_pointer_v<T>)
  using TypeOfMemberObject_T = typename TypeOfMemberObject<T>::type;
}


#pragma once
#include <type_traits>
#include <Utils/Meta/Concepts.hpp>
#include <Utils/Meta/Traits.hpp>
#include <Utils/Meta/Templates.hpp>
#include <Utils/Meta/Algorithms.hpp>
#include <Utils/Meta/DataTypes.hpp>
#include <nameof.hpp>
#include <boost/preprocessor.hpp>
#include <boost/callable_traits.hpp>

#include <string_view>

namespace Utils::Meta::Reflection
{
  namespace details
  {
    template<typename T, Templates::StringLiteral type_name, auto... mem_ptrs>
    requires (Concepts::IsMemberOf<decltype(mem_ptrs), T> && ...)
    struct ReflectionDescriptorImpl
    {
      static constexpr std::array<std::string_view, sizeof...(mem_ptrs)> field_names = {NAMEOF_MEMBER(mem_ptrs) ...};
      static constexpr std::size_t n_members = sizeof...(mem_ptrs);
      static constexpr Templates::StringLiteral this_type_name = type_name;

      template<Templates::StringLiteral member_name>
      static constexpr bool HasMember()
      {
        bool has_member = false;
        INLINE_FOR(i, 0, ReflectionDescriptorImpl::field_names.size())
        {
          constexpr const char* cur_member_name_cstr = ReflectionDescriptorImpl::field_names[i].data();

          if constexpr (Templates::StringLiteral<ReflectionDescriptorImpl::field_names[i].size() + 1>
                          {cur_member_name_cstr, ReflectionDescriptorImpl::field_names[i].size()} == member_name)
            has_member = true;
        };

        return has_member;
      };

      template<Templates::StringLiteral member_name>
      requires (HasMember<member_name>())
      static consteval bool IsMemberFunc()
      {
        return std::is_member_function_pointer_v<decltype(GetMemberPtr<member_name>())>;
      }

      template<Templates::StringLiteral member_name>
      requires (HasMember<member_name>())
      static consteval bool IsMemberVar()
      {
        return std::is_member_object_pointer_v<decltype(GetMemberPtr<member_name>())>;
      }

      template<Templates::StringLiteral member_name>
      requires (HasMember<member_name>() && IsMemberVar<member_name>())
      static auto& GetMember(T& object)
      {
        constexpr auto member_ptr = GetMemberPtr<member_name>();
        return object.*member_ptr;
      }

      template<Templates::StringLiteral member_name>
      requires (HasMember<member_name>() && IsMemberVar<member_name>())
      static auto const& GetMember(T const& object)
      {
        constexpr auto member_ptr = GetMemberPtr<member_name>();
        return object.*member_ptr;
      }

      template<Templates::StringLiteral member_name, typename...Args>
      requires (HasMember<member_name>() && IsMemberFunc<member_name>())
      static auto InvokeMemberFunc(T& object, Args... args)
      {
        constexpr auto member_ptr = GetMemberPtr<member_name>();

        return (object.*member_ptr)(std::forward<Args>(args)...);
      }

      template<Templates::StringLiteral member_name>
      static consteval auto GetMemberPtr()
      requires (HasMember<member_name>())
      {
        using ret = decltype(Algorithms::FindNTTP<DataTypes::ConstPack<mem_ptrs...>
          , [](auto var, auto i) -> bool
          {
            constexpr std::string_view cur_member_name = NAMEOF_MEMBER(decltype(var)::value);
            constexpr const char* cur_member_name_cstr = cur_member_name.data();

            return Templates::StringLiteral<cur_member_name.size() + 1>{cur_member_name_cstr, cur_member_name.size()}
            == member_name;
          }>);

        return ret::value;
      }

      template<typename Func>
      requires (std::is_invocable_r_v<void, Func, decltype(mem_ptrs), std::string_view> && ...)
      static auto ForEachMember(Func&& func)
      {
        INLINE_FOR_EACH_NTTP(DataTypes::ConstPack<mem_ptrs...>, cur_mem_ptr, i)
        {
          func(cur_mem_ptr, ReflectionDescriptorImpl::field_names[i]);
        };
      }
    };

  }

  template<typename T>
  struct Reflect;

  namespace Concepts
  {
    template<typename T>
    concept Reflectable = requires { { Reflect<T>{} }; };
  }
}

#define MAKE_MEMBER_PTR(R, TYPENAME, I, ELEM) \
BOOST_PP_COMMA_IF(I)  &TYPENAME::ELEM

#define REFLECTION_DESCRIPTOR(TYPENAME, ...) \
template<> \
struct Utils::Meta::Reflection::Reflect<TYPENAME> \
: public Utils::Meta::Reflection::details::ReflectionDescriptorImpl \
         < \
         TYPENAME, #TYPENAME, BOOST_PP_SEQ_FOR_EACH_I(MAKE_MEMBER_PTR, TYPENAME, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
         > \
 {};

#undef MAKE_MEMBERPTR
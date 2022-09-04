#pragma once
#include <type_traits>
#include <Utils/Meta/Concepts.hpp>
#include <Utils/Meta/Traits.hpp>
#include <Utils/Meta/Templates.hpp>
#include <Utils/Meta/Algorithms.hpp>
#include <nameof.hpp>
#include <boost/preprocessor.hpp>

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
      static consteval auto GetMemberPtr()
      {
        return GetMemberPtrImpl<member_name, mem_ptrs...>();
      }

    private:
      template<Templates::StringLiteral member_name, auto cur_member_ptr, auto... rem_member_ptrs>
      static consteval auto GetMemberPtrImpl()
      {
        constexpr std::string_view cur_member_name = NAMEOF_MEMBER(cur_member_ptr);
        constexpr const char* cur_member_name_cstr = cur_member_name.data();

        if constexpr (Templates::StringLiteral<cur_member_name.size() + 1>
                       {cur_member_name_cstr, cur_member_name.size()} == member_name)
          return cur_member_ptr;
        else
        {
          if constexpr (sizeof...(rem_member_ptrs) > 0)
          {
            return GetMemberPtrImpl<member_name, rem_member_ptrs...>();
          }
          else
          {
            return nullptr;
          }
        }
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
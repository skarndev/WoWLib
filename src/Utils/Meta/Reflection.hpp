#pragma once
#include <type_traits>
#include <Utils/Meta/Concepts.hpp>
#include <Utils/Meta/Traits.hpp>
#include <Utils/Meta/Templates.hpp>
#include <Utils/Meta/Algorithms.hpp>
#include <Utils/Meta/DataTypes.hpp>
#include <Validation/Log.hpp>
#include <nameof.hpp>
#include <boost/preprocessor.hpp>
#include <boost/callable_traits.hpp>

#include <string_view>

/**
 * Contains type implementing compile-time static type reflection for types.
 */
namespace Utils::Meta::Reflection
{
  template<typename T>
  struct Reflect;

  namespace Concepts
  {
    template<typename T>
    concept Reflectable = requires { { Reflect<T>{} }; };
  }

  namespace details
  {
    template<typename T, Templates::StringLiteral type_name, auto... mem_ptrs>
    requires (Utils::Meta::Concepts::IsMemberOf<decltype(mem_ptrs), T> && ...)
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
        auto ret = Algorithms::FindNTTP<DataTypes::ConstPack<mem_ptrs...>
          , [](auto var, auto i) -> bool
          {
            constexpr std::string_view cur_member_name = NAMEOF_MEMBER(decltype(var)::value);
            constexpr const char* cur_member_name_cstr = cur_member_name.data();

            return Templates::StringLiteral<cur_member_name.size() + 1>{cur_member_name_cstr, cur_member_name.size()}
            == member_name;
          }>;

        return decltype(ret)::value;
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

      template<typename LogFunc>
      static void PrettyPrint(LogFunc&& log_func, T const& instance)
      {
        log_func("type %s /* (sizeof: %d) */", NAMEOF_TYPE(T), sizeof(T));
        log_func("{");
        {
          LogIndentScoped;
          INLINE_FOR_EACH_NTTP(DataTypes::ConstPack<mem_ptrs...>, cur_mem_ptr, i)
          {
            if constexpr (std::is_member_function_pointer_v<decltype(cur_mem_ptr)>)
              return;
            else
            {
              using MemberType = Traits::TypeOfMemberObject_T<decltype(cur_mem_ptr)>;
              PrettyPrintMember(log_func, instance.*cur_mem_ptr, NAMEOF_MEMBER(cur_mem_ptr));
            }
          };
        }
        log_func("}");
      }

      template<typename LogFunc>
      static void PrettyPrint(LogFunc&& log_func, T const& instance, std::string_view field_name)
      {
        log_func("type %s", NAMEOF_TYPE(T));
        log_func("{");
        {
          LogIndentScoped;
          INLINE_FOR_EACH_NTTP(DataTypes::ConstPack<mem_ptrs...>, cur_mem_ptr, i)
          {
            if constexpr (std::is_member_function_pointer_v<decltype(cur_mem_ptr)>)
              return;
            else
            {
              using MemberType = Traits::TypeOfMemberObject_T<decltype(cur_mem_ptr)>;
              PrettyPrintMember(log_func, instance.*cur_mem_ptr, NAMEOF_MEMBER(cur_mem_ptr));
            }
          };
        }
        log_func("} %s;", field_name);
      }

    private:
      template<typename LogFunc, typename IterableType>
      static void PrettyPrintIterable(LogFunc&& log_func, IterableType const& iterable)
      {
        using ValueType = std::remove_cvref_t<decltype(*std::begin(iterable))>;

        log_func("{");
        {
          LogIndentScoped;

          if constexpr (Utils::Meta::Reflection::Concepts::Reflectable<ValueType>)
          {
            for (auto const& each : iterable)
            {
              Utils::Meta::Reflection::Reflect<ValueType>::PrettyPrint(log_func, each);
              log_func(",");
            }
          }
          else if constexpr (Utils::Meta::Traits::IsIterable_V<ValueType>)
          {
            for (auto const& each : iterable)
            {
              log_func("%s /* (size: %d) */"
                       , NAMEOF_TYPE(ValueType)
                       , std::distance(std::begin(each)
                                       , std::end(each)));
              PrettyPrintIterable(log_func, each);
            }
          }
          else if constexpr (requires (ValueType t) { { log_func("%s", t) }; } )
          {
            for (auto const& each : iterable)
            {
              log_func("%s,", each);
            }
          }
          else
          {
            log_func("<Non-reflectable data...>");
          }
        }

        log_func("}");

      }

      template<typename LogFunc, typename MemberType>
      static void PrettyPrintMember(LogFunc&& log_func, MemberType const& cur_member, std::string_view member_name)
      {
        if constexpr (Utils::Meta::Reflection::Concepts::Reflectable<MemberType>)
        {
          Utils::Meta::Reflection::Reflect<MemberType>::PrettyPrint(log_func, cur_member, member_name);
        }
        else if constexpr (Utils::Meta::Traits::IsIterable_V<MemberType>)
        {
          log_func("%s %s /* (size: %d) */"
                   , NAMEOF_TYPE(MemberType)
                   , member_name
                   , std::distance(std::begin(cur_member)
                                   , std::end(cur_member))
                  );

          PrettyPrintIterable(log_func, cur_member);
        }
        else if constexpr (std::is_union_v<MemberType>)
        {
          log_func("%s %s = %s;"
                   , NAMEOF_SHORT_TYPE(MemberType)
                   , member_name
                   , "<Non-reflectable value>");
        }
        else if constexpr (requires (MemberType t) { { log_func("%s", t) }; } )
        {
          log_func("%s %s = %s;", NAMEOF_TYPE(MemberType), member_name, cur_member);
        }
        else
        {
          log_func("%s %s = %s;"
                   , NAMEOF_SHORT_TYPE(MemberType)
                   , member_name
                   , "<Non-reflectable value>");

        }
      }
    };

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
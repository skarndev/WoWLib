#pragma once
#include <type_traits>

namespace Utils::Meta::ProtocolHelpers
{
  template<typename...>
  struct ArgPack {};

  struct Any {};

  enum class SigCheckPolicy
  {
    Exact,
    Partial
  };

  template<typename, typename = ArgPack<>, SigCheckPolicy sig_policy = SigCheckPolicy::Exact>
  struct HasMethod;

#define HASMETHOD_OVERLOADS(...)\
  template<typename Class, typename... Args>\
  requires (sig_policy == SigCheckPolicy::Partial && !std::is_same_v<Ret, Any>)\
  explicit HasMethod(Ret(Class::*)(Ts..., Args...) __VA_ARGS__) {};\
\
  template<typename Class>\
  requires (sig_policy == SigCheckPolicy::Exact && !std::is_same_v<Ret, Any>)\
  explicit HasMethod(Ret(Class::*)(Ts...) __VA_ARGS__) {};\
\
  template<typename Class, typename AnyRet, typename... Args>\
  requires (sig_policy == SigCheckPolicy::Partial && std::is_same_v<Ret, Any>)\
  explicit HasMethod(AnyRet(Class::*)(Ts..., Args...) __VA_ARGS__) {};\
\
  template<typename Class, typename AnyRet>\
  requires (sig_policy == SigCheckPolicy::Exact && std::is_same_v<Ret, Any>)\
  explicit HasMethod(AnyRet(Class::* )(Ts...) __VA_ARGS__) {};

  template<typename Ret, typename... Ts, SigCheckPolicy sig_policy>
  struct HasMethod<Ret, ArgPack<Ts...>, sig_policy>
  {
    HASMETHOD_OVERLOADS();
    HASMETHOD_OVERLOADS(&);
    HASMETHOD_OVERLOADS(&&);
    HASMETHOD_OVERLOADS(noexcept);
    HASMETHOD_OVERLOADS(& noexcept);
    HASMETHOD_OVERLOADS(&& noexcept);
    HASMETHOD_OVERLOADS(const);
    HASMETHOD_OVERLOADS(const &);
    HASMETHOD_OVERLOADS(const &&);
    HASMETHOD_OVERLOADS(const noexcept);
    HASMETHOD_OVERLOADS(const & noexcept);
    HASMETHOD_OVERLOADS(const && noexcept);
    HASMETHOD_OVERLOADS(const volatile);
    HASMETHOD_OVERLOADS(const volatile &);
    HASMETHOD_OVERLOADS(const volatile &&);
    HASMETHOD_OVERLOADS(const volatile noexcept);
    HASMETHOD_OVERLOADS(const volatile & noexcept);
    HASMETHOD_OVERLOADS(const volatile && noexcept);
    HASMETHOD_OVERLOADS(volatile);
    HASMETHOD_OVERLOADS(volatile &);
    HASMETHOD_OVERLOADS(volatile &&);
    HASMETHOD_OVERLOADS(volatile noexcept);
    HASMETHOD_OVERLOADS(volatile & noexcept);
    HASMETHOD_OVERLOADS(volatile && noexcept);
  };
}
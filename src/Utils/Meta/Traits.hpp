#include <type_traits>

namespace Utils::Meta::Traits
{
  namespace Impl
  {
    template<typename>
    struct TypeOfMemberPtr;

    template
    <
      typename Class,
      typename T
    >
      struct TypeOfMemberPtr<T Class::*>
    {
      typedef T Type;
    };

    template<typename>
    struct ClassOfMemberPtr;

    template
    <
      typename Class,
      typename T
    >
      struct ClassOfMemberPtr<T Class::*>
    {
      typedef Class Type;
    };

    template<typename>
    struct ArgOfTemplate;

    template
    <
      template<typename>
      typename Template,
      typename T
    >
      struct ArgOfTemplate<Template<T>>
    {
      typedef T Type;
    };

    struct IFNDR;

  };

  template<auto p>
  using TypeOfMemberPtr = typename Impl::TypeOfMemberPtr<decltype(p)>::Type;

  template<typename T>
  using ArgOfTemplate = typename Impl::ArgOfTemplate<T>::Type;

  template<auto p>
  using ClassOfMemberPtr = typename Impl::ClassOfMemberPtr<decltype(p)>::Type;

  template<typename T>
  constexpr bool FALSE_T = std::is_same_v<Impl::IFNDR, T> ;


}


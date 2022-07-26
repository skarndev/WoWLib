#pragma once
#include <IO/Common.hpp>
#include <Utils/Meta/Templates.hpp>
#include <Utils/Meta/Traits.hpp>
#include <Utils/Misc/ForceInline.hpp>
#include <boost/callable_traits/return_type.hpp>

#include <functional>
#include <type_traits>

namespace IO::Common::Traits
{
  namespace details
  {
    /**
     * Per class unique empty base.
     * @tparam T Any type.
     */
    template<typename T>
    struct EmptyTrait
    {
    };

    /**
     * Raises a compiletime error if no trait was selected on instantiation of SwitchableTrait.
     * @tparam T Any type.
     */
    template<typename T>
    struct SwitchError : std::true_type
    {
      using type = T;
      static_assert(sizeof(T) == 0, "Failed matching SwitchableTrait.");
    };

    /**
     * Used to skip a versioned case of SwitchableTrait.
     * @tparam T Any type.
     */
    template<typename T>
    struct SwitchFalse : std::false_type
    {
      using type = T;
    };

    /**
     * Performs validation of trait interface.
     * @tparam T Trait candidate type.
     */
    template<typename T>
    struct ValidateTrait : public T
    {
    private:
      static constexpr bool Test()
      {
        return requires
          {
            { &ValidateTrait<T>::Read };
            { &ValidateTrait<T>::Write };
          };
      }

    public:
      static constexpr bool result = Test();
    };

    /**
     * Compute if two values are equal to each other and pass the provided type.
     * To be used with std::disjunction / std::conjunction and alike.
     * @tparam V1 First value.
     * @tparam V2 Second value.
     * @tparam T Type to store.
     */
    template<auto V1, decltype(V1) V2, typename T>
    struct ValuesEqualT : std::bool_constant<V1 == V2>
    {
      using type = T;
    };
  }

  /**
   * Checks if type satisfies the named requirements to be used as any of the IO traits
   * @tparam T Any type.
   */
  template<typename T>
  concept IOTraitNamedReq = details::ValidateTrait<T>::result;

  /**
   * Provides a type for the optional trait of a chunk / class, present based on the client version.
   * @tparam T Any type that satisfies named requirements of a trait (see associated concept).
   * @tparam current_version Current version.
   * @tparam version_min Minimum version of the client this trait is supported for.
   * @tparam version_max Maximum version of the client this trait is supported for.
   */
  template
  <
    IOTraitNamedReq T
    , Common::ClientVersion current_version
    , Common::ClientVersion version_min = Common::ClientVersion::CLASSIC
    , Common::ClientVersion version_max = Common::ClientVersion::ANY
  >
  using VersionTrait = std::conditional_t
    <
      current_version >= version_min && current_version <= version_max
      , T
      , details::EmptyTrait<T>
    >;

  /**
   * Represents one of the trait options to be used with IO::Common::Traits::SwitchableTrait
   * @tparam val Trait identifier to match against, e.g. an enum.
   * @tparam T Type of trait implementation.
   * @tparam do_compute Enable computation of this case. If false, the case is false no matter the matching.
   */
  template<auto val, IOTraitNamedReq T, bool do_compute = true>
  struct TraitCase
  {
    static constexpr auto value = val;
    static constexpr bool use = do_compute;
    using type = T;
  };

  /**
   * Represents of of the trait options to be used with IO::Common::Traits::SwitchableTrait
   * @tparam val Trait identifier to match against, e.g. an enum.
   * @tparam T Type of trait implementation.
   * @tparam client_version Current version of the client.
   * @tparam version_min Minimal version of the client this trait is supported in.
   * @tparam version_max Maximum version of the client this trait is supported in.
   */
  template
  <
    auto val
    , IOTraitNamedReq T
    , Common::ClientVersion client_version
    , Common::ClientVersion version_min = Common::ClientVersion::CLASSIC
    , Common::ClientVersion version_max = Common::ClientVersion::ANY
  >
  using VersionedTraitCase = TraitCase<val, T, client_version >= version_min && client_version <= version_max>;

  /**
   * Switches traits in switch-like fashion.
   * Options are defined with IO::Common::Traits::TraitCase
   * @tparam T Any type that satisfies named requirements of a trait (see associated concept).
   * @tparam which Current trait to enable. If not matched, no trait is enabled.
   */
  template
  <
    auto which
    , typename...Options
  >
  requires (std::is_same_v<Options, TraitCase<Options::value, typename Options::type, Options::use>> && ...)
  using SwitchableTrait = typename std::disjunction
    <
      Utils::Meta::Templates::ValuesEqualT<which, Options::value, typename Options::type, Options::use>...,
      details::SwitchError<decltype(which)>
    >::type;

  /**
  * Checks if a trait exists and is enabled for type T.
  * @tparam T Any type.
  * @tparam Trait Trait implementation.
  */
  template
  <
    typename T,
    typename Trait
  >
  concept HasTraitEnabled = IOTraitNamedReq<Trait> && std::is_base_of_v<Trait, T>;

  /**
  * Checks if version trait exists for type T. Only works for traits bound with VersionTrait directly.
  * @tparam T Any type.
  * @tparam Trait VersionTrait implementation.
  */
  template
  <
    typename T,
    typename Trait
  >
  concept HasVersionTrait = IOTraitNamedReq<Trait> && (std::is_base_of_v<Trait, T>
    || std::is_base_of_v<details::EmptyTrait<Trait>, T>);

  /**
   * Checks if passed type is either an enabled trait or an empty.
   * @tparam T Any type.
   */
  template<typename T>
  concept IOTraitOrEmpty = std::is_empty_v<T> || IOTraitNamedReq<T>;

  template<typename T>
  concept TraitFeaturePointer = std::is_member_function_pointer_v<T>
    && (std::is_trivially_constructible_v<typename boost::callable_traits::return_type<T>::type>
    || std::is_same_v<typename boost::callable_traits::return_type<T>::type, void>);

  /**
   * Adapter class accepting all traits of class, and providing convenience polling methods.
   * It is recommended to use traits through this adapter class, but not required.
   * @tparam Traits
   */
  template<IOTraitOrEmpty ... Traits>
  struct IOTraits : public Traits ...
  {
    template<TraitFeaturePointer MethodPtrT, typename... Args>
    requires (HasTraitEnabled
              <
                std::remove_pointer_t<IOTraits>
                , typename Utils::Meta::Traits::ClassOfMemberFunction<MethodPtrT>::type
              >)
    auto InvokeExistingTraitFeature(MethodPtrT feature, Args&&... args)
    {
      return (this->*feature)(std::forward<Args>(args)...);
    }

    template<typename MethodPtrT, typename... Args>
    requires (!HasTraitEnabled
              <
                std::remove_pointer_t<IOTraits>
                , typename Utils::Meta::Traits::ClassOfMemberFunction<MethodPtrT>::type
              >)
    auto InvokeExistingTraitFeature(MethodPtrT feature, Args&&...)
    {
      if constexpr(std::is_default_constructible_v<typename boost::callable_traits::return_type<MethodPtrT>::type>)
      {
        return typename boost::callable_traits::return_type<MethodPtrT>::type{};
      }
    }

    template<typename MethodPtrT, typename... Args>
    requires (HasTraitEnabled
              <
                std::remove_pointer_t<IOTraits>
                , typename Utils::Meta::Traits::ClassOfMemberFunction<MethodPtrT>::type
              >)
    auto InvokeExistingTraitFeature(MethodPtrT feature, Args&&... args) const
    {
      return (this->*feature)(std::forward<Args>(args)...);
    }

    template<typename MethodPtrT, typename... Args>
    requires (!HasTraitEnabled
              <
                std::remove_pointer_t<IOTraits>
                , typename Utils::Meta::Traits::ClassOfMemberFunction<MethodPtrT>::type
              >)
    auto InvokeExistingTraitFeature(MethodPtrT feature, Args&&...) const
    {
      if constexpr(std::is_default_constructible_v<typename boost::callable_traits::return_type<MethodPtrT>::type>)
      {
        return typename boost::callable_traits::return_type<MethodPtrT>::type{};
      }
    }
  };


  template<typename CRTP>
  class AutoIOTrait
  {
  private:
    template<auto...>
    struct ChunkList {};

    template<typename Func>
    bool HandleCases(std::uint32_t, ChunkList<>, Func&& func) { return false; }

    template<typename Func, auto current_chunk, auto... chunks>
    bool HandleCases(std::uint32_t i, ChunkList<current_chunk, chunks...>, Func&& func)
    {
      if (Utils::Meta::Traits::TypeOfMemberObject_T<decltype(current_chunk)>::magic != i)
      {
        return HandleCases(i, ChunkList<chunks...>());
      }

      func(static_cast<CRTP*>(this)->*current_chunk);

      return true;
    }

    template<auto... chunks, typename Func>
    bool HandleCases(std::uint32_t i, Func&& func)
    {
      return HandleCases(i, ChunkList<chunks...>(), func);
    }

  protected:
    template<auto ...chunks>
    requires (Common::Concepts::ChunkProtocolCommon<Utils::Meta::Traits::TypeOfMemberObject_T<decltype(chunks)>> && ...)
    bool ReadChunk(std::uint32_t fourcc, Common::ByteBuffer const& buf, std::size_t size)
    {
      return HandleCases<chunks...>(fourcc, [&buf, size](auto& chunk) { chunk.Read(buf, size); });
    }

  public:

  };
}

/**
 * Checks validity of types to satisfy trait named requirements.
 * It is recommended to be used when defining traits to validate those at compile time.
 * Usage:
 *  class Foo
 *  {
 *    void Read();
 *    void Write() const;
 *  } IMPLEMENTS_IO_TRAIT(Foo);
 */
#define IMPLEMENTS_IO_TRAIT(NAME) \
  ; static_assert(IO::Common::Traits::IOTraitNamedReq<NAME> \
  && "Trait candidate definition <#NAME> does not satisfy trait named requirements.")


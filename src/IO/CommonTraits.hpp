#pragma once
#include <IO/Common.hpp>
#include <Utils/Meta/Templates.hpp>
#include <Utils/Meta/Traits.hpp>
#include <Utils/Misc/ForceInline.hpp>
#include <boost/callable_traits/return_type.hpp>

#include <functional>
#include <type_traits>
#include <concepts>

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

  /**
   * Checks if passed type is a pointer to member function that can represent a trait feature.
   * @tparam T Any type.
   */
  template<typename T>
  concept TraitFeaturePointer = std::is_member_function_pointer_v<T>
    && (std::is_trivially_constructible_v<typename boost::callable_traits::return_type<T>::type>
    || std::is_same_v<typename boost::callable_traits::return_type<T>::type, void>);

  /**
   * Checks if passed type is a pointer to member function that satisfies the provided signature.
   * @tparam T Any type.
   */
  template<typename T>
  concept ReadFeaturePointer = TraitFeaturePointer<T>
     && std::is_same_v<T, bool(Utils::Meta::Traits::ClassOfMemberFunction_T<T>::*)(Common::ByteBuffer const&
                                                                                   , Common::ChunkHeader const&)>;

  /**
    * Checks if passed type is a pointer to member function that satisfies the provided signature.
    * @tparam T Any type.
    */
  template<typename T>
  concept ExtendedReadFeaturePointer = TraitFeaturePointer<T>
    && std::is_same_v<T, bool(Utils::Meta::Traits::ClassOfMemberFunction_T<T>::*)(Common::ByteBuffer const&
                                                                                 , Common::ChunkHeader const&
                                                                                 , std::uint32_t& chunk_counter)>;
  /**
    * Checks if passed type is a pointer to member function that satisfies the provided signature.
    * @tparam T Any type.
    */
  template<typename T>
  concept WriteFeaturePointer = TraitFeaturePointer<T>
    && std::is_same_v<T, void(Utils::Meta::Traits::ClassOfMemberFunction_T<T>::*)(Common::ByteBuffer&) const>;

  /**
   * Adapter class accepting all traits of class, and providing convenience polling methods.
   * It is recommended to use traits through this adapter class, but not required.
   * @tparam Traits
   */
  template<IOTraitOrEmpty ... Traits>
  struct IOTraits : public Traits ...
  {
    template<auto feature, typename... Args>
    requires (HasTraitEnabled
              <
                std::remove_pointer_t<IOTraits>
                , typename Utils::Meta::Traits::ClassOfMemberFunction<decltype(feature)>::type
              >)
    auto InvokeExistingTraitFeature(Args&&... args)
    {
      return (this->*feature)(std::forward<Args>(args)...);
    }

    template<auto feature, typename... Args>
    requires (!HasTraitEnabled
              <
                std::remove_pointer_t<IOTraits>
                , typename Utils::Meta::Traits::ClassOfMemberFunction<decltype(feature)>::type
              >)
    auto InvokeExistingTraitFeature([[maybe_unused]] Args&&...)
    {
      if constexpr(std::is_default_constructible_v<typename boost::callable_traits::return_type<decltype(feature)>::type>)
      {
        return typename boost::callable_traits::return_type<decltype(feature)>::type{};
      }
    }

    template<auto feature, typename... Args>
    requires (HasTraitEnabled
              <
                std::remove_pointer_t<IOTraits>
                , typename Utils::Meta::Traits::ClassOfMemberFunction<decltype(feature)>::type
              >)
    auto InvokeExistingTraitFeature(Args&&... args) const
    {
      return (this->*feature)(std::forward<Args>(args)...);
    }

    template<auto feature, typename... Args>
    requires (!HasTraitEnabled
              <
                std::remove_pointer_t<IOTraits>
                , typename Utils::Meta::Traits::ClassOfMemberFunction<decltype(feature)>::type
              >)
    auto InvokeExistingTraitFeature([[maybe_unused]] Args&&...) const
    {
      if constexpr(std::is_default_constructible_v<typename boost::callable_traits::return_type<decltype(feature)>::type>)
      {
        return typename boost::callable_traits::return_type<decltype(feature)>::type{};
      }
    }

    template<auto... features>
    requires ( (IOTraitNamedReq<Utils::Meta::Traits::ClassOfMemberFunction_T<decltype(features)>> && ...)
    && ( (ReadFeaturePointer<decltype(features)> || ExtendedReadFeaturePointer<decltype(features)>) && ... ) )
    bool InvokeExistingCommonReadFeatures(Common::ByteBuffer const& buf
                                          , Common::ChunkHeader const& chunk_header
                                          , std::uint32_t& chunk_counter)
    {
      return HandleReadCases(FeatureList<features...>(), buf, chunk_header, chunk_counter);
    };

    template<auto... features>
    requires ( (IOTraitNamedReq<Utils::Meta::Traits::ClassOfMemberFunction_T<decltype(features)>> && ...)
               && ( WriteFeaturePointer<decltype(features)> && ... ) )
    void InvokeExistingCommonWriteFeatures(Common::ByteBuffer& buf) const
    {
      (InvokeExistingTraitFeature<features>(buf), ...);
    }


  private:
    template<auto...>
    struct FeatureList
    {
    };

    bool HandleReadCases(FeatureList<>
                     , [[maybe_unused]] Common::ByteBuffer const& buf
                     , [[maybe_unused]] Common::ChunkHeader const& chunk_header
                     , [[maybe_unused]] std::uint32_t& chunk_counter)
    { return false; }

    template<auto current_feature, auto... features>
    bool HandleReadCases(FeatureList<current_feature, features...>
                      , Common::ByteBuffer const& buf
                      , Common::ChunkHeader const& chunk_header
                      , std::uint32_t& chunk_counter)
    {
      // handle (buf, chunk_header, chunk_counter) overload variant
      if constexpr (ExtendedReadFeaturePointer<decltype(current_feature)>)
      {
        if (InvokeExistingTraitFeature<current_feature>(buf, chunk_header, chunk_counter))
          return true;

      }
      // handle (buf, chunk_header) overload
      else if constexpr (ReadFeaturePointer<decltype(current_feature)>)
      {
        if (InvokeExistingTraitFeature<current_feature>(buf, chunk_header))
          return true;
      }
      else
      {
        static_assert(sizeof(current_feature) != 0 && "Not implemented. Concepts are wrong."
                                                      " Should not have even gotten here.");
      }

      return HandleReadCases(FeatureList<features...>(), buf, chunk_header, chunk_counter);
    }
};


  template<typename CRTP>
  class AutoIOTrait
  {
  private:
    template<auto...>
    struct ChunkList
    {
    };

    template<typename Func>
    bool HandleCases(std::uint32_t, ChunkList<>, [[maybe_unused]] Func&& func)
    { return false; }

    template<typename Func, auto current_chunk, auto... chunks>
    bool HandleCases(std::uint32_t i, ChunkList<current_chunk, chunks...>, Func&& func)
    {
      if (Utils::Meta::Traits::TypeOfMemberObject_T<decltype(current_chunk)>::magic != i)
      {
        return HandleCases(i, ChunkList<chunks...>(), func);
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

    template<auto ...chunks>
    requires (Common::Concepts::ChunkProtocolCommon<Utils::Meta::Traits::TypeOfMemberObject_T<decltype(chunks)>> && ...)
    void WriteChunks(Common::ByteBuffer& buf) const
    {
      auto write_lambda = [&buf](auto const& chunk) { chunk.Write(buf);};
      (write_lambda(static_cast<const CRTP*>(this)->*chunks), ...);
    }

    template<auto ...chunks, typename Func>
    requires ((Common::Concepts::ChunkProtocolCommon<Utils::Meta::Traits::TypeOfMemberObject_T<decltype(chunks)>> && ...)
      && (std::invocable<std::add_lvalue_reference_t<
        Utils::Meta::Traits::TypeOfMemberObject_T<decltype(chunks)>>> && ...))
    void ForAllChunks(Func&& func)
    {
      (func(static_cast<const CRTP*>(this)->*chunks), ...);
    }
    template<auto ...chunks, typename Func>
    requires ((Common::Concepts::ChunkProtocolCommon<Utils::Meta::Traits::TypeOfMemberObject_T<decltype(chunks)>> && ...)
      && (std::invocable<std::add_const<std::add_lvalue_reference_t<
        Utils::Meta::Traits::TypeOfMemberObject_T<decltype(chunks)>>>> && ...))
    void ForAllChunks(Func&& func) const
    {
      (func(static_cast<const CRTP*>(this)->*chunks), ...);
    }
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


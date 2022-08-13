#pragma once
#include <IO/Common.hpp>
#include <Utils/Meta/Templates.hpp>
#include <Utils/Meta/Traits.hpp>
#include <Utils/Misc/ForceInline.hpp>
#include <boost/callable_traits/return_type.hpp>

#include <nameof.hpp>

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
   * @tparam Trait Trait type.
   */
  template<typename T>
  concept IOTraitOrEmpty = std::is_empty_v<T> || IOTraitNamedReq<T>;

  /**
   * Adapter class accepting all traits of class, and providing a common interface to invoke them.
   * It is recommended to use traits through this adapter class, but not required.
   * @tparam Traits
   */
  template<IOTraitOrEmpty ... Traits>
  struct IOTraits : public Traits ...
  {
    template<typename CRTP>
    friend class AutoIOTraitInterface;

  private:
    template<typename... Ts>
    struct TypePack {};

  protected:
    bool TraitsRead(Common::ByteBuffer const& buf
                                            , Common::ChunkHeader const& chunk_header)
    {
      return RecurseRead(buf, chunk_header, TypePack<Traits...>());
    };

    void TraitsWrite(Common::ByteBuffer& buf) const
    {
      RecurseWrite(buf, TypePack<Traits...>());
    }

  private:
    template<typename T, typename... Ts>
    void RecurseWrite(Common::ByteBuffer& buf, TypePack<T, Ts...>) const
    {
      if constexpr (!std::is_empty_v<T>
        && HasTraitEnabled<IOTraits, T>
        && requires { { static_cast<void(T::*)(Common::ByteBuffer&)>(&T::Write) }; }
        )
      {
        T::Write(buf);
      }

      if constexpr (sizeof...(Ts))
      {
        RecurseWrite(buf, TypePack<Ts...>());
      }
    }

    template<typename T, typename...Ts>
    [[nodiscard]]
    bool RecurseRead(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header, TypePack<T, Ts...>)
    {
      if constexpr (!std::is_empty_v<T>
        && HasTraitEnabled<IOTraits, T>
        && requires {
                      { static_cast<bool(T::*)(Common::ByteBuffer const&, Common::ChunkHeader const&) const>(&T::Read) };
                    }
        )
      {
        if (T::Read(buf))
          return true;
      }

      if constexpr (sizeof...(Ts))
      {
        if (RecurseRead(buf, chunk_header, TypePack<Ts...>()))
          return true;
      }

      return false;
    }
  };

  enum class TraitType
  {
    Component,
    File
  };

  template<typename CRTP, TraitType trait_type = TraitType::Component>
  class AutoIOTraitInterface
  {
  private:
    CRTP* GetThis() { return static_cast<CRTP*>(this); };
    CRTP const* GetThis() const { return static_cast<CRTP const*>(this); };

  public:

    [[nodiscard]]
    bool Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
    requires (trait_type == TraitType::Component)
    {
      return decltype(CRTP::_auto_trait)::template ReadChunk(GetThis(), buf, chunk_header);
    };

    void Read(Common::ByteBuffer const& buf)
    requires (trait_type == TraitType::File)
    {
      LogDebugF(CCodeZones::FILE_IO, "Reading %s file...", NAMEOF_TYPE(CRTP));

      RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
      RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");

      while (!buf.IsEof())
      {
        auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

        // Use auto-trait if present in type
        if constexpr (requires { { &CRTP::_auto_trait }; })
        {
          if (decltype(CRTP::_auto_trait)::template ReadChunk(GetThis(), buf, chunk_header))
            continue;
        }

        // invoke optional method to handle unlisted chunks that cannot be processed automatically
        if constexpr (requires (CRTP crtp){ { crtp.ReadExtra(buf, chunk_header) } -> std::same_as<bool>; })
        {
          if (GetThis()->ReadExtra(buf, chunk_header))
            continue;
        }

        buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
        LogError("Encountered unknown chunk %s.", Common::FourCCToStr(chunk_header.fourcc));
      }

      LogDebugF(LCodeZones::FILE_IO, "Done reading %s", NAMEOF_TYPE(CRTP));
      EnsureF(CCodeZones::FILE_IO, buf.IsEof(), "Not all chunks have been parsed in the file. "
                                                "Bad logic or corrupt file.");
    }

    void Write(Common::ByteBuffer& buf) const
    {
      // Use auto-trait if present in type
      if constexpr (requires { { &CRTP::_auto_trait }; })
      {
        decltype(CRTP::_auto_trait)::template WriteChunks(GetThis(), buf);
      }

      // invoke optional method to handle unlisted chunks that cannot be processed automatically
      if constexpr (requires (CRTP crtp){ { crtp.WriteExtra(buf) } -> std::same_as<void>; })
      {
        GetThis()->WriteExtra(buf);
      }
    };

  };

  /**
   * Checks if provided type is a member pointer to a chunk object.
   * @tparam T Any type.
   */
  template<typename T>
  concept IsChunkMemberPointer = std::is_member_object_pointer_v<T>
    && Common::Concepts::ChunkProtocolCommon<Utils::Meta::Traits::TypeOfMemberObject_T<T>>;

  enum class IOHandlerType
  {
    Read,
    Write,
    Null
  };

  namespace details
  {
    struct Any {};
  }

  template<typename T>
  concept IsIOHandlerCallbackRead = std::is_invocable_v<T, details::Any*
    , DataChunk<std::uint32_t, FourCC<"TEST">>&, ByteBuffer const&, std::size_t>;

  template<typename T>
  concept IsIOHandlerCallbackWrite = std::is_invocable_v<T, details::Any const*
    , DataChunk<std::uint32_t, FourCC<"TEST">> const&, ByteBuffer&>;

  template<typename T>
  concept IsIOHandlerCallbackReadOptional = IsIOHandlerCallbackRead<T> || std::is_same_v<T, std::nullptr_t>;

  template<typename T>
  concept IsIOHandlerCallbackWriteOptional = IsIOHandlerCallbackWrite<T> || std::is_same_v<T, std::nullptr_t>;

  template<auto pre = nullptr, auto post = nullptr>
  requires
  (
    (IsIOHandlerCallbackReadOptional<decltype(pre)> && IsIOHandlerCallbackReadOptional<decltype(post)>)
    || (IsIOHandlerCallbackWriteOptional<decltype(pre)> && IsIOHandlerCallbackWriteOptional<decltype(post)>)
  )
  struct IOHandler
  {

  private:
    static constexpr IOHandlerType DetermineHandlerType()
    {
      if constexpr (std::is_same_v<decltype(pre), decltype(post)> && std::is_same_v<decltype(pre), std::nullptr_t>)
        return IOHandlerType::Null;

      if constexpr (IsIOHandlerCallbackReadOptional<decltype(pre)> && IsIOHandlerCallbackReadOptional<decltype(post)>)
        return IOHandlerType::Read;

      return IOHandlerType::Write;
    }

  public:
    static constexpr IOHandlerType handler_type = DetermineHandlerType();
    static constexpr bool has_pre = std::is_same_v<decltype(pre), std::nullptr_t>;
    static constexpr bool has_post = std::is_same_v<decltype(post), std::nullptr_t>;
    static constexpr auto callback_pre = pre;
    static constexpr auto callback_post = post;
  };

  /**
   * Checks whether a given type is IOHandler.
   * @tparam T Any type.
   */
  template<typename T, IOHandlerType type>
  concept IsIOHandler = requires
    {
      { &T::handler_type };
      { &T::has_pre };
      { &T::has_post };
      { &T::callback_pre };
      { &T::callback_post };
    } && (T::handler_type == type || T::handler_type == IOHandlerType::Null);

  template
  <
    auto chunk
    , IsIOHandler<IOHandlerType::Read> ReadHandler = IOHandler<nullptr, nullptr>
    , IsIOHandler<IOHandlerType::Write> WriteHandler = IOHandler<nullptr, nullptr>
  >
  requires
  (
      IsChunkMemberPointer<decltype(chunk)>
  )
  struct TraitEntry
  {
    static constexpr std::uint32_t magic = Utils::Meta::Traits::TypeOfMemberObject_T<decltype(chunk)>::magic;
    static constexpr bool _is_trait_entry = true;

    template<typename Self>
    static void Read(Self* self, ByteBuffer const& buf, std::size_t size)
    {
      if constexpr (ReadHandler::has_pre)
        ReadHandler::callback_pre(self, self->*chunk, buf, size);

      (self->*chunk).Read(buf, size);

      if constexpr (ReadHandler::has_post)
        ReadHandler::callback_post(self, self->*chunk, buf, size);
    }

    template<typename Self>
    static void Write(Self* self, ByteBuffer& buf)
    {
      if constexpr (WriteHandler::has_pre)
        WriteHandler::callback_pre(self, self->*chunk, buf);

      (self->*chunk).Write(buf);

      if constexpr (WriteHandler::has_post)
        WriteHandler::callback_post(self, self->*chunk, buf);
    }
  };

  template<typename T>
  concept IsTraitEntry = requires { { T::_is_trait_entry } -> std::same_as<bool>;  };

  template<typename... TraitEntries>
  requires (IsTraitEntry<TraitEntries> && ...)

  class AutoIOTrait
  {
    template<typename>
    friend class AutoIOTraitInterface;
    
  private:
    template<typename...>
    struct Pack {};

    template<typename Self, typename CurEntry, typename... Entries>
    static bool ReadChunkRecurse(Self* self
                                 , Common::ByteBuffer const& buf
                                 , ChunkHeader const& chunk_header
                                 , Pack<CurEntry, Entries...>)
    {
      if (CurEntry::magic == chunk_header.fourcc)
      {
        CurEntry::Read(self, buf, chunk_header.size);
        return true;
      }

      if constexpr (sizeof...(Entries))
      {
        if (ReadChunkRecurse(self, buf, chunk_header, Pack<Entries...>{}))
          return true;
      }

      return false;
    }

  protected:

    template<typename Self>
    static bool ReadChunk(Self* self, Common::ByteBuffer const& buf, ChunkHeader const& chunk_header)
    {
      ReadChunkRecurse(self, buf, chunk_header, Pack<TraitEntries...>{});
    };

    template<typename Self>
    static void WriteChunks(Self* self, Common::ByteBuffer& buf)
    {
      (TraitEntries::Write(self, buf), ...);
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


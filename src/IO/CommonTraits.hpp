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
   * Checks if provided type is a member pointer to a chunk object.
   * @tparam T Any type.
   */
  template<typename T>
  concept IsChunkMemberPointer = std::is_member_object_pointer_v<T>
    && Common::Concepts::ChunkProtocolCommon<Utils::Meta::Traits::TypeOfMemberObject_T<T>>;


  /**
   * An empty type representing default featureless read/write trait context.
   */
  struct DefaultTraitContext {};

  /**
   * Determines the type of IO::Common::Traits::IOHandler.
   */
  enum class IOHandlerType
  {
    Read, ///> Read handler.
    Write ///> Write handler.
  };

  /**
   * Determines type of IO::Common::Traits::AutoIOTraitInterface.
   */
  enum class TraitType
  {
    Component, ///> Trait is a component used to nesting into files or other component traits.
    File, ///> Trait is a file, which is a final component that should not be inherited from within traits system.
    Chunk ///> Trait is a chunk.
  };

  namespace details
  {
    /**
     * An empty struct to validate generic arguments. If it passes through, it is likely anything else will pass
     * through.
     */
    struct Any {};
  }

  /**
   * Checks whether provided type is an IO::Common::Traits::IOHandler read callback object.
   * @tparam T Any type.
   * @tparam pre True if callback is intended to be used for pre-read stage.
   */
  template<typename T, bool pre>
  concept IsIOHandlerCallbackRead = (pre ? std::is_invocable_r_v<bool, T, details::Any*, details::Any&
      , details::Any&, ByteBuffer const&, ChunkHeader const&>
    : std::is_invocable_v<T, details::Any*, details::Any&
      , details::Any&, ByteBuffer const&, ChunkHeader const&>);

  /**
   * Checks whether provided type is an IO::Common::Traits::IOHandler write callback object.
   * @tparam T Any type.
   * @tparam pre True if callback is intended to be used for pre-write stage.
   */
  template<typename T, bool pre>
  concept IsIOHandlerCallbackWrite = (pre ? std::is_invocable_r_v<bool, T, details::Any const*, details::Any&
      , details::Any&, ByteBuffer&>
    : std::is_invocable_v<T, details::Any const*, details::Any&
      , details::Any&, ByteBuffer&>);

  /**
   * Checks whether provided type is an IO::Common::Traits::IOHandler read callback object or std::nullptr_t.
   * @tparam T Any type.
   * @tparam pre True if callback is intended to be used for pre-read stage.
   */
  template<typename T, bool pre>
  concept IsIOHandlerCallbackReadOptional = IsIOHandlerCallbackRead<T, pre> || std::is_same_v<T, std::nullptr_t>;

  /**
   * Checks whether provided type is an IO::Common::Traits::IOHandler write callback object or std::nulptr_t.
   * @tparam T Any type.
   * @tparam pre True if callback is intended to be used for pre-write stage.
   */
  template<typename T, bool pre>
  concept IsIOHandlerCallbackWriteOptional = IsIOHandlerCallbackWrite<T, pre> || std::is_same_v<T, std::nullptr_t>;

  /**
   * Defines a set of pre and post optional read callbacks to be used with Common::Traits::IOTraits
   * or Common::Traits::AutoIOTrait entries.
   * @tparam pre Pre-read callback (off, if nullptr). If result is false, chunk or trait won't be read.
   * Must match signature:
   * bool(auto* self, auto& ctx, auto& chunk_or_trait, IO::Common::ByteBuffer const& buf, ChunkHeader const& chunk_header)
   * @tparam post Post-read callback (off, if nullptr). Invoked only when pre condition returned true,
   * and chunk or trait was read.
   * Must match signature:
   * void(auto* self, auto& ctx, auto& chunk_or_trait, IO::Common::ByteBuffer const& buf, ChunkHeader const& chunk_header)
   */
  template<auto pre = nullptr, auto post = nullptr>
  requires
  (
    IsIOHandlerCallbackReadOptional<decltype(pre), true> && IsIOHandlerCallbackReadOptional<decltype(post), false>
  )
  struct IOHandlerRead
  {
    static constexpr IOHandlerType handler_type = IOHandlerType::Read;
    static constexpr bool has_pre = !std::is_same_v<decltype(pre), std::nullptr_t>;
    static constexpr bool has_post = !std::is_same_v<decltype(post), std::nullptr_t>;
    static constexpr auto callback_pre = pre;
    static constexpr auto callback_post = post;
  };

  /**
   * Defines a set of pre and post optional write callbacks to be used with Common::Traits::IOTraits
   * or Common::Traits::AutoIOTrait entries.
   * @tparam pre Pre-read callback (off, if nullptr). If result is false, chunk or trait won't be written.
   * Must match signature:
   * bool(auto* self, auto& ctx, auto& chunk_or_trait, IO::Common::ByteBuffer& buf)
   * @tparam post Post-read callback (off, if nullptr). Invoked only when pre condition returned true
   * (and chunk/trait was read).
   * Must match signature:
   * void(auto* self, auto& ctx, auto& chunk_or_trait, IO::Common::ByteBuffer& buf)
   */
  template<auto pre = nullptr, auto post = nullptr>
  requires
  (
    IsIOHandlerCallbackWriteOptional<decltype(pre), true> && IsIOHandlerCallbackWriteOptional<decltype(post), false>
  )
  struct IOHandlerWrite
  {
    static constexpr IOHandlerType handler_type = IOHandlerType::Write;
    static constexpr bool has_pre = !std::is_same_v<decltype(pre), std::nullptr_t>;
    static constexpr bool has_post = !std::is_same_v<decltype(post), std::nullptr_t>;
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
    } && (T::handler_type == type);

  /**
   * Defines an entry to be passed into IO::Common::Traits::TraitEntries.
   * @tparam chunk Pointer to member object satisfying common requirements of a chunk.
   * @tparam ReadHandler IO::Common::Traits::IOHandlerRead or alike (read handler).
   * @tparam WriteHandler IO::Common::Traits::IOHanderWrite or alike (write handler).
   */
  template
  <
    auto chunk
    , IsIOHandler<IOHandlerType::Read> ReadHandler = IOHandlerRead<nullptr, nullptr>
    , IsIOHandler<IOHandlerType::Write> WriteHandler = IOHandlerWrite<nullptr, nullptr>
  >
  requires
  (
      IsChunkMemberPointer<decltype(chunk)>
  )
  struct TraitEntry
  {
    static constexpr std::uint32_t magic = Utils::Meta::Traits::TypeOfMemberObject_T<decltype(chunk)>::magic;
    static constexpr bool _is_trait_entry = true;

    template<typename Self, typename ReadContext>
    static bool Read(Self* self, ReadContext& read_ctx, ByteBuffer const& buf, ChunkHeader const& chunk_header)
    {
      if constexpr (ReadHandler::has_pre)
      {
        if (!ReadHandler::callback_pre(self, read_ctx, self->*chunk, buf, chunk_header))
          return false;
      }

      (self->*chunk).Read(read_ctx, buf, chunk_header.size);

      if constexpr (ReadHandler::has_post)
        ReadHandler::callback_post(self, read_ctx, self->*chunk, buf, chunk_header);

      return true;
    }

    template<typename Self, typename WriteContext>
    static void Write(Self* self, WriteContext& write_ctx, ByteBuffer& buf)
    {
      if constexpr (WriteHandler::has_pre)
      {
        if (!WriteHandler::callback_pre(self, write_ctx, self->*chunk, buf))
          return;
      }

      (self->*chunk).Write(write_ctx, buf);

      if constexpr (WriteHandler::has_post)
        WriteHandler::callback_post(self, write_ctx, self->*chunk, buf);
    }
  };

  /**
   * Checks if provided type is an instance of template IO::Common::Traits::TraitEntry.
   * @tparam T Any type.
   */
  template<typename T>
  concept IsTraitEntry = requires { { T::_is_trait_entry } -> std::same_as<const bool&>;  };

  /**
   * Pack like aggregate of IO::Common::Traits::TraitEntry. To be used with IO::Common::Traits::AutoIOTrait.
   * @tparam Entries Instances of template IO::Common::Traits::TraitEntry.
   */
  template<typename... Entries>
  requires (IsTraitEntry<Entries> && ...)
  struct TraitEntries {};

  /**
   * Defines an entry to be passed into IO::Common::Traits::IOTraits.
   * @tparam Trait Any type satisfying the named requirements of an IOTrait.
   * @tparam ReadHandler IO::Common::Traits::IOHandlerRead or alike (read handler).
   * @tparam WriteHandler  IO::Common::Traits::IOHandlerWrite or alike (write handler).
   */
  template
  <
    IOTraitOrEmpty Trait
    , IsIOHandler<IOHandlerType::Read> ReadHandler = IOHandlerRead<nullptr, nullptr>
    , IsIOHandler<IOHandlerType::Write> WriteHandler = IOHandlerWrite<nullptr, nullptr>
  >
  struct IOTrait
  {
    using TraitT = Trait;
    static constexpr bool _is_io_trait = true;

    template<typename Self, typename ReadContext>
    static void Read(Self* self, ReadContext& read_ctx, ByteBuffer const& buf, ChunkHeader const& chunk_header)
    {
      if constexpr (ReadHandler::has_pre)
      {
        if (!ReadHandler::callback_pre(self, read_ctx, *static_cast<Trait*>(self), buf, chunk_header))
          return;
      }

      static_cast<Trait*>(self)->Read(read_ctx, buf, chunk_header);

      if constexpr (ReadHandler::has_post)
        ReadHandler::callback_post(self, read_ctx, *static_cast<Trait*>(self), buf, chunk_header);
    }

    template<typename Self, typename WriteContext>
    static void Write(Self* self, WriteContext& write_ctx, ByteBuffer& buf)
    {
      if constexpr (WriteHandler::has_pre)
      {
        if (!WriteHandler::callback_pre(self, write_ctx, *static_cast<Trait*>(self), buf))
          return;
      }

      static_cast<Trait*>(self)->Write(write_ctx, buf);

      if constexpr (WriteHandler::has_post)
        WriteHandler::callback_post(self, write_ctx, *static_cast<Trait*>(self), buf);
    }
  };

  template<typename T>
  concept IsIOTrait = requires { { T::_is_io_trait } -> std::same_as<const bool&>;  };

  template<typename... Traits>
  requires (IsIOTrait<Traits> && ...)
  struct IOTraits {};


  /**
   * Adapter class accepting all traits of class, and providing a common interface to invoke them.
   * It is recommended to use traits through this adapter class, but not required.
   * @tparam Traits
   */
  template
  <
    typename Traits
    , std::default_initializable ReadContext = DefaultTraitContext
    , std::default_initializable WriteContext = DefaultTraitContext
  >
  struct AutoIOTraits;

  template
  <
    typename... Traits
    , std::default_initializable ReadContext
    , std::default_initializable WriteContext
  >
  struct AutoIOTraits<ReadContext, WriteContext, IOTraits<Traits...>> : public Traits::TraitT ...
  {
    template
    <
      typename CRTP
      , TraitType
      , std::default_initializable
      , std::default_initializable
    >
    friend class AutoIOTraitInterface;

  private:
    template<typename... Ts>
    struct TypePack {};

  // interface
  private:
    bool TraitsRead(ReadContext& ctx, Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
    {
      return RecurseRead(ctx, buf, chunk_header, TypePack<Traits...>());
    };

    void TraitsWrite(WriteContext& ctx, Common::ByteBuffer& buf) const
    {
      RecurseWrite(ctx, buf, TypePack<Traits...>());
    }

  // impl
  private:
    template<typename T, typename... Ts>
    void RecurseWrite(WriteContext& ctx, Common::ByteBuffer& buf, TypePack<T, Ts...>) const
    {
      // check if trait is enabled
      if constexpr (!std::is_empty_v<typename T::TraitT> && HasTraitEnabled<AutoIOTraits, typename T::TraitT>)
      {
        T::Write(this, ctx, buf);
      }

      if constexpr (sizeof...(Ts))
      {
        RecurseWrite(ctx, buf, TypePack<Ts...>());
      }
    }

    template<typename T, typename...Ts>
    [[nodiscard]]
    bool RecurseRead(ReadContext& ctx
                     , Common::ByteBuffer const& buf
                     , Common::ChunkHeader const& chunk_header
                     , TypePack<T, Ts...>)
    {
      // check if trait is enabled
      if constexpr (!std::is_empty_v<typename T::TraitT>
        && HasTraitEnabled<AutoIOTraits, typename T::TraitT>)
      {
        if (T::Read(this, ctx, buf))
          return true;
      }

      if constexpr (sizeof...(Ts))
      {
        if (RecurseRead(ctx, buf, chunk_header, TypePack<Ts...>()))
          return true;
      }

      return false;
    }
  };

  /**
   * Provides automatic Read/Write interfaces for classes using the traits system.
   * If the derived class also inherits from AutoIOTraits, traits defined there will be processed automatically.
   * If _auto_trait field is present in the derived class and is an instance of IO::Common::Traits::AutoIOTrait,
   * chunks defined there will be processed automatically.
   * @tparam CRTP Derived class ineheriting this class.
   * @tparam ReadContext Any default-constructible type that will be used as a shared read context.
   * @tparam WriteContext Any default-constructible type that will be used as a shared write context.
   * @tparam trait_type Type of trait. See IO::Common::Traits::TraitType.
   */
  template
  <
    typename CRTP
    , TraitType trait_type
    , std::default_initializable ReadContext = DefaultTraitContext
    , std::default_initializable WriteContext = DefaultTraitContext
  >
  class AutoIOTraitInterface
  {
  private:
    CRTP* GetThis() { return static_cast<CRTP*>(this); };
    CRTP const* GetThis() const { return static_cast<CRTP const*>(this); };

  public:

    [[nodiscard]]
    bool Read(ReadContext& read_ctx, Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
    requires (trait_type == TraitType::Component)
    {
      ValidateDependentInterfaces();

      return ReadCommon(read_ctx, buf, chunk_header);
    };

    void Read(Common::ByteBuffer const& buf)
    requires (trait_type == TraitType::File)
    {
      ReadContext read_ctx {};
      Read(read_ctx, buf);
    }

    void Read(ReadContext& read_ctx, Common::ByteBuffer const& buf)
    requires (trait_type == TraitType::File)
    {
      ValidateDependentInterfaces();
      LogDebugF(LCodeZones::FILE_IO, "Reading %s file...", NAMEOF_TYPE(CRTP));
      LogIndentScoped;

      RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
      RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");

      while (!buf.IsEof())
      {
        auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

        if (ReadCommon(read_ctx, buf, chunk_header))
          continue;

        buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
        LogError("Encountered unknown or unhandled chunk %s.", Common::FourCCToStr(chunk_header.fourcc));
      }

      EnsureF(CCodeZones::FILE_IO, buf.IsEof(), "Not all chunks have been parsed in the file. "
                                                "Bad logic or corrupt file.");

    }

    void Read(ReadContext& read_ctx, Common::ByteBuffer const& buf, std::size_t size)
    requires (trait_type == TraitType::Chunk)
    {
      ValidateDependentInterfaces();

      LogDebugF(LCodeZones::FILE_IO, "Reading chunk: %s, size: %d."
                , FourCCStr<CRTP::magic, CRTP::magic_endian>
                , size);
      LogIndentScoped;

      std::size_t end_pos = buf.Tell() + size;

      while(buf.Tell() != end_pos)
      {
        EnsureF(CCodeZones::FILE_IO, buf.Tell() < end_pos, "Disproportional read attempt. Read past expected end.");

        auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

        if (ReadCommon(read_ctx, buf, chunk_header))
          continue;

        buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
        LogError("Encountered unknown or unhandled chunk %s.", Common::FourCCToStr(chunk_header.fourcc));
      }
    }

    void Write(Common::ByteBuffer& buf) const
    requires (trait_type == TraitType::File)
    {
      WriteContext write_ctx {};

      Write(write_ctx, buf);
    };

    void Write(WriteContext& write_ctx, Common::ByteBuffer& buf) const
    requires (trait_type == TraitType::File)
    {
      ValidateDependentInterfaces();
      RequireF(CCodeZones::FILE_IO, buf.IsDataOnwed(), "Attempt to write into read-only buffer.");

      LogDebugF(LCodeZones::FILE_IO, "Writing %s file...", NAMEOF_TYPE(CRTP));
      LogIndentScoped;

      WriteCommon(write_ctx, buf);
    }

    void Write(WriteContext& write_ctx, Common::ByteBuffer& buf) const
    requires (trait_type == TraitType::Component)
    {
      ValidateDependentInterfaces();
      RequireF(CCodeZones::FILE_IO, buf.IsDataOnwed(), "Attempt to write into read-only buffer.");

      WriteCommon(write_ctx, buf);
    };

    void Write(WriteContext& write_ctx, Common::ByteBuffer& buf) const
    requires (trait_type == TraitType::Chunk)
    {
      if (!GetThis()->_is_initialized) [[unlikely]]
        return;

      ValidateDependentInterfaces();
      RequireF(CCodeZones::FILE_IO, buf.IsDataOnwed(), "Attempt to write into read-only buffer.");

      LogDebugF(LCodeZones::FILE_IO, "Writing chunk: %s."
                , FourCCStr<CRTP::fourcc, CRTP::fourcc_endian>);

      std::size_t pos = buf.Tell();

      Common::ChunkHeader chunk_header {CRTP::fourcc, 0};
      buf.Write(chunk_header);

      WriteCommon(write_ctx, buf);

      std::size_t end_pos = buf.Tell();
      buf.Seek(pos);

      EnsureF(CCodeZones::FILE_IO, (end_pos - pos - sizeof(Common::ChunkHeader)) <= std::numeric_limits<std::uint32_t>::max()
              , "Chunk size overflow.");

      chunk_header.size = static_cast<std::uint32_t>(end_pos - pos - sizeof(Common::ChunkHeader));
      buf.Write(chunk_header);
      buf.Seek(end_pos);
    }

  private:
    void WriteCommon(WriteContext& write_ctx, Common::ByteBuffer& buf) const
    {
      // invoke optional method to handle unlisted chunks that cannot be processed automatically
      if constexpr (requires (CRTP crtp){ { crtp.WriteExtraPre(write_ctx, buf) } -> std::same_as<void>; })
      {
        GetThis()->WriteExtraPre(write_ctx, buf);
      }

      // Use auto-trait if present in type
      if constexpr (requires { { &CRTP::_auto_trait }; })
      {
        decltype(CRTP::_auto_trait)::template WriteChunks(GetThis(), write_ctx, buf);
      }

      // check if derived class also inherits from traits
      if constexpr (requires { { &CRTP::TraitsWrite }; })
      {
        GetThis()->TraitsWrite(write_ctx, buf);
      }

      // invoke optional method to handle unlisted chunks that cannot be processed automatically
      if constexpr (requires (CRTP crtp){ { crtp.WriteExtraPost(buf) } -> std::same_as<void>; })
      {
        GetThis()->WriteExtraPost(write_ctx, buf);
      }
    }

    bool ReadCommon(ReadContext& read_ctx, Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
    {
      // invoke optional method to handle unlisted chunks that cannot be processed automatically
      if constexpr (requires (CRTP crtp){ { crtp.ReadExtraPre(read_ctx, buf, chunk_header) } -> std::same_as<bool>; })
      {
        if (GetThis()->ReadExtraPre(read_ctx, buf, chunk_header))
          return true;
      }

      // Use auto-trait if present in type
      if constexpr (requires { { &CRTP::_auto_trait }; })
      {
        if (decltype(CRTP::_auto_trait)::template ReadChunk(GetThis(), read_ctx, buf, chunk_header))
          return true;
      }

      // check if derived class also inherits from traits
      if constexpr (requires { { &CRTP::TraitsRead }; })
      {
        if(GetThis()->TraitsRead(read_ctx, buf, chunk_header))
          return true;
      }

      // invoke optional method to handle unlisted chunks that cannot be processed automatically
      if constexpr (requires (CRTP crtp){ { crtp.ReadExtraPost(read_ctx, buf, chunk_header) } -> std::same_as<bool>; })
      {
        if (GetThis()->ReadExtraPost(read_ctx, buf, chunk_header))
          return true;
      }

      return false;
    }

    constexpr void ValidateDependentInterfaces() const
    {
      // AutoIOTrait
      if constexpr (requires { { &CRTP::_auto_trait }; })
      {
        static_assert(std::is_same_v<typename decltype(CRTP::_auto_trait)::ReadContextT, ReadContext>
                      && "Context type mismatch.");
        static_assert(std::is_same_v<typename decltype(CRTP::_auto_trait)::WriteContextT, WriteContext>
                      && "Context type mismatch.");
      }

      // AutoIOTraits::TraitsRead
      if constexpr (requires { { &CRTP::TraitsRead }; })
      {
        // validate signature
        static_assert(requires { { static_cast<bool(CRTP::*)(ReadContext&, Common::ByteBuffer const&
                                                             , Common::ChunkHeader const&)>(&CRTP::TraitsRead) }; }
                      && "Invalid TraitsRead signature. Possibly context type mismatch.");
      }

      // AutoIOTraits::TraitsWrite
      if constexpr (requires { { &CRTP::TraitsRead }; })
      {
        // validate signature
        static_assert(requires { { static_cast<void(CRTP::*)(WriteContext&, Common::ByteBuffer&)>(&CRTP::TraitsWrite) }; }
                      && "Invalid TraitsWrite. Possibly context type mismatch.");

      }

      if constexpr (trait_type == TraitType::Chunk)
      {
        static_assert(requires (CRTP crtp)
                      {
                        { CRTP::magic } -> std::same_as<std::uint32_t const&>;
                        { CRTP::magic_endian } -> std::same_as<FourCCEndian const&>;
                        { crtp._is_initialized } -> std::same_as<bool>;
                     }, "Missing FourCC and endian for type.");
      }
    }
  };

  /**
   * A descriptor class that enables automatic handling of chunk members in a class, providing Read/Write functionality.
   * Must be used a membmer variable named "_auto_trait". Accessed by IO::Common::Traits::AutoIOTraitInteface.
   * @tparam EntryPack Instance of template IO::Common::Traits::TraitEntries.
   * @tparam ReadContext Any default-constructible type that will be used as a shared read context.
   * @tparam WriteContext Any default-constructible type that will be used as a shared write context.
   */
  template
  <
    typename EntryPack,
    std::default_initializable ReadContext = DefaultTraitContext
    , std::default_initializable WriteContext = DefaultTraitContext
  >
  class AutoIOTrait;

  template<typename... Entries, std::default_initializable ReadContext, std::default_initializable WriteContext>
  class AutoIOTrait<TraitEntries<Entries...>, ReadContext, WriteContext>
  {
    template
    <
      typename CRTP
      , TraitType
      , std::default_initializable
      , std::default_initializable
    >
    friend class AutoIOTraitInterface;

  public:
    using ReadContextT = ReadContext;
    using WriteContextT = WriteContext;

  // impl
  private:
    template<typename...>
    struct Pack {};

    template<typename Self, typename CurEntry, typename... TEntries>
    static bool ReadChunkRecurse(Self* self
                                 , ReadContext& read_ctx
                                 , Common::ByteBuffer const& buf
                                 , ChunkHeader const& chunk_header
                                 , Pack<CurEntry, TEntries...>)
    {
      if (CurEntry::magic == chunk_header.fourcc)
      {
        CurEntry::Read(self, read_ctx, buf, chunk_header);
        return true;
      }

      if constexpr (sizeof...(TEntries))
      {
        if (ReadChunkRecurse(self, read_ctx, buf, chunk_header, Pack<TEntries...>{}))
          return true;
      }

      return false;
    }

  // interface
  private:

    template<typename Self>
    static bool ReadChunk(Self* self, ReadContext& read_ctx, Common::ByteBuffer const& buf, ChunkHeader const& chunk_header)
    {
      return ReadChunkRecurse(self, read_ctx, buf, chunk_header, Pack<Entries...>{});
    };

    template<typename Self>
    static void WriteChunks(Self* self, WriteContext& write_ctx, Common::ByteBuffer& buf)
    {
      (Entries::Write(self, write_ctx, buf), ...);
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


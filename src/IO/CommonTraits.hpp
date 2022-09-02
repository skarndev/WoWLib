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
      struct Any {};
      static constexpr bool Test()
      {
        return requires
          {
            { static_cast<bool(T::*)(Any&, ByteBuffer const&, ChunkHeader const&)>(
              &T::AutoIOTraitInterface_T::template ReadTrait<Any>) };

            { static_cast<void(T::*)(Any&, ByteBuffer&) const>(
              &T::AutoIOTraitInterface_T::template WriteTrait<Any>) };
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
    template<auto V1, auto V2, typename T>
    struct ValuesEqualT : std::bool_constant<V1 == V2>
    {
      using type = T;
    };
  }

  /**
   * Checks if type satisfies the named requirements to be used as any of the IO traits.
   * @tparam T Any type.
   */
  template<typename T>
  concept IsIOTraitImpl = details::ValidateTrait<T>::result;

  /**
   * Provides a type for the optional trait of a chunk / class, present based on the client version.
   * @tparam T Any type that satisfies named requirements of a trait (see associated concept).
   * @tparam current_version Current version.
   * @tparam version_min Minimum version of the client this trait is supported for.
   * @tparam version_max Maximum version of the client this trait is supported for.
   */
  template
  <
    IsIOTraitImpl T
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
  template<auto val, IsIOTraitImpl T, bool do_compute = true>
  struct TraitCase
  {
    static constexpr auto value = val;
    static constexpr bool use = do_compute;
    using type = T;
  };

  /**
   * Represents one of the trait options to be used with IO::Common::Traits::SwitchableTrait
   * @tparam val Trait identifier to match against, e.g. an enum.
   * @tparam T Type of trait implementation.
   * @tparam client_version Current version of the client.
   * @tparam version_min Minimal version of the client this trait is supported in.
   * @tparam version_max Maximum version of the client this trait is supported in.
   */
  template
  <
    auto val
    , IsIOTraitImpl T
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
  concept HasTraitEnabled = IsIOTraitImpl<Trait> && std::is_base_of_v<Trait, T>;

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
  concept HasVersionTrait = IsIOTraitImpl<Trait> && (std::is_base_of_v<Trait, T>
                                                     || std::is_base_of_v<details::EmptyTrait<Trait>, T>);

  /**
   * Checks if passed type is either an enabled trait or an empty.
   * @tparam T Any type.
   * @tparam Trait Trait type.
   */
  template<typename T>
  concept IOTraitOrEmpty = std::is_empty_v<T> || IsIOTraitImpl<T>;

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
    static constexpr bool has_pre = !std::is_same_v<decltype(pre), std::nullptr_t>;
    static constexpr bool has_post = !std::is_same_v<decltype(post), std::nullptr_t>;
    static constexpr auto callback_pre = pre;
    static constexpr auto callback_post = post;
  };

  namespace details
  {
    template<template<auto, auto> typename Handler, typename T>
    struct IsIOHanderImpl : std::false_type {};

    template<auto pre, auto post>
    struct IsIOHanderImpl<IOHandlerRead, IOHandlerRead<pre, post>> : std::true_type {};

    template<auto pre, auto post>
    struct IsIOHanderImpl<IOHandlerWrite, IOHandlerWrite<pre, post>> : std::true_type {};
  }

  /**
   * Checks whether a given type is IOHandler.
   * @tparam T Any type.
   */
  template<typename T, template<auto, auto> typename Handler>
  concept IsIOHandler = details::IsIOHanderImpl<Handler, T>::value;
  /**
   * Defines an entry to be passed into IO::Common::Traits::TraitEntries.
   * @tparam chunk Pointer to member object satisfying common requirements of a chunk.
   * @tparam ReadHandler IO::Common::Traits::IOHandlerRead or alike (read handler).
   * @tparam WriteHandler IO::Common::Traits::IOHanderWrite or alike (write handler).
   */
  template
  <
    auto chunk
    , IsIOHandler<IOHandlerRead> ReadHandler = IOHandlerRead<nullptr, nullptr>
    , IsIOHandler<IOHandlerWrite> WriteHandler = IOHandlerWrite<nullptr, nullptr>
  >
  requires
  (
      IsChunkMemberPointer<decltype(chunk)>
  )
  struct TraitEntry
  {
    static constexpr std::uint32_t magic = Utils::Meta::Traits::TypeOfMemberObject_T<decltype(chunk)>::magic;
    static constexpr std::string_view field_name = NAMEOF_MEMBER(chunk);

    template<typename Self, typename ReadContext>
    static bool Read(Self* self, ReadContext& read_ctx, ByteBuffer const& buf, ChunkHeader const& chunk_header)
    {
      LogDebugF(LCodeZones::FILE_IO, "Reading field: %s:", field_name.data());
      LogDebugF(LCodeZones::FILE_IO, "{");

      {
        LogIndentScoped;

        if constexpr (ReadHandler::has_pre)
        {
          if (!ReadHandler::callback_pre(self, read_ctx, self->*chunk, buf, chunk_header))
            return false;
        }

        (self->*chunk).Read(read_ctx, buf, chunk_header.size);

        if constexpr (ReadHandler::has_post)
          ReadHandler::callback_post(self, read_ctx, self->*chunk, buf, chunk_header);

      }

      LogDebugF(LCodeZones::FILE_IO, "}");
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

  namespace details
  {
    template<typename T>
    struct IsTraitEntryImpl : std::false_type {};

    template<auto chunk, typename...Ts>
    struct IsTraitEntryImpl<TraitEntry<chunk, Ts...>> : std::true_type {};
  }

  /**
   * Checks if provided type is an instance of template IO::Common::Traits::TraitEntry.
   * @tparam T Any type.
   */
  template<typename T>
  concept IsTraitEntry = details::IsTraitEntryImpl<T>::value;

  /**
   * Defines an entry to be passed into IO::Common::Traits::IOTraits.
   * @tparam Trait Any type satisfying the named requirements of an IOTrait.
   * @tparam ReadHandler IO::Common::Traits::IOHandlerRead or alike (read handler).
   * @tparam WriteHandler  IO::Common::Traits::IOHandlerWrite or alike (write handler).
   */
  template
  <
    IOTraitOrEmpty Trait
    , IsIOHandler<IOHandlerRead> ReadHandler = IOHandlerRead<nullptr, nullptr>
    , IsIOHandler<IOHandlerWrite> WriteHandler = IOHandlerWrite<nullptr, nullptr>
  >
  struct IOTrait
  {
    using TraitT = Trait;

    template<typename Self, typename ReadContext>
    static bool Read(Self* self, ReadContext& read_ctx, ByteBuffer const& buf, ChunkHeader const& chunk_header)
    {
      if constexpr (ReadHandler::has_pre)
      {
        if (!ReadHandler::callback_pre(self, read_ctx, *static_cast<Trait*>(self), buf, chunk_header))
          return false;
      }

      if (!static_cast<Trait*>(self)->ReadTrait(read_ctx, buf, chunk_header))
        return false;

      if constexpr (ReadHandler::has_post)
        ReadHandler::callback_post(self, read_ctx, *static_cast<Trait*>(self), buf, chunk_header);

      return true;
    }

    template<typename Self, typename WriteContext>
    static void Write(Self* self, WriteContext& write_ctx, ByteBuffer& buf)
    {
      if constexpr (WriteHandler::has_pre)
      {
        if (!WriteHandler::callback_pre(self, write_ctx, *static_cast<Trait*>(self), buf))
          return;
      }

      static_cast<const Trait*>(self)->WriteTrait(write_ctx, buf);

      if constexpr (WriteHandler::has_post)
        WriteHandler::callback_post(self, write_ctx, *static_cast<Trait*>(self), buf);
    }
  };

  namespace details
  {
    template<typename T>
    struct IsIOTraitImpl : std::false_type {};

    template<typename... Ts>
    struct IsIOTraitImpl<IOTrait<Ts...>> : std::true_type {};
  }

  /**
   * Checks if type is an instance of template IO::Common::Traits::IOTrait.
   * @tparam T Any type.
   */
  template<typename T>
  concept IsIOTrait = details::IsIOTraitImpl<T>::value;


  /**
   * Adapter class accepting all traits of class, and providing a common interface to invoke them.
   * It is recommended to use traits through this adapter class, but not required.
   * @tparam Traits List of inherited traits. Instance of template IO::Common::Traits::IOTrait.
   */
  template<typename... Traits>
  struct AutoIOTraits : public Traits::TraitT ...
  {
    template<typename CRTP, TraitType>
    friend class AutoIOTraitInterface;

  private:
    template<typename... Ts>
    struct TypePack {};

  // interface
  protected:
    template<typename ReadContext>
    bool TraitsRead(ReadContext& ctx, Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
    {
      return RecurseRead(ctx, buf, chunk_header, TypePack<Traits...>());
    };

    template<typename WriteContext>
    void TraitsWrite(WriteContext& ctx, Common::ByteBuffer& buf) const
    {
      RecurseWrite(ctx, buf, TypePack<Traits...>());
    }

  // impl
  private:
    template<typename T, typename WriteContext, typename... Ts>
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

    template<typename T, typename ReadContext, typename...Ts>
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
        if (T::Read(this, ctx, buf, chunk_header))
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

  namespace details
  {
    template<typename CRTP>
    class AutoIOTraitInterfaceFileImpl
    {
    private:
      CRTP* GetThis() { return static_cast<CRTP*>(this); };
      CRTP const* GetThis() const { return static_cast<CRTP const*>(this); };

    public:
      template<std::default_initializable ReadContext = DefaultTraitContext>
      void Read(Common::ByteBuffer const& buf)
      {
        ReadContext read_ctx {};
        Read(read_ctx, buf);
      }

      template<typename ReadContext>
      void Read(ReadContext& read_ctx, Common::ByteBuffer const& buf)
      {
        GetThis()->ValidateDependentInterfaces();
        LogDebugF(LCodeZones::FILE_IO, "Reading %s file:", NAMEOF_SHORT_TYPE(typename CRTP::Derived));
        LogDebugF(LCodeZones::FILE_IO, "{");

        {
          LogIndentScoped;
          RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
          RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");

          while (!buf.IsEof())
          {
            auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

            if (GetThis()->ReadCommon(read_ctx, buf, chunk_header))
              continue;

            buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
            LogError("Encountered unknown or unhandled chunk %s.", Common::FourCCToStr(chunk_header.fourcc));
          }

          EnsureF(CCodeZones::FILE_IO, buf.IsEof(), "Not all chunks have been parsed in the file. "
                                                    "Bad logic or corrupt file.");
        }

        LogDebugF(LCodeZones::FILE_IO, "}");

      }

      template<std::default_initializable WriteContext = DefaultTraitContext>
      void Write(Common::ByteBuffer& buf) const
      {
        WriteContext write_ctx {};

        Write(write_ctx, buf);
      };

      template<typename WriteContext>
      void Write(WriteContext& write_ctx, Common::ByteBuffer& buf) const
      {
        GetThis()->ValidateDependentInterfaces();
        RequireF(CCodeZones::FILE_IO, buf.IsDataOnwed(), "Attempt to write into read-only buffer.");

        LogDebugF(LCodeZones::FILE_IO, "Writing %s file...", NAMEOF_SHORT_TYPE(typename CRTP::Derived));
        LogIndentScoped;

        GetThis()->WriteCommon(write_ctx, buf);
      }
    };

    template<typename CRTP>
    class AutoIOTraitInterfaceComponentImpl
    {
    private:
      CRTP* GetThis() { return static_cast<CRTP*>(this); };
      CRTP const* GetThis() const { return static_cast<CRTP const*>(this); };

    public:
      template<typename ReadContext>
      [[nodiscard]]
      bool ReadTrait(ReadContext& read_ctx, Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
      {
        GetThis()->ValidateDependentInterfaces();

        return GetThis()->ReadCommon(read_ctx, buf, chunk_header);
      };

      template<typename WriteContext>
      void WriteTrait(WriteContext& write_ctx, Common::ByteBuffer& buf) const
      {
        GetThis()->ValidateDependentInterfaces();
        RequireF(CCodeZones::FILE_IO, buf.IsDataOnwed(), "Attempt to write into read-only buffer.");

        GetThis()->WriteCommon(write_ctx, buf);
      };
    };


    template<typename CRTP>
    class AutoIOTraitInterfaceChunkImpl
    {
    private:
      CRTP* GetThis() { return static_cast<CRTP*>(this); };
      CRTP const* GetThis() const { return static_cast<CRTP const*>(this); };

    public:
      template<typename ReadContext>
      void Read(ReadContext& read_ctx, Common::ByteBuffer const& buf, std::size_t size)
      {
        GetThis()->ValidateDependentInterfaces();

        LogDebugF(LCodeZones::FILE_IO, "Reading chunk: %s, size: %d."
                  , FourCCStr<CRTP::Derived::magic, CRTP::Derived::magic_endian>
                  , size);
        LogIndentScoped;

        std::size_t end_pos = buf.Tell() + size;

        while(buf.Tell() != end_pos)
        {
          EnsureF(CCodeZones::FILE_IO, buf.Tell() < end_pos, "Disproportional read attempt. Read past expected end.");

          auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

          if (GetThis()->ReadCommon(read_ctx, buf, chunk_header))
            continue;

          buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
          LogError("Encountered unknown or unhandled chunk %s.", Common::FourCCToStr(chunk_header.fourcc));
        }

        GetThis()->SetChunkInitialized();
      }

      template<typename WriteContext>
      void Write(WriteContext& write_ctx, Common::ByteBuffer& buf) const
      {
        if (!GetThis()->IsChunkInitialized()) [[unlikely]]
          return;

        GetThis()->ValidateDependentInterfaces();
        RequireF(CCodeZones::FILE_IO, buf.IsDataOnwed(), "Attempt to write into read-only buffer.");

        LogDebugF(LCodeZones::FILE_IO, "Writing chunk: %s."
                  , FourCCStr<CRTP::Derived::magic, CRTP::Derived::magic_endian>);
        LogIndentScoped;

        std::size_t pos = buf.Tell();

        Common::ChunkHeader chunk_header {CRTP::Derived::magic, 0};
        buf.Write(chunk_header);

        GetThis()->WriteCommon(write_ctx, buf);

        std::size_t end_pos = buf.Tell();
        buf.Seek(pos);

        EnsureF(CCodeZones::FILE_IO, (end_pos - pos - sizeof(Common::ChunkHeader)) <= std::numeric_limits<std::uint32_t>::max()
                , "Chunk size overflow.");

        chunk_header.size = static_cast<std::uint32_t>(end_pos - pos - sizeof(Common::ChunkHeader));
        buf.Write(chunk_header);
        buf.Seek(end_pos);
      }

    };

    struct AutoIOTraitInterfaceEmptyImpl{};
  }

  /**
   * Provides automatic Read/Write interfaces for classes using the traits system.
   * If the derived class also inherits from AutoIOTraits, traits defined there will be processed automatically.
   * If _auto_trait field is present in the derived class and is an instance of IO::Common::Traits::AutoIOTrait,
   * chunks defined there will be processed automatically.
   * @tparam CRTP Derived class ineheriting this class.
   * @tparam trait_type Type of trait. See IO::Common::Traits::TraitType.
   */
  template<typename CRTP, TraitType trait_type>
  class AutoIOTraitInterface : public std::disjunction
                                      <
                                        details::ValuesEqualT
                                        <
                                          trait_type, TraitType::File
                                          , details::AutoIOTraitInterfaceFileImpl
                                            <
                                              AutoIOTraitInterface<CRTP, trait_type>
                                            >
                                        >
                                        , details::ValuesEqualT
                                          <
                                            trait_type, TraitType::Chunk
                                            , details::AutoIOTraitInterfaceChunkImpl
                                              <
                                                AutoIOTraitInterface<CRTP, trait_type>
                                              >
                                          >
                                        , details::ValuesEqualT
                                          <
                                            true, true
                                            , details::AutoIOTraitInterfaceEmptyImpl
                                          >
                                      >::type
                              , protected std::disjunction
                                          <
                                            details::ValuesEqualT
                                            <
                                              trait_type, TraitType::Component
                                              , details::AutoIOTraitInterfaceComponentImpl
                                                <
                                                  AutoIOTraitInterface<CRTP, trait_type>
                                                >
                                            >
                                             , details::ValuesEqualT
                                               <
                                                 true, true
                                                 , details::AutoIOTraitInterfaceEmptyImpl
                                                >
                                          >::type
  {
    template<typename>
    friend struct details::ValidateTrait;

    template<typename>
    friend class details::AutoIOTraitInterfaceFileImpl;

    template<typename>
    friend class details::AutoIOTraitInterfaceComponentImpl;

    template<typename>
    friend class details::AutoIOTraitInterfaceChunkImpl;

    template<IOTraitOrEmpty, IsIOHandler<IOHandlerRead>, IsIOHandler<IOHandlerWrite>>
    friend struct IOTrait;

    using Derived = CRTP;

  public:
    using AutoIOTraitInterface_T = AutoIOTraitInterface;

  private:
    CRTP* GetThis() { return static_cast<CRTP*>(this); };
    CRTP const* GetThis() const { return static_cast<CRTP const*>(this); };

    [[nodiscard]]
    bool IsChunkInitialized() const
    requires (trait_type == TraitType::Chunk)
    {
      return GetThis()->_is_initialized;
    }

    void SetChunkInitialized()
    requires (trait_type == TraitType::Chunk)
    {
      GetThis()->_is_initialized = true;
    }

  private:
    template<typename WriteContext>
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
      if constexpr (requires { { &CRTP::template TraitsWrite<WriteContext> }; })
      {
        GetThis()->TraitsWrite(write_ctx, buf);
      }

      // invoke optional method to handle unlisted chunks that cannot be processed automatically
      if constexpr (requires (CRTP crtp){ { crtp.WriteExtraPost(write_ctx, buf) } -> std::same_as<void>; })
      {
        GetThis()->WriteExtraPost(write_ctx, buf);
      }
    }

    template<typename ReadContext>
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
      if constexpr (requires { { &CRTP::template TraitsRead<ReadContext> }; })
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
//      // AutoIOTrait
//      if constexpr (requires { { &CRTP::_auto_trait }; })
//      {
//        static_assert(std::is_same_v<typename decltype(CRTP::_auto_trait)::ReadContextT, ReadContext>
//                      && "Context type mismatch.");
//        static_assert(std::is_same_v<typename decltype(CRTP::_auto_trait)::WriteContextT, WriteContext>
//                      && "Context type mismatch.");
//      }
//
//      // AutoIOTraits::TraitsRead
//      if constexpr (requires { { &CRTP::TraitsRead }; })
//      {
//        // validate signature
//        static_assert(requires { { static_cast<bool(CRTP::*)(ReadContext&, Common::ByteBuffer const&
//                                                             , Common::ChunkHeader const&)>(&CRTP::TraitsRead) }; }
//                      && "Invalid TraitsRead signature. Possibly context type mismatch.");
//      }
//
//      // AutoIOTraits::TraitsWrite
//      if constexpr (requires { { &CRTP::TraitsRead }; })
//      {
//        // validate signature
//        static_assert(requires { { static_cast<void(CRTP::*)(WriteContext&, Common::ByteBuffer&)>(&CRTP::TraitsWrite) }; }
//                      && "Invalid TraitsWrite. Possibly context type mismatch.");
//
//      }

      if constexpr (trait_type == TraitType::Chunk)
      {
        static_assert(requires (CRTP crtp)
                      {
                        { CRTP::magic } -> std::same_as<std::uint32_t const&>;
                        { CRTP::magic_endian } -> std::same_as<FourCCEndian const&>;
                        { crtp._is_initialized } -> std::same_as<bool&>;
                     } && "Missing FourCC and endian for type.");
      }
    }
  };

  /**
   * A descriptor class that enables automatic handling of chunk members in a class, providing Read/Write functionality.
   * Must be used a membmer variable named "_auto_trait". Accessed by IO::Common::Traits::AutoIOTraitInteface.
   * @tparam Entries Chunks of this class. Instances of template IO::Common::Traits::TraitEntry
   */
  template<IsTraitEntry... Entries>
  class AutoIOTrait
  {
    template<typename CRTP, TraitType>
    friend class AutoIOTraitInterface;

  // impl
  private:
    template<typename...>
    struct Pack {};

    template<typename Self, typename CurEntry, typename ReadContext, typename... TEntries>
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

      if constexpr (sizeof...(TEntries) > 0)
      {
        if (ReadChunkRecurse(self, read_ctx, buf, chunk_header, Pack<TEntries...>{}))
          return true;
      }

      return false;
    }

  // interface
  private:

    template<typename Self, typename ReadContext>
    static bool ReadChunk(Self* self, ReadContext& read_ctx, Common::ByteBuffer const& buf, ChunkHeader const& chunk_header)
    {
      return ReadChunkRecurse(self, read_ctx, buf, chunk_header, Pack<Entries...>{});
    };

    template<typename Self, typename WriteContext>
    static void WriteChunks(Self* self, WriteContext& write_ctx, Common::ByteBuffer& buf)
    {
      (Entries::Write(self, write_ctx, buf), ...);
    }

  };
}


/**
 * This macro must be used in class declarations inheriting from IO::Common::Traits::AutoIOTraitInterface to enable
 * private method access.
 */
#define AutoIOTraitInterfaceUser \
template<typename, IO::Common::Traits::TraitType> \
friend class IO::Common::Traits::AutoIOTraitInterface

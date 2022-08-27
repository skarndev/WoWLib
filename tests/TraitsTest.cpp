#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

#include <cstdint>

using namespace IO::Common;
using namespace IO::Common::Traits;
using namespace IO::ADT;

struct TestVersionTrait : public AutoIOTraitInterface<TestVersionTrait, TraitType::Component>
{
  AutoIOTraitInterfaceUser;
private:
  DataChunk<std::uint32_t, ChunkIdentifiers::ADTRootChunks::MFBO> _trait_header;

public:
  [[nodiscard]]
  auto& GetTraitHeader()  const { return _trait_header; };

private:
  static constexpr
  AutoIOTrait
    <
      TraitEntry<&TestVersionTrait::_trait_header>
    > _auto_trait {};

};

struct TestComplexChunk : public ChunkCommon<ChunkIdentifiers::ADTRootChunks::MCNK>
                        , public AutoIOTraitInterface<TestComplexChunk, TraitType::Chunk>
{
  AutoIOTraitInterfaceUser;

private:
  DataChunk<std::uint32_t, ChunkIdentifiers::ADTRootChunks::MHDR> _header;

public:
  [[nodiscard]]
  auto& GetHeader() const { return _header; };

private:
  static constexpr
  AutoIOTrait
  <
    TraitEntry<&TestComplexChunk::_header>
  > _auto_trait {};
};


template<ClientVersion client_version>
struct TestFile : public AutoIOTraitInterface<TestFile<client_version>, TraitType::File>
                , public AutoIOTraits
                <
                  IOTrait
                  <
                    VersionTrait<TestVersionTrait, client_version, ClientVersion::SL>
                  >
                >
{
  AutoIOTraitInterfaceUser;

public:
  [[nodiscard]]
  auto& GetHeader() const { return _header; };

  [[nodiscard]]
  auto& GetComplexChunk() const { return _complex_chunk;};

private:
  DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> _header;
  TestComplexChunk _complex_chunk;

  static constexpr
  AutoIOTrait
  <
    TraitEntry<&TestFile::_header>
    , TraitEntry
    <
      &TestFile::_complex_chunk
      , IOHandlerRead
        <
          [](auto const* self, auto& ctx, auto& chunk, ByteBuffer const& buf, ChunkHeader const& chunk_header) -> bool
          {
            LogDebug("Printing from callback pre on Read, fourcc: %s", FourCCToStr(chunk_header.fourcc));
            return true; // return true to continue reading
          }
          , [](auto const* self, auto& ctx, auto& chunk, ByteBuffer const& buf, ChunkHeader const& chunk_header)
          {
            LogDebug("Printing from callback post on Read, fourcc: %s", FourCCToStr(chunk_header.fourcc));
          }
        >
    >
  > _auto_trait {};
};

template<bool with_trait>
void PrepareFile(ByteBuffer& buf)
{
  // header
  buf.Write(ChunkIdentifiers::ADTCommonChunks::MVER);
  buf.Write(static_cast<std::uint32_t>(sizeof(std::uint32_t)));
  buf.Write(static_cast<std::uint32_t>(0));

  // inlined complex chunk
  buf.Write(ChunkIdentifiers::ADTRootChunks::MCNK);
  buf.Write(static_cast<std::uint32_t>(sizeof(std::uint32_t) + sizeof(ChunkHeader)));
  buf.Write(ChunkIdentifiers::ADTRootChunks::MHDR);
  buf.Write(static_cast<std::uint32_t>(sizeof(std::uint32_t)));
  buf.Write(static_cast<std::uint32_t>(1));

  if constexpr (with_trait)
  {
    buf.Write(ChunkIdentifiers::ADTRootChunks::MFBO);
    buf.Write(static_cast<std::uint32_t>(sizeof(std::uint32_t)));
    buf.Write(static_cast<std::uint32_t>(2));
  }
  buf.Seek(0);
}

int main()
{
  ByteBuffer bb {};
  ByteBuffer bb1 {};

  // prepare files
  PrepareFile<false>(bb);
  PrepareFile<true>(bb1);

  // test files
  TestFile<ClientVersion::LEGION> t;
  t.Read(bb);
  TestFile<ClientVersion::SL> t1;
  t1.Read(bb1);


  ByteBuffer w_bb {};
  ByteBuffer w_bb1 {};
  t.Write(w_bb);
  t1.Write(w_bb1);

  Ensure(bb == w_bb, "Read and Write do not match");
  Ensure(bb1 == w_bb1, "Read and Write do not match");

  LogDebug("First: %d", t.GetHeader().data);
  LogDebug("Second: %d", t.GetComplexChunk().GetHeader().data);
  LogDebug("Trait: %d:", t1.GetTraitHeader().data);
  return 0;
}
#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

#include <cstdint>

using namespace IO::Common;
using namespace IO::Common::Traits;
using namespace IO::ADT;

struct TestComplexChunk : public ChunkCommon<ChunkIdentifiers::ADTRootChunks::MCNK>
                        , public AutoIOTraitInterface<TestComplexChunk, TraitType::Chunk>
{
private:
  DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> _header;

public:
  auto& GetHeader() { return _header; };

private:
  static constexpr
  AutoIOTrait
  <
    TraitEntries
    <
      TraitEntry<&TestComplexChunk::_header>
    >
  > _auto_trait {};
};

struct TestFile : public AutoIOTraitInterface<TestFile, TraitType::File>
{

  template
  <
    typename
    , IO::Common::Traits::TraitType
    , std::default_initializable
    , std::default_initializable
  >
  friend class IO::Common::Traits::AutoIOTraitInterface;

public:
  auto& GetHeader() const { return _header; };
  auto& GetComplexChunk() const { return _complex_chunk;};

private:
  DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> _header;
  TestComplexChunk _complex_chunk;

  static constexpr
  AutoIOTrait
  <
    TraitEntries
    <
      TraitEntry<&TestFile::_header>,
      TraitEntry<&TestFile::_complex_chunk>
    >
  > _auto_trait {};
};

int main()
{
  ByteBuffer bb {};

  // prepare file
  bb.Write(ChunkIdentifiers::ADTCommonChunks::MVER);
  bb.Write(static_cast<std::uint32_t>(sizeof(std::uint32_t)));
  bb.Write(static_cast<std::uint32_t>(0));

  bb.Write(ChunkIdentifiers::ADTRootChunks::MHDR);
  bb.Write(static_cast<std::uint32_t>(sizeof(std::uint32_t)));
  bb.Write(static_cast<std::uint32_t>(1));
  bb.Seek(0);

  // test file
  TestFile t;
  t.Read(bb);

  ByteBuffer bb1{};
  t.Write(bb1);

  Ensure(bb == bb1, "Read and Write do not match");

  LogDebug("First: %d", t.GetHeader().data);
  LogDebug("Second: %d", t.GetHeader2().data);
  return 0;
}
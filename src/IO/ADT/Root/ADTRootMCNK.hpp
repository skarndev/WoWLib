#pragma once
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/WorldConstants.hpp>
#include <IO/Common.hpp>

namespace IO::ADT
{
  class MCNKRoot
  {
  public:
    MCNKRoot();

    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer& buf);

    [[nodiscard]]
    bool IsInitialized() const { return true; };

  private:
    DataStructures::SMChunk _header;
    Common::DataArrayChunk
      <
        float
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCVT
        , false
        , Common::WorldConstants::CHUNKBUFSIZE
        , Common::WorldConstants::CHUNKBUFSIZE
      > _heightmap;

    Common::DataArrayChunk
      <
        Common::DataStructures::CArgb
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCLV
        , false
        , Common::WorldConstants::CHUNKBUFSIZE
        , Common::WorldConstants::CHUNKBUFSIZE
      > _vertex_lighting;

    Common::DataArrayChunk
      <
        DataStructures::MCCVEntry
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCCV
        , false
        , Common::WorldConstants::CHUNKBUFSIZE
        , Common::WorldConstants::CHUNKBUFSIZE
      > _vertex_color;

    Common::DataArrayChunk
      <
        DataStructures::MCNREntry
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCNR
        , false
        , Common::WorldConstants::CHUNKBUFSIZE
        , Common::WorldConstants::CHUNKBUFSIZE
      > _normals;

    Common::DataChunk<DataStructures::MCLQ, ChunkIdentifiers::ADTRootMCNKSubchunks::MCLQ> _tbc_water;
    Common::DataArrayChunk<DataStructures::MCSE, ChunkIdentifiers::ADTRootMCNKSubchunks::MCSE> _sound_emitters;
    Common::DataArrayChunk<DataStructures::MCBB, ChunkIdentifiers::ADTRootMCNKSubchunks::MCBB> _blend_batches;
    Common::DataChunk<std::uint64_t, ChunkIdentifiers::ADTRootMCNKSubchunks::MCDD> _groundeffect_disable;
  };
}
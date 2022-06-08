#pragma once
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
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
    Common::DataChunk<DataStructures::MCVT, ChunkIdentifiers::ADTRootMCNKSubchunks::MCVT> _heightmap;
    Common::DataChunk<DataStructures::MCLV, ChunkIdentifiers::ADTRootMCNKSubchunks::MCLV> _vertex_lighting;
    Common::DataChunk<DataStructures::MCCV, ChunkIdentifiers::ADTRootMCNKSubchunks::MCCV> _vertex_color;
    Common::DataChunk<DataStructures::MCNR, ChunkIdentifiers::ADTRootMCNKSubchunks::MCNR> _normals;
    Common::DataChunk<DataStructures::MCLQ, ChunkIdentifiers::ADTRootMCNKSubchunks::MCLQ> _tbc_water;
    Common::DataArrayChunk<DataStructures::MCSE, ChunkIdentifiers::ADTRootMCNKSubchunks::MCSE> _sound_emitters;
    Common::DataArrayChunk<DataStructures::MCBB, ChunkIdentifiers::ADTRootMCNKSubchunks::MCBB> _blend_batches;
    Common::DataChunk<std::uint64_t, ChunkIdentifiers::ADTRootMCNKSubchunks::MCDD> _groundeffect_disable;
  };
}
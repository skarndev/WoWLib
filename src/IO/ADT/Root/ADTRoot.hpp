#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/Root/ADTRootMCNK.hpp>
#include <IO/ADT/Root/MH2O.hpp>

#include <array>
#include <cstdint>

namespace IO::ADT
{
  class ADTRoot 
  {
  public:
    ADTRoot(std::uint32_t file_data_id);
    ADTRoot(std::uint32_t file_data_id, Common::ByteBuffer const& buf);

    [[nodiscard]]
    std::uint32_t FileDataID() const { return _file_data_id; };
    
    void Read(Common::ByteBuffer const& buf);
    void Write(Common::ByteBuffer& buf) const;

  private:
    std::uint32_t _file_data_id;

    Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> _version;
    std::array<MCNKRoot, 256> _chunks;
    MH2O _liquids;
    Common::DataChunk<DataStructures::MFBO, ChunkIdentifiers::ADTRootChunks::MFBO> _flight_bounds;
    Common::DataArrayChunk<DataStructures::MBMH, ChunkIdentifiers::ADTRootChunks::MBMH> _blend_mesh_headers;
    Common::DataArrayChunk<DataStructures::MBBB, ChunkIdentifiers::ADTRootChunks::MBBB> _blend_mesh_bounding_boxes;
    Common::DataArrayChunk<DataStructures::MBNV, ChunkIdentifiers::ADTRootChunks::MBNV> _blend_mesh_vertices;
    Common::DataArrayChunk<std::uint16_t, ChunkIdentifiers::ADTRootChunks::MBMI> _blend_mesh_indices;
  };


}
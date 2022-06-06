#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/Root/ADTRootMCNK.hpp>

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
    void Write(Common::ByteBuffer& buf);

  private:
    std::uint32_t _file_data_id;

    Common::DataChunk<std::uint32_t> _version;
    Common::DataChunk<DataStructures::MHDR> _header;
    std::array<MCNKRoot, 256> _chunks;
    // todo: mh20
    Common::DataChunk<DataStructures::MFBO> _flight_bounds;
    Common::DataArrayChunk<DataStructures::MBMH> _blend_mesh_headers;
    Common::DataArrayChunk<DataStructures::MBBB> _blend_mesh_bounding_boxes;
    Common::DataArrayChunk<DataStructures::MBNV> _blend_mesh_vertices;
    Common::DataArrayChunk<std::uint16_t> _blend_mesh_indices;
  };


}
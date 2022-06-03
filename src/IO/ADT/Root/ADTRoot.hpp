#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>


#include <array>
#include <cstdint>

namespace IO::ADT
{
  class ADTRoot 
  {
  public:
 
    ADTRoot(std::uint32_t file_data_id);

  
    Common::DataChunk<std::uint32_t> _version;
    Common::DataChunk<DataStructures::MHDR> _header;
    // todo: mh20, mcnks
    Common::DataChunk<DataStructures::MFBO> _flight_bounds;
    Common::DataArrayChunk<DataStructures::MBMH> _blend_mesh_headers;
    Common::DataArrayChunk<DataStructures::MBBB> _blend_mesh_bounding_boxes;
    Common::DataArrayChunk<DataStructures::MBNV> _blend_mesh_vertices;
    Common::DataArrayChunk<std::uint16_t> _blend_mesh_indices;

    void Read(Common::ByteBuffer const& buf);
    void Write(Common::ByteBuffer& buf);

    
  };


}
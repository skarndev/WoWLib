#pragma once
#include <IO/ADT/DataStructures.hpp>
#include <IO/Common.hpp>

namespace IO::ADT
{
  class MCNKRoot
  {
  public:
    MCNKRoot() = default;


    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer& buf);

  private:
    Common::DataChunk<DataStructures::SMChunk> header;
    Common::DataChunk<DataStructures::MCVT> heightmap;
    Common::DataChunk<DataStructures::MCLV> vertex_lighting;
    Common::DataChunk<DataStructures::MCCV> vertex_color;
    Common::DataChunk<DataStructures::MCNR> normals;
    Common::DataChunk<DataStructures::MCLQ> tbc_water;
    Common::DataArrayChunk<DataStructures::MCSE> sound_emitters;
    Common::DataArrayChunk<DataStructures::MCBB> blend_batches;
    Common::DataChunk<std::uint64_t> groundeffect_disable;


  };
}
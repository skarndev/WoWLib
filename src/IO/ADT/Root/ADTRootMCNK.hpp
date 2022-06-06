#pragma once
#include <IO/ADT/DataStructures.hpp>
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
    Common::DataChunk<DataStructures::MCVT> _heightmap;
    Common::DataChunk<DataStructures::MCLV> _vertex_lighting;
    Common::DataChunk<DataStructures::MCCV> _vertex_color;
    Common::DataChunk<DataStructures::MCNR> _normals;
    Common::DataChunk<DataStructures::MCLQ> _tbc_water;
    Common::DataArrayChunk<DataStructures::MCSE> _sound_emitters;
    Common::DataArrayChunk<DataStructures::MCBB> _blend_batches;
    Common::DataChunk<std::uint64_t> _groundeffect_disable;
  };
}
#include <IO/ADT/Root/MH2O.hpp>
#include <Utils/Meta/Future.hpp>

#include <cstdint>
#include <cassert>

using namespace IO::ADT;


void MH2O::Read(Common::ByteBuffer const& buf, std::size_t size)
{
  LogDebugF(LCodeZones::FILE_IO, "Loading ADT root chunk MH2O.");

  std::size_t data_pos = buf.Tell();

  std::array<DataStructures::SMLiquidChunk, 16 * 16> header_chunks{};
  buf.Read(header_chunks.begin(), header_chunks.end());

  
  for (auto&& [header_chunk, chunk] : future::zip(header_chunks, _chunks))
  {
    if (!header_chunk.layer_count)
      continue;

    std::vector<DataStructures::SMLiquidChunkAttributes> layer_attrs;
    std::vector<DataStructures::SMLiquidInstance> layer_instances;

    layer_attrs.resize(header_chunk.layer_count);
    layer_instances.resize(header_chunk.layer_count);

    buf.Seek(data_pos + header_chunk.offset_instances);
    buf.Read(layer_instances.begin(), layer_instances.end());

    bool has_attributes = header_chunk.offset_attributes;

    // attributes can be omitted for all-0 height liquids, in this case the offset is zero.
    if (has_attributes)
    {
      buf.Seek(data_pos + header_chunk.offset_attributes);
      buf.Read(layer_attrs.begin(), layer_attrs.end());
    }
    




  }

 



}

void MH2O::Write(Common::ByteBuffer& buf)
{
   
}

void LiquidLayer::SetLiquidObjectOrLiquidVertexFormat(std::uint16_t liquid_object_or_lvf)
{
  if (liquid_type < 42)
  {
    RequireF(CCodeZones::FILE_IO, liquid_type <= 3, "Bad liquid vertex format.");
    this->liquid_vertex_format = static_cast<LiquidLayer::LiquidvertexFormat>(liquid_type);
  }
  else
  {
    assert("Not implemented yet! Requires DB2 reader. Can't read this file.");
  }
}

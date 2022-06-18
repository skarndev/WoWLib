#include <IO/ADT/Root/MH2O.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <Utils/Meta/Future.hpp>

#include <cstdint>
#include <cassert>
#include <algorithm>

using namespace IO::ADT;
using namespace IO::ADT::ChunkIdentifiers;
using namespace IO::Common;


void MH2O::Read(Common::ByteBuffer const& buf, std::size_t size)
{
  LogDebugF(LCodeZones::FILE_IO, "Loading ADT root chunk MH2O.");

  std::size_t data_pos = buf.Tell();

  std::array<DataStructures::SMLiquidChunk, 16 * 16> header_chunks{};
  buf.Read(header_chunks.begin(), header_chunks.end());

  
  for (auto&& [header_chunk, chunk] : future::zip(header_chunks, _chunks))
  {

    if (!header_chunk.layer_count) [[unlikely]]
    {
       continue;
    }
    
    std::vector<DataStructures::SMLiquidInstance> layer_instances;

    layer_instances.resize(header_chunk.layer_count);

    buf.Seek(data_pos + header_chunk.offset_instances);
    buf.Read(layer_instances.begin(), layer_instances.end());

    bool has_attributes = header_chunk.offset_attributes;

    // attributes can be omitted for all-0 height liquids, in this case the offset is zero.
    if (has_attributes) [[likely]]
    {
      DataStructures::SMLiquidChunkAttributes attributes;

      buf.Seek(data_pos + header_chunk.offset_attributes);
      buf.Read(attributes);
      chunk.AddAttributes(attributes);
    }
    
    chunk.Layers().resize(header_chunk.layer_count);
    
    for (auto&& [layer, instance] : future::zip(chunk.Layers(), layer_instances))
    {
      layer.min_height_level = instance.min_height_level;
      layer.max_height_level = instance.max_height_level;

      // handle liquid instance
      layer.liquid_type = instance.liquid_type;
      layer.SetLiquidObjectOrLiquidVertexFormat(instance.liquid_object_or_lvf);

      // handle exists map, we un-compress it to a 64-bit full bitset for convenience
      std::uint8_t offset_index = instance.y_offset * 8 + instance.x_offset;
      std::uint8_t n_used_bits = instance.width * instance.height;

      layer.exists_map = std::bitset<64>(0);

      if (instance.offset_exists_bitmap)
      {
        buf.Seek(data_pos + instance.offset_exists_bitmap);

        std::uint8_t n_bytes_to_read = (instance.width * instance.height + 7) / 8;
        EnsureF(CCodeZones::FILE_IO, n_bytes_to_read <= 8, "MH2O: bad exists bitmap");

        std::uint64_t exists_bitmap = 0;
        buf.Read(reinterpret_cast<char*>(&exists_bitmap), n_bytes_to_read);

        if (!offset_index)
        {
          layer.exists_map = std::bitset<64>(exists_bitmap);
        }
        else
        {
          auto bitmap_no_offset = std::bitset<64>(exists_bitmap);
          for (int i = 0; i < n_used_bits; ++i)
          {
            layer.exists_map[offset_index + i] = bitmap_no_offset[i];
          }
        }
      }

      // handle vertex data

      if (instance.offset_vertex_data)
      {
        layer.has_vertex_data = true;
        buf.Seek(data_pos + instance.offset_vertex_data);

        std::uint8_t offset = instance.y_offset * 8 + instance.x_offset;
        std::uint8_t end_offset = offset + (instance.width + 1) * (instance.height + 1);

        switch (layer.liquid_vertex_format)
        {
          case LiquidLayer::LiquidVertexFormat::HEIGHT_DEPTH:
          {
            auto& layer_data = layer.vertex_data.emplace<DataStructures::MH2OHeightDepth>();
            buf.Read(layer_data.heightmap.begin() + offset, layer_data.heightmap.begin() + end_offset);
            buf.Read(layer_data.depthmap.begin() + offset, layer_data.depthmap.begin() + end_offset);
            break;
          }
          case LiquidLayer::LiquidVertexFormat::HEIGHT_TEXCOORD:
          {
            auto& layer_data = layer.vertex_data.emplace<DataStructures::MH2OHeightTexCoord>();
            buf.Read(layer_data.heightmap.begin() + offset, layer_data.heightmap.begin() + end_offset);
            buf.Read(layer_data.uvmap.begin() + offset, layer_data.uvmap.begin() + end_offset);
            break;
          }
          case LiquidLayer::LiquidVertexFormat::DEPTH:
          {
            auto& layer_data = layer.vertex_data.emplace<DataStructures::MH2ODepth>();
            buf.Read(layer_data.depthmap.begin() + offset, layer_data.depthmap.begin() + end_offset);
            break;
          }
          case LiquidLayer::LiquidVertexFormat::HEIGHT_DEPTH_TEXCOORD:
          {
            auto& layer_data = layer.vertex_data.emplace<DataStructures::MH2OHeightDepthTexCoord>();
            buf.Read(layer_data.heightmap.begin() + offset, layer_data.heightmap.begin() + end_offset);
            buf.Read(layer_data.depthmap.begin() + offset, layer_data.depthmap.begin() + end_offset);
            buf.Read(layer_data.uvmap.begin() + offset, layer_data.uvmap.begin() + end_offset);
            break;
          }

        }
      }

    }

    buf.Seek(data_pos + size);

  }

}

void MH2O::Write(Common::ByteBuffer& buf)
{
  LogDebugF(LCodeZones::FILE_IO, "Writing chunk: MH20.");

  std::size_t pos = buf.Tell();

  // writing chunk header to just initialize buffer bytes
  ChunkHeader chunk_header{};
  buf.Write(chunk_header);

  std::size_t data_pos = buf.Tell();

  std::array<DataStructures::SMLiquidChunk, 16 * 16> header_chunks{};
  buf.Reserve(16 * 16 * sizeof(DataStructures::SMLiquidChunk));

  for (auto&& [header_chunk, chunk] : future::zip(header_chunks, _chunks))
  {
    header_chunk.layer_count = chunk.Layers().size();

    if (header_chunk.layer_count)
    {
      std::size_t instances_pos = buf.Tell();
      header_chunk.offset_instances = buf.Tell() - data_pos;

      std::vector<DataStructures::SMLiquidInstance> liquid_instances;
      liquid_instances.resize(header_chunk.layer_count);

      // allocate space for all layers
      buf.Reserve(header_chunk.layer_count * sizeof(DataStructures::SMLiquidInstance));

      // fill layers
      for (auto&& [layer, instance] : future::zip(chunk.Layers(), liquid_instances))
      {
        instance.liquid_object_or_lvf = layer.GetLiquidObjectOrLVF();
        instance.liquid_type = layer.liquid_type;

        EnsureF(CCodeZones::FILE_IO, layer.exists_map.to_ullong(), "Attempted to write unused liquid layer. Editor code should clean those up.");

        bool all_exist = true;
        for (std::uint8_t i = 0; i < 64; ++i)
        {
          if (!layer.exists_map[i])
          {
            all_exist = false;
            break;
          }
        }

        if (all_exist)
        {
          instance.x_offset = 0;
          instance.y_offset = 0;
          instance.width = 8;
          instance.height = 8;
          instance.offset_exists_bitmap = 0;
        }
        else
        {

          // identify the used rectangle
          std::uint8_t begin = 0;
          std::uint8_t end = 0;

          for (std::uint8_t i = 0; i < 64; ++i)
          {
            if (layer.exists_map[i])
            {
              begin = i;
              break;
            }
          }

          for (std::int8_t i = 63; i >= 0; --i)
          {
            if (layer.exists_map[i])
            {
              end = i;
              break;
            }
          }

          instance.x_offset = begin % 8;
          instance.y_offset = begin / 8;

          instance.width = (end % 8) - (begin % 8) + 1;
          instance.height = (end / 8) - (begin / 8) + 1;

          // identify the number of bytes required to store

          std::uint8_t n_exists_bitmap_bytes = (instance.width * instance.height + 7) / 8;

          std::bitset<64> bitmap_temp{0};
          std::uint8_t counter = 0;
          for (std::uint8_t i = begin; i <= end; ++i)
          {
            bitmap_temp[counter] = layer.exists_map[i];
            counter++;
          }

          std::uint64_t exists_bitmap = bitmap_temp.to_ullong();

          instance.offset_exists_bitmap = buf.Tell() - data_pos;
          buf.Write(reinterpret_cast<char*>(&exists_bitmap), n_exists_bitmap_bytes);
        }

        // write vertex data
        if (layer.has_vertex_data)
        {
          instance.offset_vertex_data = buf.Tell() - data_pos;

          EnsureF(CCodeZones::FILE_IO, layer.vertex_data.index() == static_cast<unsigned>(layer.liquid_vertex_format),
                  "MH2O layer: wrong vertex format, expected %d, got %d.", static_cast<unsigned>(layer.liquid_vertex_format), layer.vertex_data.index());

          std::uint8_t begin_offset = instance.y_offset * 8 + instance.x_offset;
          std::uint8_t end_offset = begin_offset + (instance.width + 1) * (instance.height + 1);
          switch (layer.liquid_vertex_format)
          {
            case LiquidLayer::LiquidVertexFormat::HEIGHT_DEPTH:
            {
              auto& layer_data = std::get<DataStructures::MH2OHeightDepth>(layer.vertex_data);

              buf.Write(layer_data.heightmap.begin() + begin_offset, layer_data.heightmap.begin() + end_offset);
              buf.Write(layer_data.depthmap.begin() + begin_offset, layer_data.depthmap.begin() + end_offset);
              break;
            }
            case LiquidLayer::LiquidVertexFormat::HEIGHT_TEXCOORD:
            {
              auto& layer_data = std::get<DataStructures::MH2OHeightTexCoord>(layer.vertex_data);
              buf.Write(layer_data.heightmap.begin() + begin_offset, layer_data.heightmap.begin() + end_offset);
              buf.Write(layer_data.uvmap.begin() + begin_offset, layer_data.uvmap.begin() + end_offset);
              break;
            }
            case LiquidLayer::LiquidVertexFormat::DEPTH:
            {
              auto& layer_data = std::get<DataStructures::MH2ODepth>(layer.vertex_data);
              buf.Write(layer_data.depthmap.begin() + begin_offset, layer_data.depthmap.begin() + end_offset);
              break;
            }
            case LiquidLayer::LiquidVertexFormat::HEIGHT_DEPTH_TEXCOORD:
            {
              auto& layer_data = std::get<DataStructures::MH2OHeightDepthTexCoord>(layer.vertex_data);
              buf.Write(layer_data.heightmap.begin() + begin_offset, layer_data.heightmap.begin() + end_offset);
              buf.Write(layer_data.depthmap.begin() + begin_offset, layer_data.depthmap.begin() + end_offset);
              buf.Write(layer_data.uvmap.begin() + begin_offset, layer_data.uvmap.begin() + end_offset);
              break;
            }
          }
        }
        else
        {
          instance.offset_vertex_data = 0;
        }
        


     
     
      }

      std::size_t end_pos = buf.Tell();

      // actually write instance data
      buf.Seek(instances_pos);
      buf.Write(liquid_instances.begin(), liquid_instances.end());

      buf.Seek(end_pos);

      // handle attributes
      if (chunk.Attributes().has_value())
      {
        header_chunk.offset_attributes = buf.Tell() - data_pos;
        DataStructures::SMLiquidChunkAttributes attrs{chunk.Attributes()->fishable.to_ullong(), chunk.Attributes()->deep.to_ullong()};
        buf.Write(attrs);
      }
      else
      {
        header_chunk.offset_attributes = 0;
      }
    }
    else
    {
      header_chunk.offset_attributes = 0;
      header_chunk.offset_instances = 0;
    }

  }

  // go back and write relevant header data
  std::size_t end_pos = buf.Tell();
  buf.Seek(data_pos);
  buf.Write(header_chunks.begin(), header_chunks.end());
  buf.Seek(end_pos);
}

void LiquidLayer::SetLiquidObjectOrLiquidVertexFormat(std::uint16_t liquid_object_or_lvf)
{
  if (liquid_object_or_lvf < 42)
  {
    RequireF(CCodeZones::FILE_IO, liquid_object_or_lvf <= 3, "Bad liquid vertex format.");
    this->liquid_vertex_format = static_cast<LiquidLayer::LiquidVertexFormat>(liquid_object_or_lvf);
  }
  else
  {
    assert("Not implemented yet! Requires DB2 reader. Can't read this file.");
  }
}

std::uint16_t IO::ADT::LiquidLayer::GetLiquidObjectOrLVF()
{
  // TODO: get back here
  return static_cast<std::uint16_t>(this->liquid_vertex_format);
}

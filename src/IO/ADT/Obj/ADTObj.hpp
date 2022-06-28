#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/ADT/Obj/ADTObjMCNK.hpp>

#include <cstdint>
#include <array>
#include <type_traits>

namespace IO::ADT
{
  enum class ADTObjLodLevel
  {
    NORMAL = 0,
    LOD = 1
  };

  class AdtObj0SpecificData
  {
  public:
    AdtObj0SpecificData();

  protected:
    Common::DataArrayChunk<DataStructures::MDDF, ChunkIdentifiers::ADTObj0Chunks::MDDF> _model_placements;
    Common::DataArrayChunk<DataStructures::MODF, ChunkIdentifiers::ADTObj0Chunks::MODF> _map_object_placements;
    std::array<MCNKObj, 16 * 16> _chunks;
  };

  class AdtObj1SpecificData
  {
  public:
    AdtObj1SpecificData();

  protected:
    Common::DataArrayChunk<DataStructures::MLMD, ChunkIdentifiers::ADTObj1Chunks::MLMD> _lod_map_object_placements;
    Common::DataArrayChunk<DataStructures::MLMX, ChunkIdentifiers::ADTObj1Chunks::MLMX> _lod_map_object_extents;
    Common::DataArrayChunk<DataStructures::MDDF, ChunkIdentifiers::ADTObj1Chunks::MLDD> _lod_model_placements;
    Common::DataArrayChunk<DataStructures::MLDX, ChunkIdentifiers::ADTObj1Chunks::MLDX> _lod_model_extents;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj1Chunks::MLDL> _lod_model_unknown;
    Common::DataArrayChunk<DataStructures::MLFD, ChunkIdentifiers::ADTObj1Chunks::MLFD> _lod_mapping;
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObj1Chunks::MLDB> _map_object_lod_batches;
  };

  template<ADTObjLodLevel lod_level>
  class ADTObj
      : public std::conditional_t<static_cast<std::uint8_t>(lod_level), AdtObj1SpecificData, AdtObj0SpecificData>
  {
  public:
    explicit ADTObj(std::uint32_t file_data_id)
        : std::conditional_t<static_cast<std::uint8_t>(lod_level), AdtObj1SpecificData, AdtObj0SpecificData>()
        , _file_data_id(file_data_id)
    {
    };

    void Read(Common::ByteBuffer const& buf, std::size_t size);

    void Write(Common::ByteBuffer& buf) const;

    void GenerateLod(ADTObj<ADTObjLodLevel::NORMAL> const& tile_obj) requires (lod_level == ADTObjLodLevel::LOD);

  private:
    std::uint32_t _file_data_id;

    // common obj0 & obj1 chunks
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObjCommonChunks::MLMB> _lod_map_object_batches;
    Common::DataArrayChunk<std::int16_t, ChunkIdentifiers::ADTObjCommonChunks::MWDS> _wmo_dooodadset_overrides;
    Common::DataArrayChunk<DataStructures::MWDR, ChunkIdentifiers::ADTObjCommonChunks::MWDR> _wmo_doodadset_overrides_ranges;
  };

  // impl

  template<ADTObjLodLevel lod_level>
  inline void ADTObj<lod_level>::Read(IO::Common::ByteBuffer const& buf, std::size_t size)
  {
    LogDebugF(LCodeZones::FILE_IO, "Reading ADT Obj%d. Filedata ID: %d."
              , static_cast<std::uint8_t>(lod_level), _file_data_id);
    LogIndentScoped;

    RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
    RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");

    std::size_t chunk_counter = 0;

    while (!buf.IsEof())
    {
      auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

      switch (chunk_header.fourcc)
      {
        case ChunkIdentifiers::ADTCommonChunks::MVER:
        {
          Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> version{18};
          version.Read(buf, chunk_header.size);
          continue;
        }
        case ChunkIdentifiers::ADTObjCommonChunks::MWDR:
          _wmo_doodadset_overrides_ranges.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTObjCommonChunks::MWDS:
          _wmo_dooodadset_overrides.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTObjCommonChunks::MLMB:
          _lod_map_object_batches.Read(buf, chunk_header.size);
          continue;
      }

      // handle the obj0-specific stuff here
      if constexpr(lod_level == ADTObjLodLevel::NORMAL)
      {
        switch (chunk_header.fourcc)
        {
          case ChunkIdentifiers::ADTObj0Chunks::MCNK:
            LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCNK (obj0) (%d / 255), size: %d."
                      , chunk_counter, chunk_header.size);
            this->_chunks[chunk_counter++].Read(buf, chunk_header.size);
            break;
          case ChunkIdentifiers::ADTObj0Chunks::MDDF:
            this->_model_placements.Read(buf, chunk_header.size);
            break;
          case ChunkIdentifiers::ADTObj0Chunks::MODF:
            this->_map_object_placements.Read(buf, chunk_header.size);
            break;
          default:
            buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
            LogError("Encountered unknown ADT Obj0 chunk %s.", Common::FourCCToStr(chunk_header.fourcc).c_str());
            break;
        }
      }
      else
      // handle the obj1-specific stuff here
      {
        switch (chunk_header.fourcc)
        {
          case ChunkIdentifiers::ADTObj1Chunks::MLMD:
            this->_lod_map_object_placements.Read(buf, chunk_header.size);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLMX:
            this->_lod_map_object_extents.Read(buf, chunk_header.size);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLDD:
            this->_lod_model_placements.Read(buf, chunk_header.size);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLDX:
            this->_lod_model_extents.Read(buf, chunk_header.size);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLDL:
            this->_lod_model_unknown.Read(buf, chunk_header.size);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLFD:
            this->_lod_mapping.Read(buf, chunk_header.size);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLDB:
            this->_map_object_lod_batches.Read(buf, chunk_header.size);
            break;
          default:
            buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
            LogError("Encountered unknown ADT Obj1 chunk %s.", Common::FourCCToStr(chunk_header.fourcc).c_str());
            break;
        }
      }
    }
  }

  template<ADTObjLodLevel lod_level>
  inline void ADTObj<lod_level>::Write(Common::ByteBuffer& buf) const
  {

  }

  template<ADTObjLodLevel lod_level>
  inline void ADTObj<lod_level>::GenerateLod(ADTObj<ADTObjLodLevel::NORMAL> const& tile_obj)
  requires (lod_level == ADTObjLodLevel::LOD)
  {

  }

}




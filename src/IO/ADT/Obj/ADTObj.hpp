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

  class AdtObj1Data
  {
  protected:
    Common::DataArrayChunk<DataStructures::MLMD, ChunkIdentifiers::ADTObj1Chunks::MLMD> _lod_map_object_placements;
    Common::DataArrayChunk<DataStructures::MLMX, ChunkIdentifiers::ADTObj1Chunks::MLMX> _lod_map_object_extents;
    Common::DataArrayChunk<DataStructures::MDDF, ChunkIdentifiers::ADTObj1Chunks::MLDD> _lod_model_placements;
    Common::DataArrayChunk<DataStructures::MLDX, ChunkIdentifiers::ADTObj1Chunks::MLDX> _lod_model_extents;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj1Chunks::MLDL> _lod_model_unknown;
    Common::DataArrayChunk<DataStructures::MLFD, ChunkIdentifiers::ADTObj1Chunks::MLFD> _lod_mapping;
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObj1Chunks::MLDB> _map_object_lod_batches;
  };

  class AdtObj1DataEmpty
  {

  };

  template<ADTObjLodLevel lod_level>
  class ADTObj : std::conditional_t<static_cast<std::uint8_t>(lod_level), AdtObj1Data, AdtObj1DataEmpty>
  {
  public:
    explicit ADTObj(std::uint32_t file_data_id)
    : _file_data_id(file_data_id)
    {
      _model_placements.Initialize();
      _map_object_placements.Initialize();
    };

    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer& buf) const;

  private:
    std::uint32_t _file_data_id;
    Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> _version;
    Common::DataArrayChunk<DataStructures::MDDF, ChunkIdentifiers::ADTObj0Chunks::MDDF> _model_placements;
    Common::DataArrayChunk<DataStructures::MODF, ChunkIdentifiers::ADTObj0Chunks::MODF> _map_object_placements;

    std::array<MCNKObj, 16 * 16> _chunks;
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObj0Chunks::MLMB> _lod_map_object_batches;
    Common::DataArrayChunk<std::int16_t, ChunkIdentifiers::ADTObj0Chunks::MWDS> _wmo_dooodadset_overrides;
    Common::DataArrayChunk<DataStructures::MWDR, ChunkIdentifiers::ADTObj0Chunks::MWDR> _wmo_doodadset_overrides_ranges;
  };

  template<ADTObjLodLevel lod_level>
  inline void ADTObj<lod_level>::Read(IO::Common::ByteBuffer const& buf, std::size_t size)
  {
    LogDebugF(LCodeZones::FILE_IO, "Reading ADT Obj%d. Filedata ID: %d."
              , static_cast<std::uint8_t>(lod_level), _file_data_id);
    LogIndentScoped;

    RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
    RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");

    std::size_t chunk_counter = 0;

    while(!buf.IsEof())
    {
      auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

      switch (chunk_header.fourcc)
      {
        case ChunkIdentifiers::ADTCommonChunks::MVER:
          _version.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTObj0Chunks::MDDF:
          _model_placements.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTObj0Chunks::MODF:
          _map_object_placements.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTObj0Chunks::MCNK:
          LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCNK (obj) (%d / 255), size: %d."
                    , chunk_counter, chunk_header.size);
          _chunks[chunk_counter++].Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTObj0Chunks::MLMB:
          _lod_map_object_batches.Read(buf, chunk_header.size);
          continue;
        default:
        {
          if constexpr (lod_level != ADTObjLodLevel::LOD)
          {
            buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
            LogError("Encountered unknown ADT root chunk %s.", Common::FourCCToStr(chunk_header.fourcc).c_str());
            continue;
          }
        }
      }
      // handle the obj1 stuff here
      if constexpr (lod_level == ADTObjLodLevel::LOD)
      {
        switch (chunk_header.fourcc)
        {
          case ChunkIdentifiers::ADTObj1Chunks::MLMD:
            this->_lod_map_object_placements.Read(buf);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLMX:
            this->_lod_map_object_extents.Read(buf);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLDD:
            this->_lod_model_placements.Read(buf);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLDX:
            this->_lod_model_extents.Read(buf);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLDL:
            this->_lod_model_unknown.Read(buf);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLFD:
            this->_lod_mapping.Read(buf);
            break;
          case ChunkIdentifiers::ADTObj1Chunks::MLDB:
            this->_map_object_lod_batches.Read(buf);
            break;
          default:
            buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
            LogError("Encountered unknown ADT root chunk %s.", Common::FourCCToStr(chunk_header.fourcc).c_str());
            break;
        }
      }
    }
  }

  template<ADTObjLodLevel lod_level>
  void ADTObj<lod_level>::Write(Common::ByteBuffer& buf) const
  {

  }

}




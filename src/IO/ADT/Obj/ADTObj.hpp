#ifndef IO_ADT_OBJ_ADTOBJ_HPP
#define IO_ADT_OBJ_ADTOBJ_HPP

#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/ADT/Obj/ADTObjMCNK.hpp>
#include <IO/WorldConstants.hpp>

#include <cstdint>
#include <array>
#include <type_traits>

namespace IO::ADT
{
  enum class ADTObjLodLevel
  {
    // obj0 file
    NORMAL = 0,

    // obj1 file
    LOD = 1
  };

  // ADT obj0-specific

  // Enables storing textures by filepath
  class ADTObj0ModelStorageFilepath
  {
  protected:
    Common::StringBlockChunk<Common::StringBlockChunkType::OFFSET
                              , ChunkIdentifiers::ADTObj0Chunks::MMDX
                            > _model_filenames;

    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj0Chunks::MMID> _model_filename_offsets;

    Common::StringBlockChunk<Common::StringBlockChunkType::OFFSET
                              , ChunkIdentifiers::ADTObj0Chunks::MWMO
                            > _map_object_filenames;

    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj0Chunks::MWID> _map_object_filename_offsets;
  };
  class ADTObj0NoModelStorageFilepath {};

  template<Common::ClientVersion client_version>
  class AdtObj0SpecificData : public std::conditional_t<client_version < Common::ClientVersion::BFA
                                                        , ADTObj0ModelStorageFilepath
                                                        , ADTObj0NoModelStorageFilepath
                                                       >
  {
  public:
    AdtObj0SpecificData();

  protected:
    Common::DataArrayChunk<DataStructures::MDDF, ChunkIdentifiers::ADTObj0Chunks::MDDF> _model_placements;
    Common::DataArrayChunk<DataStructures::MODF, ChunkIdentifiers::ADTObj0Chunks::MODF> _map_object_placements;
    std::array<MCNKObj, Common::WorldConstants::CHUNKS_PER_TILE> _chunks;
  };

  // ADT obj-1 specific

  // Enables support for model lod batches in obj1
  class ADTObj1SpecificDataDoodadLodBatches
  {
  protected:
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObj1Chunks::MLDB> _lod_model_batches;
  };
  class ADTObj1SpecificDataNoDoodadLodBatches {};

  template<Common::ClientVersion client_version>
  class AdtObj1SpecificData
    : public std::conditional_t<client_version >= Common::ClientVersion::SL
                                , ADTObj1SpecificDataDoodadLodBatches
                                , ADTObj1SpecificDataNoDoodadLodBatches
                               >
  {
  static_assert(client_version >= Common::ClientVersion::LEGION && "Obj1 files are only supported since Legion.");
  public:
    AdtObj1SpecificData();

  protected:
    Common::DataArrayChunk<DataStructures::MLMD, ChunkIdentifiers::ADTObj1Chunks::MLMD> _lod_map_object_placements;
    Common::DataArrayChunk<DataStructures::MLMX, ChunkIdentifiers::ADTObj1Chunks::MLMX> _lod_map_object_extents;
    Common::DataArrayChunk<DataStructures::MDDF, ChunkIdentifiers::ADTObj1Chunks::MLDD> _lod_model_placements;
    Common::DataArrayChunk<DataStructures::MLDX, ChunkIdentifiers::ADTObj1Chunks::MLDX> _lod_model_extents;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj1Chunks::MLDL> _lod_model_unknown;
    Common::DataArrayChunk<DataStructures::MLFD, ChunkIdentifiers::ADTObj1Chunks::MLFD> _lod_mapping;
  };

  // switch the implementation (obj0 vs obj1)
  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  using LodLevelImpl = std::conditional_t<static_cast<std::uint8_t>(lod_level)
                                              , AdtObj1SpecificData<client_version>
                                              , AdtObj0SpecificData<client_version>
                                             >;

  // Enables support for LOD map object batches (BfA+).
  class ADTObjWithLodMapObjectBatches
  {
  protected:
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObjCommonChunks::MLMB> _lod_map_object_batches;
  };
  class ADTObjNoLodMapObjectBatches {};

  class ADTObjWithDoodadsetOverrides
  {
  protected:
    Common::DataArrayChunk<std::int16_t, ChunkIdentifiers::ADTObjCommonChunks::MWDS> _wmo_doodadset_overrides;
    Common::DataArrayChunk<DataStructures::MWDR
        , ChunkIdentifiers::ADTObjCommonChunks::MWDR
    > _wmo_doodadset_overrides_ranges;
  };

  class ADTObjNoDoodadsetOverides {};



  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  class ADTObj
      : public LodLevelImpl<client_version, lod_level>
      , public std::conditional_t<client_version >= Common::ClientVersion::BFA
                                  , ADTObjWithLodMapObjectBatches
                                  , ADTObjNoLodMapObjectBatches
                                 >
      , public std::conditional_t<client_version >= Common::ClientVersion::SL
                                  , ADTObjWithDoodadsetOverrides
                                  , ADTObjNoDoodadsetOverides
                                >
  {
  static_assert(client_version >= Common::ClientVersion::CATA && "Split files did not exist before Cataclysm.");
  public:
    explicit ADTObj(std::uint32_t file_data_id)
        : LodLevelImpl<client_version, lod_level>()
        , _file_data_id(file_data_id)
    {
    };

    void Read(Common::ByteBuffer const& buf);

    void Write(Common::ByteBuffer& buf) const;

    template<Common::ClientVersion client_v>
    void GenerateLod(ADTObj<client_v, ADTObjLodLevel::NORMAL> const& tile_obj)
    requires (lod_level == ADTObjLodLevel::LOD);

  private:
    // obj0
    [[nodiscard]]
    bool ReadObj0SpecificChunk(Common::ByteBuffer const& buf
                               , Common::ChunkHeader const& chunk_header
                               , std::uint32_t& chunk_counter) requires (lod_level == ADTObjLodLevel::NORMAL);

    void WriteObj0SpecificChunks(Common::ByteBuffer& buf) const requires (lod_level == ADTObjLodLevel::NORMAL);

    // This method converts MODF/MDDF references to filename offsets into filename indices.
    void PatchObjectFilenameReferences() requires (lod_level == ADTObjLodLevel::NORMAL);

    // TODO: chunk protocol concepts here
    template<Common::Concepts::DataArrayChunkProtocol FilepathOffsetStorage
        , Common::Concepts::StringBlockChunkProtocol FilepathStorage
        , Common::Concepts::DataArrayChunkProtocol InstanceStorage>
    void PatchObjectFilenameReferences_detail(FilepathOffsetStorage& offset_storage
                                              , FilepathStorage& filepath_storage
                                              , InstanceStorage& instance_storage)
    requires (lod_level == ADTObjLodLevel::NORMAL);

    // obj1
    [[nodiscard]]
    bool ReadObj1SpecificChunk(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
    requires (lod_level == ADTObjLodLevel::LOD);

    void WriteObj1SpecificChunks(Common::ByteBuffer& buf) const requires (lod_level == ADTObjLodLevel::LOD);

    std::uint32_t _file_data_id;

  };
}
#include <IO/ADT/Obj/ADTObj.inl>
#endif // IO_ADT_OBJ_ADTOBJ_HPP



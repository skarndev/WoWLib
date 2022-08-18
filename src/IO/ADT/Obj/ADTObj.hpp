#ifndef IO_ADT_OBJ_ADTOBJ_HPP
#define IO_ADT_OBJ_ADTOBJ_HPP

#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/ADT/Obj/ADTObjMCNK.hpp>
#include <IO/WorldConstants.hpp>
#include <Utils/Meta/Templates.hpp>

#include <cstdint>
#include <array>
#include <type_traits>

namespace IO::ADT
{
  /**
   * Determines LOD level of ADTObj file.
   */
  enum class ADTObjLodLevel
  {
    NORMAL = 0, ///> obj0 file.
    LOD = 1 ///> obj1 file.
  };

  // forward decl traits
  class ADTObj0ModelStorageFilepath;

  template<Common::ClientVersion client_version>
  class AdtObj0SpecificData;

  template<Common::ClientVersion client_version>
  class AdtObj1SpecificData;

  class ADTLodMapObjectBatches;

  class ADTDoodadsetOverrides;

  /**
   * Switches the implementation of ADTObj file based on lod level.
   * @tparam client_version Version of the game client.
   * @tparam lod_level Lod level.
   */
  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  using LodLevelImpl = Common::Traits::SwitchableTrait
    <
      lod_level
      , Common::Traits::TraitCase
      <
        ADTObjLodLevel::NORMAL
        , AdtObj0SpecificData<client_version>
      >
      , Common::Traits::VersionedTraitCase
      <
        ADTObjLodLevel::LOD
        , AdtObj1SpecificData<client_version>
        , client_version
        , Common::ClientVersion::LEGION
      >
    >;

  /**
   * Split ADT file containing data associated with object placements.
   * @tparam client_version Version of the game client.
   * @tparam lod_level Lod level.
   */
  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  class ADTObj : public Common::Traits::AutoIOTraits
                        <
                          Common::Traits::IOTraits
                          <
                            Common::Traits::IOTrait<LodLevelImpl<client_version, lod_level>>
                            , Common::Traits::IOTrait
                              <
                                Common::Traits::VersionTrait
                                <
                                  ADTLodMapObjectBatches
                                  , client_version
                                  , Common::ClientVersion::BFA
                                >
                              >
                            , Common::Traits::IOTrait
                              <
                                Common::Traits::VersionTrait
                                <
                                  ADTDoodadsetOverrides
                                  , client_version
                                  , Common::ClientVersion::SL
                                >
                              >
                          >
                        >
               , public Common::Traits::AutoIOTraitInterface
                        <
                          ADTObj<client_version, lod_level>
                          , Common::Traits::DefaultTraitContext
                          , Common::Traits::DefaultTraitContext
                          , Common::Traits::TraitType::File
                        >
  {
  static_assert(client_version >= Common::ClientVersion::CATA && "Split files did not exist before Cataclysm.");
  public:
    explicit ADTObj(std::uint32_t file_data_id)
        : _file_data_id(file_data_id)
    {
    };

  private:
    // This method converts MODF/MDDF references to filename offsets into filename indices.
    void PatchObjectFilenameReferences() requires (lod_level == ADTObjLodLevel::NORMAL);

    template
    <
      Common::Concepts::DataArrayChunkProtocol FilepathOffsetStorage
      , Common::Concepts::StringBlockChunkProtocol FilepathStorage
      , Common::Concepts::DataArrayChunkProtocol InstanceStorage
    >
    void PatchObjectFilenameReferences_detail(FilepathOffsetStorage& offset_storage
                                              , FilepathStorage& filepath_storage
                                              , InstanceStorage& instance_storage)
    requires (lod_level == ADTObjLodLevel::NORMAL);

    std::uint32_t _file_data_id;

  };

  // ADT obj0-specific

  // Enables storing textures by filepath
  class ADTObj0ModelStorageFilepath : public Common::Traits::AutoIOTraitInterface<ADTObj0ModelStorageFilepath>
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

  private:
    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&ADTObj0ModelStorageFilepath::_model_filenames>
        , Common::Traits::TraitEntry<&ADTObj0ModelStorageFilepath::_model_filename_offsets>
        , Common::Traits::TraitEntry<&ADTObj0ModelStorageFilepath::_map_object_filenames>
        , Common::Traits::TraitEntry<&ADTObj0ModelStorageFilepath::_map_object_filename_offsets>
      >
    > _auto_trait {};
  };

  template<Common::ClientVersion client_version>
  class AdtObj0SpecificData : public Common::Traits::AutoIOTraits
                                     <
                                       Common::Traits::IOTraits
                                       <
                                        Common::Traits::IOTrait
                                        <
                                          Common::Traits::VersionTrait
                                          <
                                            ADTObj0ModelStorageFilepath
                                            , client_version
                                            , Common::ClientVersion::CATA
                                            , Common::ClientVersion::BFA
                                          >
                                        >
                                       >
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
  class LodModelBatches : public Common::Traits::AutoIOTraitInterface<LodModelBatches>
  {
  protected:
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObj1Chunks::MLDB> _lod_model_batches;

  private:
    static constexpr Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&LodModelBatches::_lod_model_batches>
      >
    > _auto_trait {};
  } IMPLEMENTS_IO_TRAIT(LodModelBatches);

  template<Common::ClientVersion client_version>
  class AdtObj1SpecificData : public Common::Traits::AutoIOTraits
                                     <
                                       Common::Traits::IOTraits
                                       <
                                         Common::Traits::IOTrait
                                         <
                                           Common::Traits::VersionTrait
                                             <
                                               LodModelBatches
                                               , client_version
                                               , Common::ClientVersion::SL
                                             >
                                         >
                                       >
                                     >
                              , public Common::Traits::AutoIOTraitInterface<AdtObj1SpecificData<client_version>>
  {
  public:
    AdtObj1SpecificData();

    template<Common::ClientVersion client_v>
    void GenerateLod(ADTObj<client_v, ADTObjLodLevel::NORMAL> const& tile_obj);

  protected:
    Common::DataArrayChunk<DataStructures::MLMD, ChunkIdentifiers::ADTObj1Chunks::MLMD> _lod_map_object_placements;
    Common::DataArrayChunk<DataStructures::MLMX, ChunkIdentifiers::ADTObj1Chunks::MLMX> _lod_map_object_extents;
    Common::DataArrayChunk<DataStructures::MDDF, ChunkIdentifiers::ADTObj1Chunks::MLDD> _lod_model_placements;
    Common::DataArrayChunk<DataStructures::MLDX, ChunkIdentifiers::ADTObj1Chunks::MLDX> _lod_model_extents;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj1Chunks::MLDL> _lod_model_unknown;
    Common::DataArrayChunk<DataStructures::MLFD, ChunkIdentifiers::ADTObj1Chunks::MLFD> _lod_mapping;

  public:
    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&AdtObj1SpecificData::_lod_map_object_placements>
        , Common::Traits::TraitEntry<&AdtObj1SpecificData::_lod_map_object_extents>
        , Common::Traits::TraitEntry<&AdtObj1SpecificData::_lod_model_placements>
        , Common::Traits::TraitEntry<&AdtObj1SpecificData::_lod_model_extents>
        , Common::Traits::TraitEntry<&AdtObj1SpecificData::_lod_model_unknown>
        , Common::Traits::TraitEntry<&AdtObj1SpecificData::_lod_mapping>
      >
    > _auto_trait {};
  };

  // Enables support for LOD map object batches (BfA+).
  class ADTLodMapObjectBatches : public Common::Traits::AutoIOTraitInterface<ADTLodMapObjectBatches>
  {
  protected:
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObjCommonChunks::MLMB> _lod_map_object_batches;

  private:
    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&ADTLodMapObjectBatches::_lod_map_object_batches>
      >
    > _auto_trait = {};
  };

  class ADTDoodadsetOverrides : public Common::Traits::AutoIOTraitInterface<ADTDoodadsetOverrides>
  {
  protected:
    Common::DataArrayChunk<std::int16_t, ChunkIdentifiers::ADTObjCommonChunks::MWDS> _wmo_doodadset_overrides;
    Common::DataArrayChunk<DataStructures::MWDR
                            , ChunkIdentifiers::ADTObjCommonChunks::MWDR> _wmo_doodadset_overrides_ranges;

  private:
   static constexpr
   Common::Traits::AutoIOTrait
   <
    Common::Traits::TraitEntries
    <
      Common::Traits::TraitEntry<&ADTDoodadsetOverrides::_wmo_doodadset_overrides>
      , Common::Traits::TraitEntry<&ADTDoodadsetOverrides::_wmo_doodadset_overrides_ranges>
    >
   > _auto_trait = {};

  };

}
#include <IO/ADT/Obj/ADTObj.inl>
#endif // IO_ADT_OBJ_ADTOBJ_HPP



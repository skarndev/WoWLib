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

  // switch the implementation (obj0 vs obj1)
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

  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  class ADTObj : protected Common::Traits::IOTraits
                            <
                              LodLevelImpl<client_version, lod_level>
                              , Common::Traits::VersionTrait
                                <
                                  ADTLodMapObjectBatches
                                  , client_version
                                  , Common::ClientVersion::BFA
                                >
                              , Common::Traits::VersionTrait
                                <
                                  ADTDoodadsetOverrides
                                  , client_version
                                  , Common::ClientVersion::SL
                                >
                            >
  {
  static_assert(client_version >= Common::ClientVersion::CATA && "Split files did not exist before Cataclysm.");
  public:
    explicit ADTObj(std::uint32_t file_data_id)
        : _file_data_id(file_data_id)
    {
    };

    void Read(Common::ByteBuffer const& buf);

    void Write(Common::ByteBuffer& buf) const;

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
  class ADTObj0ModelStorageFilepath
  {
  public:
    [[nodiscard]]
    bool Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header);

    void Write(Common::ByteBuffer& buf) const;
  protected:
    Common::StringBlockChunk<Common::StringBlockChunkType::OFFSET
                              , ChunkIdentifiers::ADTObj0Chunks::MMDX
                            > _model_filenames;

    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj0Chunks::MMID> _model_filename_offsets;

    Common::StringBlockChunk<Common::StringBlockChunkType::OFFSET
                              , ChunkIdentifiers::ADTObj0Chunks::MWMO
                            > _map_object_filenames;

    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj0Chunks::MWID> _map_object_filename_offsets;
  } IMPLEMENTS_IO_TRAIT(ADTObj0ModelStorageFilepath);

  template<Common::ClientVersion client_version>
  class AdtObj0SpecificData : protected Common::Traits::IOTraits
                                        <
                                          Common::Traits::VersionTrait
                                            <
                                              ADTObj0ModelStorageFilepath
                                              , client_version
                                              , Common::ClientVersion::CATA
                                              , Common::ClientVersion::BFA
                                            >
                                        >
  {
  public:
    AdtObj0SpecificData();

    [[nodiscard]]
    bool Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header, std::uint32_t& chunk_counter);

    void Write(Common::ByteBuffer& buf) const;

  protected:
    Common::DataArrayChunk<DataStructures::MDDF, ChunkIdentifiers::ADTObj0Chunks::MDDF> _model_placements;
    Common::DataArrayChunk<DataStructures::MODF, ChunkIdentifiers::ADTObj0Chunks::MODF> _map_object_placements;
    std::array<MCNKObj, Common::WorldConstants::CHUNKS_PER_TILE> _chunks;
  } IMPLEMENTS_IO_TRAIT(AdtObj0SpecificData<Common::ClientVersion::ANY>);

  // ADT obj-1 specific

  // Enables support for model lod batches in obj1
  class LodModelBatches
  {
  public:
    [[nodiscard]]
    bool Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header);

    void Write(Common::ByteBuffer& buf) const;

  protected:
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObj1Chunks::MLDB> _lod_model_batches;

  } IMPLEMENTS_IO_TRAIT(LodModelBatches);

  template<Common::ClientVersion client_version>
  class AdtObj1SpecificData : protected Common::Traits::IOTraits
                                        <
                                          Common::Traits::VersionTrait
                                            <
                                              LodModelBatches
                                              , client_version
                                              , Common::ClientVersion::SL
                                            >
                                        >
                              , public Common::Traits::AutoIOTraitInterface<AdtObj1SpecificData<client_version>>
  {
  public:
    AdtObj1SpecificData();

    template<Common::ClientVersion client_v>
    void GenerateLod(ADTObj<client_v, ADTObjLodLevel::NORMAL> const& tile_obj);

    [[nodiscard]]
    bool Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header);

    void Write(Common::ByteBuffer& buf) const;

  protected:
    Common::DataArrayChunk<DataStructures::MLMD, ChunkIdentifiers::ADTObj1Chunks::MLMD> _lod_map_object_placements;
    Common::DataArrayChunk<DataStructures::MLMX, ChunkIdentifiers::ADTObj1Chunks::MLMX> _lod_map_object_extents;
    Common::DataArrayChunk<DataStructures::MDDF, ChunkIdentifiers::ADTObj1Chunks::MLDD> _lod_model_placements;
    Common::DataArrayChunk<DataStructures::MLDX, ChunkIdentifiers::ADTObj1Chunks::MLDX> _lod_model_extents;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj1Chunks::MLDL> _lod_model_unknown;
    Common::DataArrayChunk<DataStructures::MLFD, ChunkIdentifiers::ADTObj1Chunks::MLFD> _lod_mapping;

  public:
    static constexpr Common::Traits::AutoIOTrait
      <
        &AdtObj1SpecificData::_lod_map_object_placements,
        &AdtObj1SpecificData::_lod_map_object_extents,
        &AdtObj1SpecificData::_lod_model_placements,
        &AdtObj1SpecificData::_lod_model_extents,
        &AdtObj1SpecificData::_lod_model_unknown,
        &AdtObj1SpecificData::_lod_mapping
      > auto_trait;
  } IMPLEMENTS_IO_TRAIT(AdtObj1SpecificData<Common::ClientVersion::SL>);

  // Enables support for LOD map object batches (BfA+).
  class ADTLodMapObjectBatches : public Common::Traits::AutoIOTraitInterface<ADTLodMapObjectBatches>
  {
  protected:
    Common::DataArrayChunk<char, ChunkIdentifiers::ADTObjCommonChunks::MLMB> _lod_map_object_batches;

  public:
    static constexpr Common::Traits::AutoIOTrait<&ADTLodMapObjectBatches::_lod_map_object_batches> auto_trait;
  } IMPLEMENTS_IO_TRAIT(ADTLodMapObjectBatches);

  class ADTDoodadsetOverrides : public Common::Traits::AutoIOTraitInterface<ADTDoodadsetOverrides>
  {
    template<auto... all_chunks>
    requires (Common::Concepts::ChunkProtocolCommon<Utils::Meta::Traits::TypeOfMemberObject_T<decltype(all_chunks)>>
              && ...)
    friend class Common::Traits::AutoIOTrait;

  protected:
    Common::DataArrayChunk<std::int16_t, ChunkIdentifiers::ADTObjCommonChunks::MWDS> _wmo_doodadset_overrides;
    Common::DataArrayChunk<DataStructures::MWDR
                            , ChunkIdentifiers::ADTObjCommonChunks::MWDR> _wmo_doodadset_overrides_ranges;

  public:
     static constexpr Common::Traits::AutoIOTrait
                      <
                       &ADTDoodadsetOverrides::_wmo_doodadset_overrides
                      , &ADTDoodadsetOverrides::_wmo_doodadset_overrides_ranges
                      > auto_trait;

  } IMPLEMENTS_IO_TRAIT(ADTDoodadsetOverrides);

}
#include <IO/ADT/Obj/ADTObj.inl>
#endif // IO_ADT_OBJ_ADTOBJ_HPP



#ifndef IO_ADT_ROOT_ADTROOTMCNK_HPP
#define IO_ADT_ROOT_ADTROOTMCNK_HPP

#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/WorldConstants.hpp>
#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>

#include <concepts>

namespace IO::ADT
{
  /**
   * Implements a VersionTrait enabling terrain blend batches support for MCNKRoot.
   * Blend batches are responsible for seamlessly blending WMOs with terrain.
   * @tparam ReadContext Any default constructible used as read context.
   * @tparam WriteContext Any default constructible used as write context.
   */
  template<std::default_initializable ReadContext, std::default_initializable WriteContext>
  class MCNKRootBlendBatches : public Common::Traits::AutoIOTraitInterface
                                      <
                                        MCNKRootBlendBatches<ReadContext, WriteContext>
                                        , ReadContext
                                        , WriteContext
                                      >
  {
  protected:
    Common::DataArrayChunk
      <
        DataStructures::MCBB
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCBB
        , Common::FourCCEndian::Little
        , 0
        , 256
      > _blend_batches;

  private:
    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&MCNKRootBlendBatches::_blend_batches>
      >
      , ReadContext
      , WriteContext
    > _auto_trait{};
  };

  template
  <
    Common::ClientVersion client_version
    , std::default_initializable ReadContext
    , std::default_initializable WriteContext
  >
  class MCNKRoot : public Common::Traits::AutoIOTraits
                          <
                            Common::Traits::IOTraits
                            <
                              Common::Traits::IOTrait
                              <
                                Common::Traits::VersionTrait
                                <
                                  MCNKRootBlendBatches<ReadContext, WriteContext>
                                  , client_version
                                  , Common::ClientVersion::MOP
                                >
                              >
                            >
                            , ReadContext
                            , WriteContext
                          >
  {
  public:
    MCNKRoot();

  private:
    DataStructures::SMChunk _header;
    Common::DataArrayChunk
      <
        float
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCVT
        , Common::FourCCEndian::Little
        , Common::WorldConstants::CHUNK_BUF_SIZE
        , Common::WorldConstants::CHUNK_BUF_SIZE
      > _heightmap;

    Common::DataArrayChunk
      <
        Common::DataStructures::CArgb
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCLV
        , Common::FourCCEndian::Little
        , Common::WorldConstants::CHUNK_BUF_SIZE
        , Common::WorldConstants::CHUNK_BUF_SIZE
      > _vertex_lighting;

    Common::DataArrayChunk
      <
        DataStructures::MCCVEntry
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCCV
        , Common::FourCCEndian::Little
        , Common::WorldConstants::CHUNK_BUF_SIZE
        , Common::WorldConstants::CHUNK_BUF_SIZE
      > _vertex_color;

    Common::DataArrayChunk
      <
        DataStructures::MCNREntry
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCNR
        , Common::FourCCEndian::Little
        , Common::WorldConstants::CHUNK_BUF_SIZE
        , Common::WorldConstants::CHUNK_BUF_SIZE
      > _normals;

    Common::DataChunk<DataStructures::MCLQ, ChunkIdentifiers::ADTRootMCNKSubchunks::MCLQ> _tbc_water;
    Common::DataArrayChunk<DataStructures::MCSE, ChunkIdentifiers::ADTRootMCNKSubchunks::MCSE> _sound_emitters;
    Common::DataChunk<std::uint64_t, ChunkIdentifiers::ADTRootMCNKSubchunks::MCDD> _groundeffect_disable;

    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&MCNKRoot::_header>
        , Common::Traits::TraitEntry<&MCNKRoot::_heightmap>
        , Common::Traits::TraitEntry<&MCNKRoot::_vertex_lighting>
        , Common::Traits::TraitEntry<&MCNKRoot::_vertex_color>
        , Common::Traits::TraitEntry<&MCNKRoot::_normals>
        , Common::Traits::TraitEntry<&MCNKRoot::_tbc_water>
        , Common::Traits::TraitEntry<&MCNKRoot::_sound_emitters>
        , Common::Traits::TraitEntry<&MCNKRoot::_groundeffect_disable>
      >
      , ReadContext
      , WriteContext
    > _auto_trait{};

  };
}

#include <IO/ADT/Root/ADTRootMCNK.inl>

#endif // IO_ADT_ROOT_ADTROOTMCNK_HPP
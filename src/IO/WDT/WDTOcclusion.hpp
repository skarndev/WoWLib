#pragma once
#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>
#include <IO/CommonChunkIdentifiers.hpp>
#include <IO/WDT/ChunkIdentifiers.hpp>
#include <IO/WDT/DataStructures.hpp>

namespace IO::WDT
{
  class WDTOcclusion : public Common::Traits::AutoIOTraitInterface<WDTOcclusion, Common::Traits::TraitType::File>
  {
    AutoIOTraitInterfaceUser;

  private:
    Common::DataChunk
    <
      std::uint32_t
      , Common::ChunkIdentifiers::CommonChunks::MVER
    > _version;

    Common::DataArrayChunk
    <
      DataStructures::MapAreaOcclusionIndex
      , WDT::ChunkIdentifiers::WDTOcclusionChunks::MAOI
    > _occlusion_index;

    Common::DataArrayChunk
    <
      DataStructures::MapAreaOcclusionHeightmap
      , WDT::ChunkIdentifiers::WDTOcclusionChunks::MAOH
    > _occlusion_heightmap;

  private:
    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntry<&WDTOcclusion::_version>
      , Common::Traits::TraitEntry<&WDTOcclusion::_occlusion_index>
      , Common::Traits::TraitEntry<&WDTOcclusion::_occlusion_heightmap>
    > _auto_trait {};

  };
}
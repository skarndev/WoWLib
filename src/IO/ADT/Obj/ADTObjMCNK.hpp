#pragma once
#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

#include <cstdint>

namespace IO::ADT
{
  class MCNKObj : public Common::Traits::AutoIOTraitInterface
                         <
                           MCNKObj
                           , Common::Traits::DefaultTraitContext
                           , Common::Traits::DefaultTraitContext
                           , Common::Traits::TraitType::Chunk
                         >
  {
  private:
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj0MCNKSubchunks::MCRD> _model_references;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj0MCNKSubchunks::MCRW> _map_object_references;

    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&MCNKObj::_model_references>
        , Common::Traits::TraitEntry<&MCNKObj::_map_object_references>
      >
    > _auto_traits {};
  };
}
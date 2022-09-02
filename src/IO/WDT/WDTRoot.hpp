#pragma once

#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>
#include <IO/WorldConstants.hpp>
#include <IO/CommonChunkIdentifiers.hpp>
#include <IO/WDT/DataStructures.hpp>
#include <IO/WDT/ChunkIdentifiers.hpp>

namespace IO::WDT
{
  namespace details
  {
    class WDTFilenameBasedComponent : public Common::Traits::AutoIOTraitInterface
                                             <
                                                WDTFilenameBasedComponent
                                                , Common::Traits::TraitType::Component
                                             >
    {
      AutoIOTraitInterfaceUser;
    private:
      Common::StringBlockChunk
      <
        Common::StringBlockChunkType::NORMAL
        , WDT::ChunkIdentifiers::WDTRootChunks::MWMO
        , Common::FourCCEndian::Little
        , 1
        , 1
      > _global_map_object_filename;

      Common::DataChunk
      <
        DataStructures::MapObjectPlacement
        , WDT::ChunkIdentifiers::WDTRootChunks::MODF
      > _global_map_object_placement;

    private:
      static constexpr
      Common::Traits::AutoIOTrait
      <
        Common::Traits::TraitEntry<&WDTFilenameBasedComponent::_global_map_object_filename>
        , Common::Traits::TraitEntry<&WDTFilenameBasedComponent::_global_map_object_placement>
      > _auto_trait {};
    };

    class WDTFiledataIDBasedComponent : public Common::Traits::AutoIOTraitInterface
                                               <
                                                 WDTFiledataIDBasedComponent
                                                 , Common::Traits::TraitType::Component
                                               >
    {
      AutoIOTraitInterfaceUser;
    private:
      Common::DataArrayChunk
      <
        DataStructures::MapAreaID
        , WDT::ChunkIdentifiers::WDTRootChunks::MAID
        , Common::FourCCEndian::Little
        , Common::WorldConstants::MAX_TILES_PER_MAP
        , Common::WorldConstants::MAX_TILES_PER_MAP
      > _map_area_filedataid_index;

      Common::DataChunk
      <
        DataStructures::MapObjectPlacement
        , WDT::ChunkIdentifiers::WDTRootChunks::MODF
      > _global_map_object_placement;

    private:
      static constexpr
      Common::Traits::AutoIOTrait
      <
        Common::Traits::TraitEntry<&WDTFiledataIDBasedComponent::_map_area_filedataid_index>
        , Common::Traits::TraitEntry<&WDTFiledataIDBasedComponent::_global_map_object_placement>
      > _auto_trait {};
    };

  }

  template<Common::ClientVersion client_version>
  class WDTRoot : public Common::Traits::AutoIOTraitInterface<WDTRoot<client_version>, Common::Traits::TraitType::File>
                , public Common::Traits::AutoIOTraits
                         <
                           Common::Traits::IOTrait
                           <
                            Common::Traits::VersionTrait
                            <
                              details::WDTFilenameBasedComponent
                              , client_version
                              , Common::ClientVersion::CLASSIC
                              , Common::ClientVersion::LEGION
                            >
                           >
                           , Common::Traits::IOTrait
                           <
                             Common::Traits::VersionTrait
                               <
                                 details::WDTFiledataIDBasedComponent
                                 , client_version
                                 , Common::ClientVersion::BFA
                               >
                           >
                         >
  {
    AutoIOTraitInterfaceUser;
  private:
    Common::DataChunk
    <
      std::uint32_t
      , Common::ChunkIdentifiers::CommonChunks::MVER
    > _version;

    Common::DataChunk
    <
      DataStructures::MapHeader<client_version>
      , WDT::ChunkIdentifiers::WDTRootChunks::MPHD
    > _map_header;

    Common::DataArrayChunk
    <
      DataStructures::MapAreaInfo<client_version>
      , WDT::ChunkIdentifiers::WDTRootChunks::MAIN
      , Common::FourCCEndian::Little
      , Common::WorldConstants::MAX_TILES_PER_MAP
      , Common::WorldConstants::MAX_TILES_PER_MAP
    > _map_area_index;

  private:

    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntry<&WDTRoot::_version>
      , Common::Traits::TraitEntry<&WDTRoot::_map_header>
      , Common::Traits::TraitEntry<&WDTRoot::_map_area_index>
    > _auto_trait {};
  };
}

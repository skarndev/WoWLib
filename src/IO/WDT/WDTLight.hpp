#pragma once

#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>
#include <IO/CommonChunkIdentifiers.hpp>
#include <IO/WDT/ChunkIdentifiers.hpp>
#include <IO/WDT/DataStructures.hpp>

namespace IO::WDT
{
  namespace details
  {
    class WDTLightUseMPLT : public Common::Traits::AutoIOTraitInterface
                                   <
                                     WDTLightUseMPLT
                                     , Common::Traits::TraitType::Component
                                   >
    {
     AutoIOTraitInterfaceUser;

    protected:
      Common::DataArrayChunk
      <
        WDT::DataStructures::MapPointLightWoD
        , WDT::ChunkIdentifiers::WDTLightChunks::MPLT
      > _point_lights;

    private:
      static constexpr
      Common::Traits::AutoIOTrait
      <
        Common::Traits::TraitEntry<&WDTLightUseMPLT::_point_lights>
      > _auto_trait {};
    };

    class WDTLightUse_MPL2_MSLT_MTEX_MLTA : public Common::Traits::AutoIOTraitInterface
                                                   <
                                                     WDTLightUse_MPL2_MSLT_MTEX_MLTA
                                                     , Common::Traits::TraitType::Component
                                                   >
    {
       AutoIOTraitInterfaceUser;

    protected:
      Common::DataArrayChunk
      <
        WDT::DataStructures::MapPointLightLegion
        , WDT::ChunkIdentifiers::WDTLightChunks::MPL2
      > _point_lights;

      Common::DataArrayChunk
      <
        WDT::DataStructures::MapSpotLight
        , WDT::ChunkIdentifiers::WDTLightChunks::MSLT
      > _spot_lights;

      Common::DataArrayChunk
      <
        std::uint32_t
        , WDT::ChunkIdentifiers::WDTLightChunks::MTEX
      > _textures;

      Common::DataArrayChunk
      <
        WDT::DataStructures::MapLightTextureArrayEntry
        , WDT::ChunkIdentifiers::WDTLightChunks::MLTA
      > _texture_array_entries; ///> TODO: likely incorrect naming here


    private:
      static constexpr
      Common::Traits::AutoIOTrait
      <
        Common::Traits::TraitEntry<&WDTLightUse_MPL2_MSLT_MTEX_MLTA::_point_lights>
        , Common::Traits::TraitEntry<&WDTLightUse_MPL2_MSLT_MTEX_MLTA::_spot_lights>
        , Common::Traits::TraitEntry<&WDTLightUse_MPL2_MSLT_MTEX_MLTA::_textures>
        , Common::Traits::TraitEntry<&WDTLightUse_MPL2_MSLT_MTEX_MLTA::_texture_array_entries>
      > _auto_trait {};

    };
  }

  template<Common::ClientVersion client_version>
  class WDTLight : public Common::Traits::AutoIOTraitInterface
                          <
                            WDTLight<client_version>
                            , Common::Traits::TraitType::File
                          >
                 , public Common::Traits::AutoIOTraits
                          <
                            Common::Traits::IOTrait
                            <
                              Common::Traits::VersionTrait
                              <
                                details::WDTLightUseMPLT
                                , client_version
                                , Common::ClientVersion::WOD
                                , Common::ClientVersion::WOD
                              >
                            >
                            , Common::Traits::IOTrait
                              <
                                Common::Traits::IOTrait
                                <
                                  Common::Traits::VersionTrait
                                  <
                                    details::WDTLightUse_MPL2_MSLT_MTEX_MLTA
                                    , client_version
                                    , Common::ClientVersion::LEGION
                                  >
                                >
                              >
                          >
  {
    AutoIOTraitInterfaceUser;

  private:
    Common::DataChunk<std::uint32_t, Common::ChunkIdentifiers::CommonChunks::MVER> _version;

  private:
    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntry
      <
        &WDTLight::_version
        , Common::Traits::IOHandlerRead<>
        , Common::Traits::IOHandlerWrite
          <
            [](auto* self, auto& ctx, auto& version, Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
            {
              // TODO: validate versions here.
              return true;
            }
          >
      >
    > _auto_trait {};
  };
}
#pragma once

#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/WorldConstants.hpp>
#include <IO/Common.hpp>
#include <IO/ADT/Tex/MCSH.hpp>
#include <IO/ADT/Tex/MCAL.hpp>
#include <Utils/Misc/ForceInline.hpp>

#include <bitset>
#include <cstdint>

namespace IO::ADT
{
  class MCNKTex
  {
  public:

  public:
    MCNKTex() = default;

    void Read(Common::ByteBuffer const& buf
              , std::size_t size
              , MCAL::AlphaFormat alpha_format
              , bool fix_alphamap);

    void  Write(Common::ByteBuffer& buf
                , MCAL::AlphaFormat alpha_format) const;

    [[nodiscard]]
    FORCEINLINE bool IsInitialized() const { return true; };

    void AddShadow() { _shadowmap.Initialize(); }

    private:
      Common::DataArrayChunk
        <
          DataStructures::SMLayer
          , ChunkIdentifiers::ADTTexMCNKSubchunks::MCLY
          , Common::FourCCEndian::LITTLE
          , 0
          , 4
        > _alpha_layers;

      MCSH _shadowmap;
      MCAL _alphamaps;

  // getters
  public:
    [[nodiscard]] FORCEINLINE auto& AlphaLayers() { return _alpha_layers; };
    [[nodiscard]] FORCEINLINE auto const& AlphaLayers() const { return _alpha_layers; };

    [[nodiscard]] FORCEINLINE auto& ShadowMap() { return _shadowmap; };
    [[nodiscard]] FORCEINLINE auto const& ShadowMap() const { return _shadowmap; };

    [[nodiscard]] FORCEINLINE auto& Alphamaps() { return _alphamaps; };
    [[nodiscard]] FORCEINLINE auto const& Alphamaps() const { return _alphamaps; };
  };
}
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

  template<typename T>
  concept MCNKTexReadContext = MCALReadContext<T>;

  template<typename T>
  concept MCNKTexWriteContext = MCALWriteContext<T>;

  template<MCNKTexReadContext ReadContext, MCNKTexWriteContext WriteContext>
  class MCNKTex
  {
  public:

  public:
    MCNKTex() = default;

    void Read(ReadContext& read_ctx, Common::ByteBuffer const& buf, std::size_t size);

    void  Write(WriteContext& write_ctx, Common::ByteBuffer& buf) const;

    [[nodiscard]]
    FORCEINLINE bool IsInitialized() const { return true; };

    void AddShadow() { _shadowmap.Initialize(); }

    private:
      Common::DataArrayChunk
      <
        DataStructures::SMLayer
        , ChunkIdentifiers::ADTTexMCNKSubchunks::MCLY
        , Common::FourCCEndian::Little
        , 0
        , 4
      > _alpha_layers;

      MCSH _shadowmap;
      MCAL<ReadContext, WriteContext> _alphamaps;

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
#pragma once
#include <IO/Common.hpp>
#include <IO/ByteBuffer.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

#include <cstdint>
#include <vector>
#include <array>

namespace IO::ADT
{
  class MCAL
  {
    using Alphamap = std::array<std::uint8_t, 64 * 64>;
    using Alphamaps = std::vector<Alphamap>;

  public:

    enum class AlphaFormat
    {
      LOWRES = 0,
      HIGHRES = 1
    };

    enum class AlphaCompression
    {
      UNCOMPRESSED = 0,
      COMPRESSED = 1
    };

    void Read(Common::ByteBuffer const& buf
              , std::size_t size
              , AlphaFormat format
              , Common::DataArrayChunk
                <
                  DataStructures::SMLayer
                  , ChunkIdentifiers::ADTTexMCNKSubchunks::MCLY
                  , Common::FourCCEndian::LITTLE
                  , 0
                  , 4
                > const& alpha_layer_params
              , bool fix_alpha);

    void Write(Common::ByteBuffer& buf
               , AlphaFormat format
               , Common::DataArrayChunk
                  <
                      DataStructures::SMLayer
                      , ChunkIdentifiers::ADTTexMCNKSubchunks::MCLY
                      , Common::FourCCEndian::LITTLE
                      , 0
                      , 4
                  > const& alpha_layer_params) const;

    // access interface
    Alphamap& Add();

    [[nodiscard]]
    Alphamap& At(std::uint8_t index);

    [[nodiscard]]
    Alphamap const& At(std::uint8_t index) const;

    void Clear() { _alphamap_layers.clear(); };

    void Remove(std::uint8_t index);

    // iterators
    [[nodiscard]]
    Alphamaps::iterator begin() { return _alphamap_layers.begin(); };

    [[nodiscard]]
    Alphamaps::iterator end() { return _alphamap_layers.end(); };

    [[nodiscard]]
    Alphamaps::const_iterator begin() const { return _alphamap_layers.cbegin(); };

    [[nodiscard]]
    Alphamaps::const_iterator end() const { return _alphamap_layers.cend(); };

    [[nodiscard]]
    Alphamaps::const_iterator cbegin() const { return _alphamap_layers.cbegin(); };

    [[nodiscard]]
    Alphamaps ::const_iterator cend() const { return _alphamap_layers.cend(); };

    [[nodiscard]]
    Alphamap& operator[](std::size_t index);

    [[nodiscard]]
    Alphamap const& operator[](std::size_t index) const;

    [[nodiscard]]
    FORCEINLINE bool IsInitialized() const { return true; };


  private:
    static std::uint8_t NormalizeLowresAlpha(std::uint8_t alpha)
    {
      return alpha / 255 + (alpha % 255 <= 127 ? 0 : 1);
    };

    static std::uint8_t NormalizeHighresAlpha(std::uint32_t alpha, std::uint32_t div)
    {
      return alpha / div + (alpha % div <= (div >> 1) ? 0 : 1);
    };

    Alphamaps _alphamap_layers;
  };
}

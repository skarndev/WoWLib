#pragma once
#include <IO/Common.hpp>
#include <IO/ByteBuffer.hpp>

#include <cstdint>
#include <vector>
#include <array>

namespace IO::ADT
{
  class MCAL
  {
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
              , std::uint8_t n_layers
              , AlphaFormat format
              , AlphaCompression compression
              , bool fix_alpha);

    void Write(Common::ByteBuffer& buf, AlphaFormat format, AlphaCompression compression) const;

    static std::uint8_t NormalizeLowresAlpha(std::uint8_t alpha)
    {
      return alpha / 255 + (alpha % 255 <= 127 ? 0 : 1);
    };

    static std::uint8_t NormalizeHighresAlpha(std::uint32_t alpha, std::uint32_t div)
    {
      return alpha / div + (alpha % div <= (div >> 1) ? 0 : 1);
    };
  private:
      std::vector<std::array<std::uint8_t, 64 * 64>> _alphamap_layers;
  };
}

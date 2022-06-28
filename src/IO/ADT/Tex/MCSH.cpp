#include <IO/ADT/Tex/MCSH.hpp>
#include <Validation/Log.hpp>

#include <array>

using namespace IO::ADT;

constexpr unsigned N_BYTES_PER_SHADOWMAP = IO::Common::WorldConstants::N_PIXELS_PER_SHADOWMAP / 8;

MCSH::MCSH()
: _shadowmap(0)
{
}

void MCSH::Read(Common::ByteBuffer const& buf, std::size_t size, bool fix_last_row_col)
{
  LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCSH, size: %d", size);

  std::array<unsigned char, N_BYTES_PER_SHADOWMAP> shadowmap_bytes{};
  buf.Read(shadowmap_bytes.begin(), shadowmap_bytes.end());

  for (std::size_t i = 0; i < N_BYTES_PER_SHADOWMAP; ++i)
  {
    std::bitset<8> cur_byte {shadowmap_bytes[i]};

    for (std::size_t j = 0; j < 8; ++j)
    {
      _shadowmap[i * 8 + j] = cur_byte[j];
    }
  }

  if (fix_last_row_col)
  {
    constexpr std::size_t last_pixel = Common::WorldConstants::SHADOWMAP_DIM - 1;
    constexpr std::size_t pre_last_pixel = last_pixel - 1;

    for (std::size_t i = 0; i < Common::WorldConstants::SHADOWMAP_DIM; ++i)
    {
      _shadowmap[last_pixel * Common::WorldConstants::SHADOWMAP_DIM + i]
        = _shadowmap[pre_last_pixel * Common::WorldConstants::SHADOWMAP_DIM + i];
      _shadowmap[i * Common::WorldConstants::SHADOWMAP_DIM + last_pixel]
        = _shadowmap[i * Common::WorldConstants::SHADOWMAP_DIM + pre_last_pixel];
    }
    _shadowmap[last_pixel * Common::WorldConstants::SHADOWMAP_DIM + last_pixel]
      = _shadowmap[pre_last_pixel * Common::WorldConstants::SHADOWMAP_DIM + pre_last_pixel];
  }
}

void MCSH::Write(Common::ByteBuffer& buf) const
{
  LogDebugF(LCodeZones::FILE_IO, "Writing chunk: MCSH");

  for (std::size_t i = 0; i < N_BYTES_PER_SHADOWMAP; ++i)
  {
    std::bitset<8> cur_byte {0};

    for (std::size_t j = 0; j < 8; ++j)
    {
      cur_byte[j] = _shadowmap[i * 8 + j];
    }

    buf.Write(static_cast<std::uint8_t>(cur_byte.to_ulong()));
  }

}
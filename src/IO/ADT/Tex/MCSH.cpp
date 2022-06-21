#include <IO/ADT/Tex/MCSH.hpp>
#include <Validation/Log.hpp>

#include <array>

using namespace IO::ADT;

MCSH::MCSH()
: _shadowmap(0)
{
}

void MCSH::Read(Common::ByteBuffer const& buf, std::size_t size, bool fix_last_row_col)
{
  LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCSH, size: %d", size);

  std::array<unsigned char, 64 * 64 / 8> shadowmap_bytes{};
  buf.Read(shadowmap_bytes.begin(), shadowmap_bytes.end());

  for (std::size_t i = 0; i < 64 * 64 / 8; ++i)
  {
    std::bitset<8> cur_byte {shadowmap_bytes[i]};

    for (std::size_t j = 0; j < 8; ++j)
    {
      _shadowmap[i * 8 + j] = cur_byte[j];
    }
  }

  if (fix_last_row_col)
  {
    // "fix" last row
    for (std::size_t i = 0; i < 64; ++i)
    {
      _shadowmap[63 * 64 + i] = _shadowmap[62 * 64 + i];
    }

    // "fix" last column
    for (std::size_t i = 0; i < 64; ++i)
    {
      _shadowmap[i * 64 + 63] = _shadowmap[i * 64 + 62];
    }
  }
}

void MCSH::Write(Common::ByteBuffer& buf)
{
  LogDebugF(LCodeZones::FILE_IO, "Writing chunk: MCSH");

  for (std::size_t i = 0; i < 64 * 64 / 8; ++i)
  {
    std::bitset<8> cur_byte {0};

    for (std::size_t j = 0; j < 8; ++j)
    {
      cur_byte[j] = _shadowmap[i * 8 + j];
    }

    buf.Write(static_cast<std::uint8_t>(cur_byte.to_ulong()));
  }

}
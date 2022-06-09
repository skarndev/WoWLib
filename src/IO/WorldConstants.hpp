#pragma once

namespace IO::Common::WorldConstants
{
  inline constexpr float TILESIZE = 533.33333f;
  inline constexpr float CHUNKSIZE = ((TILESIZE) / 16.0f);
  inline constexpr float UNITSIZE = (CHUNKSIZE / 8.0f);
  inline constexpr float MINICHUNKSIZE = (CHUNKSIZE / 4.0f);
  inline constexpr float TEXDETAILSIZE = (CHUNKSIZE / 64.0f);
  inline constexpr float ZEROPOINT = (32.0f * (TILESIZE));
  inline constexpr double MAPCHUNK_RADIUS = 47.140452079103168293389624140323; //sqrt((533.33333/16)^2 + (533.33333/16)^2)
  inline constexpr unsigned CHUNKBUFSIZE = 9 * 9 + 8 * 8;
}
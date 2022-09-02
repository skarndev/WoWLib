#pragma once
#include <IO/Common.hpp>
#include <cstdint>

namespace IO::Common::ChunkIdentifiers
{
  /**
   * Common chunks shared by multiple WoW files.
   */
  namespace CommonChunks
  {
    enum eCommonChunks : std::uint32_t
    {
      MVER = IO::Common::FourCC<"MVER"> ///> Version chunk representing often meaningless versions of WoW files.
    };
  }
}
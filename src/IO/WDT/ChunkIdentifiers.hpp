#pragma once
#include <Utils/Meta/Templates.hpp>
#include <IO/Common.hpp>

namespace IO::WDT::ChunkIdentifiers
{
  /**
   * Chunks found in WDT root files.
   */
  namespace WDTRootChunks
  {
    enum eWDTRootChunks
    {
      MPHD = Common::FourCC<"MPHD">, ///> Map Header. Contains global flags and FileDataID references in post-BfA clients.
      MAIN = Common::FourCC<"MAIN">, ///> Map Area Index. Responsible for identifying tiles present on map.
      MODF = Common::FourCC<"MODF">, ///> Map Object Definition. WMO placement information for global WMO maps.

      // pre BfA
      MWMO = Common::FourCC<"MWMO">, ///> Map World Map Object. Contains WMO filename for global WMO maps.

      // post BfA
      MAID = Common::FourCC<"MAID">, ///> Map Area ID. Contains FileDataID references of various map files.

      // post SL
      MANM = Common::FourCC<"MANM">, ///> Map Anima. Anima-specific chunk for Shadowlands expansion.
    };
  }

  /**
   * Chunks found in WDT occ (occlusion) files.
   */
  namespace WDTOcclusionChunks
  {
    enum eWDTOcclusionChunks
    {
      MAOI = Common::FourCC<"MAOI">, // Map Area Occlusion Index. Index structures into MAOH.
      MAOH = Common::FourCC<"MAOH"> // Map Area Occlusion Height. Heightmap data for terrain occluders.
    };
  }

  namespace WDTLightChunks
  {
    enum eWDTLightChunks
    {
      MPLT = Common::FourCC<"MPLT">, ///> Map Point Lights (WOD).
      MPL2 = Common::FourCC<"MPL2">, ///> Map Point Lights (Legion+).
      MPL3 = Common::FourCC<"MPL3">, ///> Map Point Lights (SL+).
      MSLT = Common::FourCC<"MSLT">, ///> Map Spot Lights (Legion+).
      MTEX = Common::FourCC<"MTEX">, ///> Map textures (Legion+).
      MLTA = Common::FourCC<"MLTA">, ///> TODO: Map light areas? (Legion+).
    };
  }
}
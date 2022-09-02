#pragma once

#include <IO/Common.hpp>
#include <IO/CommonDataStructures.hpp>
#include <IO/WorldConstants.hpp>

#include <cstdint>
#include <array>

namespace IO::WDT::DataStructures
{
  /**
   * Enum-like structure holding MPHD's flags values.
   * @tparam client_version Version of game client.
   */
  template<Common::ClientVersion client_version>
  struct MapHeaderFlags;

  template<Common::ClientVersion client_version>
  requires (client_version == Common::ClientVersion::ANY)
  struct MapHeaderFlags<client_version> : public Utils::Meta::Templates::VersionedEnum<client_version, std::uint32_t>
  {};

  template<Common::ClientVersion client_version>
  requires (client_version < Common::ClientVersion::WOTLK)
  struct MapHeaderFlags<client_version> : public MapHeaderFlags<Common::ClientVersion::ANY>
  {
    enum : std::uint32_t
    {
      WDTUsesGlobalMapObj = 0x1 ///> Use global WMO on the map instead of ADT files.
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version == Common::ClientVersion::WOTLK)
  struct MapHeaderFlags<client_version> : public MapHeaderFlags<Common::ClientVersion::TBC>
  {
    enum : std::uint32_t
    {
      SupportsVertexColor = 0x2, ///> Enables support for vertex coloring (MCCV).
      UseHighresAlphamap = 0x4, ///> Enables support for highres alphamaps (aka big alpha).
      ModelsSortedBySizeCategory = 0x8 ///> Turns of optimizations asssuming M2 model placements are pre-sorted by size category.
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version == Common::ClientVersion::CATA)
  struct MapHeaderFlags<client_version> : public MapHeaderFlags<Common::ClientVersion::WOTLK>
  {
    enum : std::uint32_t
    {
      SupportsVertexLighting = 0x10, ///> Enables support for vertex lighting MCLV. TODO: deprecated since 8.x?
      HasUpsidedownGround = 0x20 ///> Inverts the ground to create a ceiling. TODO: find if used and works in the game.
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version >= Common::ClientVersion::MOP && client_version < Common::ClientVersion::LEGION)
  struct MapHeaderFlags<client_version> : public MapHeaderFlags<Common::ClientVersion::CATA>
  {
    enum : std::uint32_t
    {
      Unknown_0x40 = 0x40, ///> Firelands2.wdt has this enabled since MoP until Legion.
      SupportsHeightTextureBlending = 0x80 ///> Enables height based texture blending (_h.blp textures / MTXP).
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version == Common::ClientVersion::LEGION)
  struct MapHeaderFlags<client_version> : public MapHeaderFlags<Common::ClientVersion::MOP>
  {
    enum : std::uint32_t
    {
      UnknownLodRelatedImplicitSet0x8000_0x100 = 0x100, ///> Implicitly set 0x8000.
      UnknownLodRelated_0x8000 = 0x8000 ///> If set, map textures and LOD are required. TODO: Name UseLod?
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version >= Common::ClientVersion::BFA)
  struct MapHeaderFlags<client_version> : public MapHeaderFlags<Common::ClientVersion::LEGION>
  {
    enum : std::uint32_t
    {
      LodADTByFileDataID = 0x200, ///> Enables loading ADTs by FileDataID instead of filenames. Requires MAID chunk to work.
      Unknown_0x400 = 0x400,
      Unknown_0x800 = 0x800,
      Unknown_0x1000 = 0x1000,
      Unknown_0x2000 = 0x2000,
      Unknown_0x4000 = 0x4000,
      Unknown_0x8000 = 0x8000,
    };
  };

  /**
   * Header a of WDT main file. FileDataID fields
   * @tparam client_version Version of game client.
   */
  template<Common::ClientVersion client_version>
  struct MapHeader;

  template<Common::ClientVersion client_version>
  requires (client_version < Common::ClientVersion::BFA)
  struct MapHeader<client_version>
  {
    MapHeaderFlags<client_version> flags;
    uint32_t unknown;
    uint32_t pad[6];
  };

  template<Common::ClientVersion client_version>
  requires (client_version >= Common::ClientVersion::BFA)
  struct MapHeader<client_version>
  {
    MapHeaderFlags<client_version> flags;
    uint32_t lgt_file_data_id;
    uint32_t occ_file_data_id;
    uint32_t fogs_file_data_id;
    uint32_t mpv_file_data_id;
    uint32_t tex_file_data_id;
    uint32_t wdl_file_data_id;
    uint32_t pd4_file_data_id;
  };

  template<Common::ClientVersion client_version>
  struct MapAreaInfoFlags;

  template<Common::ClientVersion client_version>
  requires (client_version == Common::ClientVersion::ANY)
  struct MapAreaInfoFlags<client_version> : public Utils::Meta::Templates::VersionedEnum<client_version, std::uint32_t>
  {};

  template<Common::ClientVersion client_version>
  requires (client_version <= Common::ClientVersion::WOTLK)
  struct MapAreaInfoFlags<client_version> : public MapHeaderFlags<Common::ClientVersion::ANY>
  {
    enum : std::uint32_t
    {
      TileExists = 0x1, ///> Indicates that ADT for this tile exists.
      TileLoaded = 0x2 ///> Tile loaded. TODO: Runtime-only flag? Useless in the files?
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version > Common::ClientVersion::WOTLK)
  struct MapAreaInfoFlags<client_version> : public MapHeaderFlags<Common::ClientVersion::ANY>
  {
    enum : std::uint32_t
    {
      TileExists = 0x1, ///> Indicates that ADT for this tile exists.
      AllWater = 0x2, ///> Tile is entirely covered by ocean. Produces "fake" ocean.
      TileLoaded = 0x4 ///> Tile loaded. TODO: Runtime-only flag? Useless in the files?
    };
  };

  /**
   * Structured used in MAIN chunk to indicate presence and behavior a tile.
   * @tparam client_version Version of game client.
   */
  template<Common::ClientVersion client_version>
  struct MapAreaInfo
  {
    MapAreaInfoFlags<client_version> flags; ///> Flags defining the presence and behavior of a tile.
    std::uint32_t async_id; ///> Only used during runtime. Safe to keep uninitialized in the files.
  };

  /**
   * Structure used in MAID chunk to indicate FileDataIDs of corresponding map components.
   */
  struct MapAreaID
  {
    uint32_t root_adt; ///> reference to FileDataID of mapname_xx_yy.adt.
    uint32_t obj0_adt; ///> reference to FileDataID of mapname_xx_yy_obj0.adt.
    uint32_t obj1_adt; ///> reference to FileDataID of mapname_xx_yy_obj1.adt.
    uint32_t tex0_adt; ///> reference to FileDataID of mapname_xx_yy_tex0.adt.
    uint32_t lod_adtT;  ///> reference to FileDataID of mapname_xx_yy_lod.adt.
    uint32_t map_texture; ///> reference to FileDataID of mapname_xx_yy.blp.
    uint32_t map_texture_n; ///> reference to FileDataID of mapname_xx_yy_n.blp.
    uint32_t minimap_texture; ///> reference to FileDataID of mapxx_yy.blp.
  };

  /**
   * Structure used in MODF chunk to define global placement of a WMO.
   */
  struct MapObjectPlacement
  {
    std::uint32_t name_id; ///> Unused, MWMO content is used instead. TODO: FileDataID in newer clients?
    std::uint32_t unique_id; ///> Unique ID of spawned WMO. TODO: Value is reported to be actually unused.
    Common::DataStructures::C3Vector position; ///> Position of the WMO placement.
    Common::DataStructures::C3Vector rotation; ///> Euler rotation of the WMO placement.
    Common::DataStructures::CAaBox extents; ///> Bounding box of the WMO placement.
    std::uint16_t flags; ///> TODO: which flags are these? ADT's MODF?
    std::uint16_t doodad_set; ///> Doodad set index.
    std::uint16_t name_set; ///> TODO: what is that?
    std::uint16_t _pad; ///> TODO: perhaps scale since Legion?
  };

  /**
   * Index structure pointing to entries of data ina MAOH (MapAreaOcclusionHeightmap).
   */
  struct MapAreaOcclusionIndex
  {
    Common::DataStructures::TileIndex tile_index; ///> Tile coordinates on WDT grid.
    std::uint32_t offset; ///> Offset of heightmap data in MAOH.
    std::uint32_t size; ///> Data size. Always (17*17+16*16) * sizeof(std::uint16_t)
  };

  /**
   * Entry of MAOH. Defines a heightmap that occludes everything behind it. Same content as WDL::MARE.
   */
  struct MapAreaOcclusionHeightmap
  {
    std::array<std::uint16_t, Common::WorldConstants::MAP_AREA_OCCLUSION_HEIGHTMAP_SIZE> interleaved_heightmap;
  };

  /**
   * Point light used in WDT lgt in WoD (MPLT chunk). No longer parsed since Legion.
   */
  struct MapPointLightWoD
  {
    std::uint32_t id; ///> TODO: unique ID of light?
    Common::DataStructures::TileIndex tile_index;  ///> Tile coordinates on WDT grid.
    Common::DataStructures::CArgb color; ///> Color of light.
    Common::DataStructures::C3Vector position; ///> Position of light source.
    std::array<float, 3> unknown; ///> TODO: intensity? flicker?
  };

  /**
   * Point light used in WDT lgt since Legion.
   */
  struct MapPointLightLegion
  {
    std::uint32_t id; ///> TODO: unique ID of light?
    Common::DataStructures::CArgb color; ///> Color of light.
    Common::DataStructures::C3Vector position; ///> Position of light source.
    std::array<float, 3> unknown; ///> TODO: intensity? flicker? Same as WoD version.
    std::array<float, 3> unknown_1; ///> TODO: what is it?
    Common::DataStructures::TileIndex tile_index;  ///> Tile coordinates on WDT grid.
    std::array<std::int16_t, 2> unknown_2; ///> TODO: what is it? Seen values of [0,-1] and [-1,-1].
  };

  /**
   * Spot light used in WDT lgt since Legion.
   */
  struct MapSpotLight
  {
    std::uint32_t id; ///> TODO: unique ID of light?
    Common::DataStructures::CArgb color; ///> Color of light.
    Common::DataStructures::C3Vector position; ///> Position of light source.
    Common::DataStructures::CRange attenuation; ///> Start and end of attenuation range.
    float intensity; ///> Intensity of light? TODO: range 0 to 1?
    Common::DataStructures::C3Vector rotation; ///> Rotation in radians of the spot light source.
    float falloff_exponent; ///> Fallof exponent? TODO: better desc.
    float inner_radius; ///> Inner radius of the spot light source.
    Common::DataStructures::TileIndex tile_index;  ///> Tile coordinates on WDT grid.
    std::uint32_t unk_or_mlta_id; ///> TODO: MLTA id? int16_t[2]?.
  };

  /**
   * TODO: MLTA, come up with a better name
   */
  struct MapLightTextureArrayEntry
  {
    float unk; ///> TODO: what is it?
    float unk1; ///> TODO: what is it?
    std::uint32_t unk_or_mtex_idx; ///> TODO: MTEX index? Seen values: 1, 2.
  };
}
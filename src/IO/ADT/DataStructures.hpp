#include <IO/Common/CommonDataStructures.hpp>
#include <cstdint>

namespace IO::ADT::DataStructures
{
  struct MVER
  {
    std::uint32_t version;
  };

  namespace MHDRFlags
  {
    enum eMHDFlags
    {
      mhdr_MFBO = 1,                // contains a MFBO chunk.
      mhdr_northrend = 2,           // is set for some northrend ones.
    };
  }

  struct MHDR
  {
    std::uint32_t flags;
    std::uintptr_t mcin;                     // Cata+: obviously gone. probably all offsets gone, except mh2o(which remains in root file).
    std::uintptr_t mtex;
    std::uintptr_t mmdx;
    std::uintptr_t mmid;
    std::uintptr_t mwmo;
    std::uintptr_t mwid;
    std::uintptr_t mddf;
    std::uintptr_t modf;
    std::uintptr_t mfbo;                     // this is only set if flags & mhdr_MFBO.
    std::uintptr_t mh2o;
    std::uintptr_t mtxf;
    std::uint8_t mamp_value;             // Cata+, explicit MAMP chunk overrides data
    std::uint8_t padding[3];
    std::uint32_t unused[3];
  };
  
  namespace MDDFFlags
  {
    enum eMDDFFlags
    {
      mddf_biodome = 1,                     // this sets internal flags to | 0x800 (WDOODADDEF.var0xC).
      mddf_shrubbery = 2,                   // the actual meaning of these is unknown to me. maybe biodome is for really big M2s. 6.0.1.18179 seems 
                                            // not to check  for this flag
      mddf_unk_4 = 0x4,                     // Legion+ᵘ
      mddf_unk_8 = 0x8,                     // Legion+ᵘ
      SMDoodadDef::Flag_liquidKnown = 0x20, // Legion+ᵘ
      mddf_entry_is_filedata_id = 0x40,     // Legion+ᵘ nameId is a file data id to directly load
      mddf_unk_100 = 0x100,                 // Legion+ᵘ
    };
  }

  struct MDDF
  {
    /*0x00*/  std::uint32_t nameId;              // references an entry in the MMID chunk, specifying the model to use.
                                            // if flag mddf_entry_is_filedata_id is set, a file data id instead, ignoring MMID.
    /*0x04*/  std::uint32_t uniqueId;            // this ID should be unique for all ADTs currently loaded. Best, they are unique for the whole map. Blizzard has 
                                            // these unique for the whole game.
    /*0x08*/  Common::DataStructures::C3Vector position;           // This is relative to a corner of the map. Subtract 17066 from the non vertical values and you should start to see 
                                            // something that makes sense. You'll then likely have to negate one of the non vertical values in whatever 
                                            // coordinate system you're using to finally move it into place.
    /*0x14*/  Common::DataStructures::C3Vector rotation;           // degrees. This is not the same coordinate system orientation like the ADT itself! (see history.)
    /*0x20*/  std::uint16_t scale;               // 1024 is the default size equaling 1.0f.
    /*0x22*/  std::uint16_t flags;               // values from enum MDDFFlags.
    /*0x24*/
  };

  namespace MODFFlags
  {
    enum eMODFFlags 
    {
      modf_destroyable = 0x1,         // set for destroyable buildings like the tower in DeathknightStart. This makes it a server-controllable game object.
      modf_use_lod = 0x2,             // WoD(?)+: also load _LOD1.WMO for use dependent on distance
      modf_unk_has_scale = 0x4,       // Legion+: if this flag is set then use scale = scale / 1024, otherwise scale is 1.0
      modf_entry_is_filedata_id = 0x8, // Legion+: nameId is a file data id to directly load //SMMapObjDef::FLAG_FILEDATAID
      modf_use_sets_from_mwds = 0x80  // Shadowlands+: if set, doodad set indexes of which to load should be taken from MWDS chunk
    };
  }

  struct MODF
  {
    /*0x00*/  std::uint32_t nameId;              // references an entry in the MWID chunk, specifying the model to use.
    /*0x04*/  std::uint32_t uniqueId;            // this ID should be unique for all ADTs currently loaded. Best, they are unique for the whole map.
    /*0x08*/  Common::DataStructures::C3Vector position;
    /*0x14*/  Common::DataStructures::C3Vector rotation;           // same as in MDDF.
    /*0x20*/  Common::DataStructures::CAaBox extents;              // position plus the transformed wmo bounding box. used for defining if they are rendered as well as collision.
    /*0x38*/  std::uint16_t flags;               // values from enum MODFFlags.
    /*0x3A*/  std::uint16_t doodadSet;           // which WMO doodad set is used. Traditionally references WMO#MODS_chunk, if modf_use_sets_from_mwds is set, references #MWDR_.28Shadowlands.2B.29
    /*0x3C*/  std::uint16_t nameSet;             // which WMO name set is used. Used for renaming goldshire inn to northshire inn while using the same model.
    /*0x3E*/  std::uint16_t scale;               // Legion+: scale, 1024 means 1 (same as MDDF). Padding in 0.5.3 alpha. 
    /*0x40*/
  };

  struct SMLiquidChunk 
  {
    std::uint32_t offset_instances;       // points to SMLiquidInstance[layer_count]
    std::uint32_t layer_count;            // 0 if the chunk has no liquids. If > 1, the offsets will point to arrays.
    std::uint32_t offset_attributes;      // points to mh2o_chunk_attributes, can be ommitted for all-0
  };

  struct MH20ChunkAttributes 
  {
    std::uint64_t fishable;               // seems to be usable as visibility information.
    std::uint64_t deep;                   // TC: treat as fatigue area if bit set
  };

  struct SMLiquidInstance 
  {
    std::uint16_t liquid_type;
    std::uint16_t liquid_object_or_lvf;        // if >= 42, look up via LiquidObjectRec::LiquidTypeID => LiquidTypeRec::MaterialID => LiquidMaterialRec::LVF, otherwise LVF
    float min_height_level;          // used as height if no heightmap given and cullingᵘ
    float max_height_level;          // ≥ WoD ignores value and assumes to both be 0.0 for LVF = 2!ᵘ
    std::uint8_t x_offset;                // The X offset of the liquid square (0-7)
    std::uint8_t y_offset;                // The Y offset of the liquid square (0-7)
    std::uint8_t width;                   // The width of the liquid square (1-8)
    std::uint8_t height;                  // The height of the liquid square (1-8)
                                     // The above four members are only used if liquid_object_or_lvf <= 41. Otherwise they are assumed 0, 0, 8, 8. (18179) 
    std::uint32_t offset_exists_bitmap;   // not all tiles in the instances need to be filled. always (width * height + 7) / 8 bytes.
                                     // offset can be 0 for all-exist. also see (and extend) Talk:ADT/v18#SMLiquidInstance
    std::uint32_t offset_vertex_data;     // actual data format defined by LiquidMaterialRec::m_LVF via LiquidTypeRec::m_materialID
                                     // if offset = 0 and liquidType ≠ 2, then let LVF = 2, i.e. some ocean shit
  };

  struct MH20UVMapEntry
  {
    std::uint16_t x;                      // divided by 8 for shaders
    std::uint16_t y;
  };

  namespace SMChunkFlags
  {

  }
  
  struct SMChunk
  {

    /*0x004*/  uint32_t IndexX;
    /*0x008*/  uint32_t IndexY;
#if version < ?
    float radius;
#endif
    /*0x00C*/  uint32_t nLayers;                              // maximum 4
    /*0x010*/  uint32_t nDoodadRefs;
#if version >= ~5.3
    uint64_t holes_high_res;                                // only used with flags.high_res_holes
#else
    /*0x014*/  uint32_t ofsHeight;
    /*0x018*/  uint32_t ofsNormal;
#endif
    /*0x01C*/  uint32_t ofsLayer;
    /*0x020*/  uint32_t ofsRefs;
    /*0x024*/  uint32_t ofsAlpha;
    /*0x028*/  uint32_t sizeAlpha;
    /*0x02C*/  uint32_t ofsShadow;                            // only with flags.has_mcsh
    /*0x030*/  uint32_t sizeShadow;
    /*0x034*/  uint32_t areaid;                              // in alpha: both zone id and sub zone id, as uint16s.
    /*0x038*/  uint32_t nMapObjRefs;
    /*0x03C*/  uint16_t holes_low_res;
    /*0x03E*/  uint16_t unknown_but_used;                    // in alpha: padding
    /*0x040*/  uint2_t[8][8] ReallyLowQualityTextureingMap;  // "predTex", It is used to determine which detail doodads to show. Values are an array of two bit 
                                                             // unsigned integers, naming the layer.
    /*0x050*/  uint1_t[8][8] noEffectDoodad;                 // doodads disabled if 1; WoD: may be an explicit MCDD chunk
    /*0x058*/  uint32_t ofsSndEmitters;
    /*0x05C*/  uint32_t nSndEmitters;                        // will be set to 0 in the client if ofsSndEmitters doesn't point to MCSE!
    /*0x060*/  uint32_t ofsLiquid;
    /*0x064*/  uint32_t sizeLiquid;                          // 8 when not used; only read if >8.

    // in alpha, remainder is padding but unused.

    /*0x068*/  C3Vectorⁱ position;
    /*0x074*/  uint32_t ofsMCCV;                             // only with flags.has_mccv, had uint32_t textureId; in ObscuR's structure.
    /*0x078*/  uint32_t ofsMCLV;                             // introduced in Cataclysm
    /*0x07C*/  uint32_t unused;                              // currently unused
    /*0x080*/
  };

}
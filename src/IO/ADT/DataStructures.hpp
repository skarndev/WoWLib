#pragma once
#include <IO/CommonDataStructures.hpp>
#include <cstdint>
#include <type_traits>


namespace IO::ADT::DataStructures
{

#pragma pack(push, 1)

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
    std::uintptr_t mcin_unused;                     // Cata+: obviously gone. probably all offsets gone, except mh2o(which remains in root file).
    std::uintptr_t mtex_unused;
    std::uintptr_t mmdx_unused;
    std::uintptr_t mmid_unused;
    std::uintptr_t mwmo_unused;
    std::uintptr_t mwid_unused;
    std::uintptr_t mddf_unused;
    std::uintptr_t modf_unused;
    std::uintptr_t mfbo;                     // this is only set if flags & mhdr_MFBO.
    std::uintptr_t mh2o;
    std::uintptr_t mtxf_unused;
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
      flag_liquidKnown = 0x20, // Legion+ᵘ
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

  struct SMChunkFlags
  {
    std::uint32_t has_mcsh : 1;
    std::uint32_t impass : 1;
    std::uint32_t lq_river : 1;
    std::uint32_t lq_ocean : 1;
    std::uint32_t lq_magma : 1;
    std::uint32_t lq_slime : 1;
    std::uint32_t has_mccv : 1;
    std::uint32_t unknown_0x80 : 1;
    std::uint32_t : 7;                                         // not set in 6.2.0.20338
    std::uint32_t do_not_fix_alpha_map : 1;                    // "fix" alpha maps in MCAL and MCSH (4 bit alpha maps are 63*63 instead of 64*64).
                                                          // If this flag is not set, the MCAL format *has* to be unfixed4444, otherwise UnpackAlphaShadowBits will assert.
    std::uint32_t high_res_holes : 1;                          // Since ~5.3 WoW uses full 64-bit to store holes for each tile if this flag is set.
    std::uint32_t : 15;                                        // not set in 6.2.0.20338
  };
  
  struct SMChunk
  {
               SMChunkFlags flags;
    /*0x004*/  std::uint32_t IndexX;
    /*0x008*/  std::uint32_t IndexY;
    /*0x00C*/  std::uint32_t nLayers;                              // maximum 4
    /*0x010*/  std::uint32_t nDoodadRefs;
               std::uint64_t holes_high_res;                                // only used with flags.high_res_holes

    /*0x01C*/  std::uint32_t ofsLayer;
    /*0x020*/  std::uint32_t ofsRefs;
    /*0x024*/  std::uint32_t ofsAlpha;
    /*0x028*/  std::uint32_t sizeAlpha;
    /*0x02C*/  std::uint32_t ofsShadow;                            // only with flags.has_mcsh
    /*0x030*/  std::uint32_t sizeShadow;
    /*0x034*/  std::uint32_t areaid;                              // in alpha: both zone id and sub zone id, as uint16s.
    /*0x038*/  std::uint32_t nMapObjRefs;
    /*0x03C*/  std::uint16_t holes_low_res;
    /*0x03E*/  std::uint16_t unknown_but_used;                    // in alpha: padding
               std::uint16_t doodadMapping[8];
               std::uint8_t doodadStencil[8];
    // doodads disabled if 1; WoD: may be an explicit MCDD chunk
    /*0x058*/  std::uint32_t ofsSndEmitters;
    /*0x05C*/  std::uint32_t nSndEmitters;                        // will be set to 0 in the client if ofsSndEmitters doesn't point to MCSE!
    /*0x060*/  std::uint32_t ofsLiquid;
    /*0x064*/  std::uint32_t sizeLiquid;                          // 8 when not used; only read if >8.

    // in alpha, remainder is padding but unused.

    /*0x068*/  Common::DataStructures::C3Vector position;
    /*0x074*/  std::uint32_t ofsMCCV;                             // only with flags.has_mccv, had uint32_t textureId; in ObscuR's structure.
    /*0x078*/  std::uint32_t ofsMCLV;                             // introduced in Cataclysm
    /*0x07C*/  std::uint32_t unused;                              // currently unused
    /*0x080*/
  };

  struct MCVT
  {
    float height[9 * 9 + 8 * 8];
  };

  struct MCLV
  {
    Common::DataStructures::CArgb values[9 * 9 + 8 * 8]; // or rgba?
  };

  struct MCCVEntry
  {
    std::uint8_t blue;                 // these values range from 0x00 to 0xFF with 0x7F being the default.
    std::uint8_t green;                // you can interpret the values as 0x7F being 1.0 and these values being multiplicated with the vertex colors.
    std::uint8_t red;                  // setting all values to 0x00 makes a chunk completely black.
    std::uint8_t alpha;                // seems not to have any effect.
  };

  struct MCCV 
  {
    MCCVEntry entries[9 * 9 + 8 * 8];
  };

  struct MCNREntry
  {
    std::int8_t normal[3];             // normalized. X, Z, Y. 127 == 1.0, -127 == -1.0.
  };

  struct MCNR 
  {
    MCNREntry entries[9 * 9 + 8 * 8];

    std::uint8_t padding[13];            // this data is not included in the MCNR chunk but additional data which purpose is unknown. 0.5.3.3368 lists this as padding
                                    // always 0 112 245  18 0  8 0 0  0 84  245 18 0. Nobody yet found a different pattern. The data is not derived from the normals.
                                    // It also does not seem that the client reads this data. --Schlumpf (talk) 23:01, 26 July 2015 (UTC)
                                    // While stated that this data is not "included in the MCNR chunk", the chunk-size defined for the MCNR chunk does cover this data. --Kruithne Feb 2016
                                    // ... from Cataclysm only (on LK files and before, MCNR defined size is 435 and not 448) Mjollna (talk)
  };

  struct SMLayerFlags
  {
    uint32_t animation_rotation : 3;        // each tick is 45°
    uint32_t animation_speed : 3;
    uint32_t animation_enabled : 1;
    uint32_t overbright : 1;                // This will make the texture way brighter. Used for lava to make it "glow".
    uint32_t use_alpha_map : 1;             // set for every layer after the first
    uint32_t alpha_map_compressed : 1;      // see MCAL chunk description
    uint32_t use_cube_map_reflection : 1;   // This makes the layer behave like its a reflection of the skybox. See below
    uint32_t unknown_0x800 : 1;             // WoD?+ if either of 0x800 or 0x1000 is set, texture effects' texture_scale is applied
    uint32_t unknown_0x1000 : 1;            // WoD?+ see 0x800
    uint32_t : 19;
  };

  struct SMLayer
  {
    /*0x00*/  uint32_t textureId;
    /*0x04*/  SMLayerFlags flags;
    /*0x08*/  uint32_t offsetInMCAL;
    /*0x0C*/  uint32_t effectId;     // 0xFFFFFFFF for none, in alpha: uint16_t + padding
    /*0x10*/
  };

  struct MCSHEntry
  {
    char shadow_map[64][64];
    // or 63x63 with the last column&row&cell auto-filled as detailed in MCAL.
  };

  struct MCLQ_SWVert
  {
    char depth;
    char flow0Pct;
    char flow1Pct;
    char filler;
    float height;
  };

  struct MCLQ_SOVert
  {
    char depth;
    char foam;
    char wet;
    char filler;
  };

  struct MCLQ_SMVert 
  {
    std::int16_t s;
    std::int16_t t;
    float height;
  };

  union MCLQVert  
  {
    MCLQ_SWVert waterVert;
    MCLQ_SOVert oceanVert;
    MCLQ_SMVert magmaVert;
  };

  struct MCLQ_SWFlowv 
  {
    Common::DataStructures::CAaSphere sphere;
    Common::DataStructures::C3Vector dir;
    float velocity;
    float amplitude;
    float frequency;
  };

  struct MCLQ
  {
    Common::DataStructures::CRange height;
    MCLQVert verts[9 * 9];
    char tiles[8][8];
    std::uint32_t n_flows;
    MCLQ_SWFlowv flows[2];
  };

  struct MCSE
  {
    /*000h*/  std::uint32_t entry_id;
    /*004h*/  Common::DataStructures::C3Vector position;
    /*008h*/
    /*00Ch*/
    /*010h*/  Common::DataStructures::C3Vector size;           // I'm not really sure with this. I'm far too lazy to analyze this. Seems like  noone ever needed these anyway.
    /*014h*/
    /*018h*/
  };

  struct MCBB // blend batches. max 256 per MCNK
  {
    std::uint32_t mbmh_index;
    std::uint32_t indexCount; // MBMI
    std::uint32_t indexFirst; // in addition to mbmh.mbnv_base
    std::uint32_t vertexCount; // MBNV 
    std::uint32_t vertexFirst; // in addition to mbmh.mbnv_base
  };

  struct MFBOPlane 
  {
    std::int16_t height[3][3];
  };

  struct MFBO
  {
    MFBOPlane maximum;
    MFBOPlane minimum;
  };

  struct SMTextureFlags
  {
    /*0x00*/  std::uint32_t do_not_load_specular_or_height_texture_but_use_cubemap : 1; // probably just 'disable_all_shading'
    /*0x00*/  std::uint32_t : 3;                                                        // no non-zero values in 20490
    /*0x00*/  std::uint32_t texture_scale : 4;                                          // Texture scale here is not an actual "scale". Default value is 0 (no extra scaling applied). The values are computed as 1 << SMTextureFlags.texture_scale. 
    /*0x01*/  std::uint32_t : 24;                                                       // no non-zero values in 20490                                                   // no non-zero values in 20490
  };

  struct SMTextureParams
  {
    /*0x00*/  SMTextureFlags flags; // same as in mtxf (or taken from there if no mtxp present)
    /*0x04*/  float heightScale;    // default 0.0 -- the _h texture values are scaled to [0, value) to determine actual "height".
                            //                this determines if textures overlap or not (e.g. roots on top of roads). 
    /*0x08*/  float heightOffset;   // default 1.0 -- note that _h based chunks are still influenced by MCAL (blendTex below)
    /*0x0C*/  std::uint32_t padding;     // no default, no non-zero values in 20490
    /*0x10*/
  };

  struct MTCG
  {
    /*0x00*/ std::uint32_t _00; // data is used if one of _00 and _04 are non-0
    /*0x04*/ std::uint32_t _04;
    /*0x08*/ std::uint32_t colorGradingFdid;
    /*0x0C*/ std::uint32_t colorGradingRampFdid;
    /*0x10*/
  };

  struct MBMH // blend mesh header
  {
    std::uint32_t mapObjectID; // (unique ID)
    std::uint32_t textureId; // of linked WMO
    std::uint32_t unknown; // always zero?
    std::uint32_t mbmi_count; // record count in MBMI for this mesh
    std::uint32_t mbnv_count; // record count in MBNV for this mesh
    std::uint32_t mbmi_start; // start record into MBMI for this mesh
    std::uint32_t mbnv_start; // start record into MBNV for this mesh
  };

  struct MBBB  // blend mesh bounding boxes
  {
    std::uint32_t mapObjectID; // (unique ID) -- repeated for unknown reason
    Common::DataStructures::CAaBox bounding;
  };

  struct MBNV // blend mesh vertices
  {
    Common::DataStructures::C3Vector pos;
    Common::DataStructures::C3Vector normal;
    Common::DataStructures::C2Vector texture_coordinates;
    Common::DataStructures::CArgb color[3]; // used: PN: none; PNC: 0; PNC2: 0, 1; PNC2T: 0, 2
  };

  struct MLHD
  {
    std::uint32_t unknown;
    float some_kind_of_bounding[6];
  };

  struct MLLL
  {
    float lod; // lod bands : // 32, 16, 8…
    std::uint32_t height_length;
    std::uint32_t height_index; //index into MLVI
    std::uint32_t mapAreaLow_length;
    std::uint32_t mapAreaLow_index; //index into MLVI
  };

  struct MLND
  {
    std::uint32_t index;  //index into MLVI
    std::uint32_t length; //number of elements in MLVI used
    std::uint32_t _2;
    std::uint32_t _3;
    std::uint16_t indices[4]; // indexes into MLND for child leaves
  };

  struct MLLN
  {
    std::uint32_t _0;
    std::uint32_t num_indices; // MLLI
    std::uint32_t _2;
    std::uint16_t _3a;
    std::uint16_t _3b;
    std::uint32_t _4;
    std::uint32_t _5;
  };

  struct MLMD
  {                        // same as MODF but without bounding box (may be out of sync), better look at both. 
    std::uint32_t mwidEntry;           // they seem to be sorted based on the MLMX's radius, from largest to smallest, likely for optimization, rather than straight out the same as MODF.
    std::uint32_t uniqueId;
    Common::DataStructures::C3Vector position;
    Common::DataStructures::C3Vector rotation;
    std::uint16_t flags;
    std::uint16_t doodadSet;
    std::uint16_t nameSet;
    std::uint16_t unk;
  };

  struct MLMX
  {
    Common::DataStructures::CAaBox bounding;
    float radius;
  };

  struct MLDX
  {
    Common::DataStructures::CAaBox bounding;
    float radius;
  };

  struct MLFD
  {
    std::uint32_t m2LodOffset[3];  //Index into MLDD per lod
    std::uint32_t m2LodLength[3];  //Number of elements used from MLDD per lod
    std::uint32_t wmoLodOffset[3]; //Index into MLMD per lod
    std::uint32_t wmoLodLength[3]; //Number of elements used from MLMD per lod
  };

  struct MLMB
  {
    char unk[20];
  };

  struct MWDR
  {
    std::uint32_t begin; // Index into MWDS.
    std::uint32_t end;   // inclusive: [7, 10] = MWDS[7] + MWDS[8] + MWDS[9] + MWDS[10]
  };

#pragma pack(pop)

}
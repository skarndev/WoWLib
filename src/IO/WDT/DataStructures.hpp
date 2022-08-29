#pragma once

#include <IO/Common.hpp>

#include <cstdint>

namespace IO::WDT::DataStructures
{

  /**
   * Enum-like structure holding MPHD's flags values.
   * @tparam client_version Version of game client.
   */
  template<Common::ClientVersion client_version>
  struct WDTMapHeaderFlags;

  template<Common::ClientVersion client_version>
  requires (client_version == Common::ClientVersion::ANY)
  struct WDTMapHeaderFlags<client_version> : public Utils::Meta::Templates::VersionedEnum<client_version, std::uint32_t>
  {};

  template<Common::ClientVersion client_version>
  requires (client_version < Common::ClientVersion::WOTLK)
  struct WDTMapHeaderFlags<client_version> : public WDTMapHeaderFlags<Common::ClientVersion::ANY>
  {
    enum : std::uint32_t
    {
      WDTUsesGlobalMapObj = 0x1
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version == Common::ClientVersion::WOTLK)
  struct WDTMapHeaderFlags<client_version> : public WDTMapHeaderFlags<Common::ClientVersion::TBC>
  {
    enum : std::uint32_t
    {
      SupportsVertexColor = 0x2,
      UseHighresAlphamap = 0x4,
      SortModelsBySizeCategory = 0x8
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version == Common::ClientVersion::CATA)
  struct WDTMapHeaderFlags<client_version> : public WDTMapHeaderFlags<Common::ClientVersion::WOTLK>
  {
    enum : std::uint32_t
    {
      SupportsVertexLighting = 0x10,
      HasUpsidedownGround = 0x20
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version >= Common::ClientVersion::MOP && client_version < Common::ClientVersion::LEGION)
  struct WDTMapHeaderFlags<client_version> : public WDTMapHeaderFlags<Common::ClientVersion::CATA>
  {
    enum : std::uint32_t
    {
      Unknown_0x40 = 0x40,
      SupportsHeightTextureBlending = 0x80
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version == Common::ClientVersion::LEGION)
  struct WDTMapHeaderFlags<client_version> : public WDTMapHeaderFlags<Common::ClientVersion::MOP>
  {
    enum : std::uint32_t
    {
      UnknownLodRelatedImplicitSet0x8000_0x100 = 0x100,
      UnknownLodRelated_0x8000 = 0x8000
    };
  };

  template<Common::ClientVersion client_version>
  requires (client_version >= Common::ClientVersion::BFA)
  struct WDTMapHeaderFlags<client_version> : public WDTMapHeaderFlags<Common::ClientVersion::LEGION>
  {
    enum : std::uint32_t
    {
      LodADTByFileDataID = 0x200,
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
  struct WDTMapHeader;

  template<Common::ClientVersion client_version>
  requires (client_version < Common::ClientVersion::BFA)
  struct WDTMapHeader<client_version>
  {
    WDTMapHeaderFlags<client_version> flags;
    uint32_t unknown;
    uint32_t pad[6];
  };

  template<Common::ClientVersion client_version>
  requires (client_version >= Common::ClientVersion::BFA)
  struct WDTMapHeader<client_version>
  {
    WDTMapHeaderFlags<client_version> flags;
    uint32_t lgt_file_data_id;
    uint32_t occ_file_data_id;
    uint32_t fogs_file_data_id;
    uint32_t mpv_file_data_id;
    uint32_t tex_file_data_id;
    uint32_t wdl_file_data_id;
    uint32_t pd4_file_data_id;
  };
}
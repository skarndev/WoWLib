#pragma once
#include <IO/ADT/DataStructures.hpp>
#include <IO/Common.hpp>

#include <array>
#include <variant>
#include <bitset>

namespace IO::ADT
{
  struct LiquidLayer;

  class LiquidChunk
  {
  public:
    [[nodiscard]]
    std::vector<LiquidLayer>& layers() { return _layers; };

  private:
    std::vector<LiquidLayer> _layers;
  };

  struct LiquidLayer
  {
    enum class LiquidvertexFormat
    {
      HEIGHT_DEPTH = 0,
      HEIGHT_TEXCOORD = 1,
      DEPTH = 2,
      HEIGHT_DEPTH_TEXCOORD = 3
    };

    std::uint16_t liquid_type;
    LiquidvertexFormat liquid_vertex_format;
    float min_height_level;
    float max_height_level;
    std::uint8_t x_offset;
    std::uint8_t y_offset;
    std::uint8_t width;
    std::uint8_t height;
    std::uint64_t holemap;

    // attributes
    std::bitset<64> fishable;
    std::bitset<64> deep;
    
    bool has_attributes = false;

    std::variant<DataStructures::MH2OHeightDepth, DataStructures::MH2OHeightTexCoord, DataStructures::MH2ODepth, DataStructures::MH2OHeightDepthTexCoord> vertex_data;
    
    void SetLiquidObjectOrLiquidVertexFormat(std::uint16_t liquid_object_or_lvf);


  };

  class MH2O
  {
  public:
    MH2O() = default;

    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer& buf);

    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; }

    [[nodiscard]]
    std::array<LiquidChunk, 16 * 16>& chunks() { return _chunks; }


  private:
    std::array<LiquidChunk, 16 * 16> _chunks;
    bool _is_initialized = false;

  };
}
#pragma once
#include <IO/ADT/DataStructures.hpp>
#include <IO/Common.hpp>

#include <array>
#include <variant>
#include <bitset>
#include <optional>

namespace IO::ADT
{
  struct LiquidLayer
  {
    enum class LiquidVertexFormat
    {
      HEIGHT_DEPTH = 0,
      HEIGHT_TEXCOORD = 1,
      DEPTH = 2,
      HEIGHT_DEPTH_TEXCOORD = 3
    };

    std::uint16_t liquid_type;
    LiquidVertexFormat liquid_vertex_format;
    float min_height_level;
    float max_height_level;

    std::bitset<64> exists_map = {0};

    bool has_vertex_data = false;

    std::variant<DataStructures::MH2OHeightDepth, DataStructures::MH2OHeightTexCoord, DataStructures::MH2ODepth, DataStructures::MH2OHeightDepthTexCoord> vertex_data;

    void SetLiquidObjectOrLiquidVertexFormat(std::uint16_t liquid_object_or_lvf);
    std::uint16_t GetLiquidObjectOrLVF();
  };

  class LiquidChunk
  {
  public:

    struct LiquidAttributes
    {
      std::bitset<64> fishable = {0};
      std::bitset<64> deep = {0};
    };

    [[nodiscard]]
    std::vector<LiquidLayer>& Layers() { return _layers; };

    [[nodiscard]]
    std::optional<LiquidAttributes>& Attributes() { return _attributes; };

    LiquidAttributes& AddAttributes() { return _attributes.emplace(); };

    LiquidAttributes& AddAttributes(DataStructures::SMLiquidChunkAttributes const& attrs) 
    {
      return _attributes.emplace(LiquidChunk::LiquidAttributes{attrs.fishable, attrs.deep});
    };

    LiquidAttributes& AddAttributes(std::uint64_t fishable, std::uint64_t deep) 
    {
      return _attributes.emplace(LiquidChunk::LiquidAttributes{fishable, deep});
    };

  private:
    std::vector<LiquidLayer> _layers;
    std::optional<LiquidAttributes> _attributes;
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
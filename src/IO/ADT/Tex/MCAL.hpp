#ifndef IO_ADT_TEX_MCAL_HPP
#define IO_ADT_TEX_MCAL_HPP
#include <IO/Common.hpp>
#include <IO/ByteBuffer.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/WorldConstants.hpp>
#include <Utils/Meta/Templates.hpp>

#include <cstdint>
#include <vector>
#include <array>

namespace IO::ADT
{
  using Alphamap = std::array<std::uint8_t, Common::WorldConstants::N_PIXELS_PER_ALPHAMAP>;

  enum class AlphaFormat
  {
    LOWRES = 0,
    HIGHRES = 1
  };

  enum class AlphaCompression
  {
    UNCOMPRESSED = 0,
    COMPRESSED = 1
  };

  using MCLYChunk = Common::DataArrayChunk
                    <
                      DataStructures::SMLayer
                      , ChunkIdentifiers::ADTTexMCNKSubchunks::MCLY
                      , Common::FourCCEndian::Little
                      , 0
                      , Common::WorldConstants::CHUNK_MAX_TEXTURE_LAYERS
                    >;


  template<typename T>
  concept MCALReadContext = requires (T t) {
                                             { t.alpha_layer_params } -> std::same_as<MCLYChunk&>;
                                             { t.fix_alpha } -> std::same_as<bool&>;
                                             { t.alpha_format } -> std::same_as<AlphaFormat&>;
                                           };


  template<typename T>
  concept MCALWriteContext = requires (T t) {
                                              { t.alpha_layer_params } -> std::same_as<MCLYChunk&>;
                                              { t.alpha_format } -> std::same_as<AlphaFormat&>;
                                            };


  template<MCALReadContext ReadContext, MCALWriteContext WriteContext>
  class MCAL : public Utils::Meta::Templates::ConstrainedArray<Alphamap, 0, 3>
  {
  public:
    void Read(ReadContext& read_ctx, Common::ByteBuffer const& buf, std::size_t size);

    void Write(WriteContext& write_ctx, Common::ByteBuffer& buf) const;

    [[nodiscard]]
    FORCEINLINE bool IsInitialized() const { return true; };

  private:
    static std::uint8_t NormalizeLowresAlpha(std::uint8_t alpha)
    {
      return alpha / 255 + (alpha % 255 <= 127 ? 0 : 1);
    };

    static std::uint8_t NormalizeHighresAlpha(std::uint32_t alpha, std::uint32_t div)
    {
      return alpha / div + (alpha % div <= (div >> 1) ? 0 : 1);
    };

  };
}

#include <IO/ADT/Tex/MCAL.inl>
#endif // IO_ADT_TEX_MCAL_HPP

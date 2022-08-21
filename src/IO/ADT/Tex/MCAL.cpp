#include <IO/ADT/Tex/MCAL.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/WorldConstants.hpp>
#include <Validation/Log.hpp>
#include <Validation/Contracts.hpp>
#include <Utils/Meta/Future.hpp>

#include <boost/range/combine.hpp>
#include <boost/range/iterator_range.hpp>

#include <algorithm>

using namespace IO::ADT;
using namespace IO::Common;
using namespace IO::ADT::ChunkIdentifiers;

template<MCALReadContext ReadContext, MCALWriteContext WriteContext>
inline void MCAL<ReadContext, WriteContext>::Read(ReadContext& read_ctx
                                                  , ByteBuffer const& buf
                                                  , std::size_t size)
{

  RequireF(CCodeZones::FILE_IO
           , read_ctx.alpha_layer_params.Size() > 0
           && read_ctx.alpha_layer_params.Size() <= WorldConstants::CHUNK_MAX_TEXTURE_LAYERS
           , "Only 3 alpha layers is supported.");
  RequireF(CCodeZones::FILE_IO, (read_ctx.fix_alpha && read_ctx.alpha_format == AlphaFormat::LOWRES)
          || !read_ctx.fix_alpha,
          "Alpha fixing is only needed for lowres alpha.");

  LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCAL, size: %d.", size);

  if (read_ctx.alpha_format == AlphaFormat::HIGHRES)
  {
    for (std::size_t layer_idx = 1; layer_idx < read_ctx.alpha_layer_params.Size(); ++layer_idx)
    {
      auto compression_type = read_ctx.alpha_layer_params[layer_idx].flags.alpha_map_compressed
          ? AlphaCompression::COMPRESSED : AlphaCompression::UNCOMPRESSED;

      // 4096 uncompressed highres alpha
      if (compression_type == AlphaCompression::UNCOMPRESSED)
      {
          auto& alphamap = _data.emplace_back();
          buf.Read(alphamap.begin(), alphamap.end());
      }
      // compressed highres alpha
      else
      {
        auto& alphamap = _data.emplace_back();

        std::size_t pixel = 0;

        while (pixel != Common::WorldConstants::N_BYTES_PER_HIGHRES_ALPHA)
        {
          auto const& control_byte = buf.ReadView<DataStructures::CompressedAlphaByte>();

          switch (control_byte.mode)
          {
            case DataStructures::AlphaCompressionMode::COPY:
            {
              buf.Read(alphamap.begin() + pixel, alphamap.begin() + pixel + control_byte.count);
              pixel += control_byte.count;
              break;
            }
            case DataStructures::AlphaCompressionMode::FILL:
            {
              auto next_byte = buf.Read<std::uint8_t>();
              std::fill(alphamap.begin() + pixel, alphamap.begin() + pixel + control_byte.count, next_byte);
              pixel += control_byte.count;
              break;
            }
          }
        }
      }
    }
  }
  // uncompressed 2048 alpha
  else
  {
    for (std::size_t layer_idx = 1; layer_idx < read_ctx.alpha_layer_params.Size(); ++layer_idx)
    {
      InvariantF(CCodeZones::FILE_IO, !read_ctx.alpha_layer_params[layer_idx].flags.alpha_map_compressed
        , "Appha compression is not supported for 2048 alpha. Potentially corrupt file.");

      auto& alphamap = _data.emplace_back();

      std::size_t pos = buf.Tell();
      const char* raw_buffer = buf.Data() + pos;

      for (std::size_t i = 0; i < WorldConstants::ALPHAMAP_DIM; ++i)
      {
        for (std::size_t j = 0; j < WorldConstants::ALPHAMAP_DIM; j += 2)
        {
          alphamap[i * WorldConstants::ALPHAMAP_DIM + j] = ((*raw_buffer & 0x0f) << 4) | (*raw_buffer & 0x0f);
          alphamap[i * WorldConstants::ALPHAMAP_DIM + j + 1] = ((*raw_buffer & 0xf0) >> 4) | (*raw_buffer & 0xf0);
          raw_buffer++;
        }
      }
      buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(2048);

      // Fill last row and column from the previous ones
      if (read_ctx.fix_alpha)
      {
        constexpr std::uint32_t last_pixel_idx = WorldConstants::ALPHAMAP_DIM - 1;
        constexpr std::uint32_t pre_last_pixel_idx = last_pixel_idx - 1;

        for (std::size_t i = 0; i < WorldConstants::ALPHAMAP_DIM; ++i)
        {
          alphamap[i * WorldConstants::ALPHAMAP_DIM + last_pixel_idx]
            = alphamap[i * WorldConstants::ALPHAMAP_DIM + pre_last_pixel_idx];
          alphamap[last_pixel_idx * WorldConstants::ALPHAMAP_DIM + i]
            = alphamap[pre_last_pixel_idx * WorldConstants::ALPHAMAP_DIM + i];
        }
        // handle corner pixel
        alphamap[pre_last_pixel_idx * WorldConstants::ALPHAMAP_DIM + last_pixel_idx]
          = alphamap[pre_last_pixel_idx * WorldConstants::ALPHAMAP_DIM + pre_last_pixel_idx];
      }
    }

    // normalize alpha for highres format
    for (std::size_t i = 0; i < WorldConstants::N_PIXELS_PER_ALPHAMAP; ++i)
    {
      std::uint8_t max_alpha = 255;

      for (auto it = _data.rbegin(); it < _data.rend(); ++it)
      {
        std::uint8_t val = MCAL::NormalizeLowresAlpha((*it)[i] * max_alpha);
        EnsureF(CCodeZones::FILE_IO, max_alpha >= val, "Unexpected underflow.");
        max_alpha -= val;
        (*it)[i] = val;
      }
    }
  }
}


template<MCALReadContext ReadContext, MCALWriteContext WriteContext>
void MCAL<ReadContext, WriteContext>::Write(WriteContext& write_ctx, Common::ByteBuffer& buf) const
{
  RequireF(CCodeZones::FILE_IO
           , !_data.empty() && _data.size() < WorldConstants::CHUNK_MAX_TEXTURE_LAYERS
           , "Only 3 alpha layers is supported.");

  RequireF(CCodeZones::FILE_IO, _data.size() == (write_ctx.alpha_layer_params.Size() - 1)
           , "Layers params size mismatch.");

  ChunkHeader header {Common::FourCC<"MCAL">, 0};
  std::size_t chunk_pos = buf.Tell();
  buf.Write(header);

  // highres 4096 alpha
  if (write_ctx.alpha_format == AlphaFormat::HIGHRES)
  {
    for (auto const&& [alphamap, layer_params]
      : boost::combine(_data, boost::iterator_range(write_ctx.alpha_layer_params.begin() + 1
                                                    , write_ctx.alpha_layer_params.end())))
    {
      auto compression = layer_params.flags.alpha_map_compressed
          ? AlphaCompression::COMPRESSED : AlphaCompression::UNCOMPRESSED;

      // uncompressed
      if (compression == AlphaCompression::UNCOMPRESSED)
      {
        buf.Write(alphamap.begin(), alphamap.end());
      }
      // compressed
      else
      {
        for (std::size_t i = 0; i < WorldConstants::ALPHAMAP_DIM; ++i)
        {
          // we go line by line and identify contigious blocks in the current line of pixels
          std::vector<std::pair<std::uint8_t, std::size_t>> compression_blocks {};

          auto& cur_block = compression_blocks.emplace_back(std::pair{alphamap[i * WorldConstants::ALPHAMAP_DIM], 1});

          for (std::size_t j = 1; j < WorldConstants::ALPHAMAP_DIM; ++j)
          {
            std::uint8_t cur_pixel = alphamap[i * WorldConstants::ALPHAMAP_DIM + j];

            if (cur_pixel != cur_block.first)
            {
              cur_block = compression_blocks.emplace_back(std::pair{cur_pixel, 1});
            }
            else
            {
              cur_block.second++;
            }
          }

          bool is_copy_block_current = false;
          std::size_t copy_control_byte_pos = 0;
          std::uint8_t n_copy_bytes = 0;

          for (auto& block : compression_blocks)
          {
            // write fill
            if (block.second > 1)
            {
              // if breaking copy block, make sure to write the relevant control byte
              if (is_copy_block_current)
              {
                is_copy_block_current = false;
                std::size_t cur_pos = buf.Tell();

                DataStructures::CompressedAlphaByte control_byte {static_cast<std::uint8_t>(n_copy_bytes)
                                                                  , DataStructures::AlphaCompressionMode::COPY};

                buf.Seek(copy_control_byte_pos);
                buf.Write(control_byte);
                buf.Seek(cur_pos);

                copy_control_byte_pos = 0;
                n_copy_bytes = 0;

              }

              DataStructures::CompressedAlphaByte control_byte {static_cast<std::uint8_t>(block.second)
                                                                , DataStructures::AlphaCompressionMode::FILL};

              buf.Write(control_byte);
              buf.WriteFill(block.first, block.second);
            }
            else
            {
              // if last block was fill, make sure to start a new copy block
              if (!is_copy_block_current)
              {
                is_copy_block_current = true;
                copy_control_byte_pos = buf.Tell();

                // write dummy control byte to keep the layout, we return to it later
                DataStructures::CompressedAlphaByte control_byte {0, DataStructures::AlphaCompressionMode::FILL};
                buf.Write(control_byte);
              }
              n_copy_bytes++;
              buf.Write(block.first);
            }

            // if we ended on copy block, make sure to write its control byte
            if (is_copy_block_current)
            {
              std::size_t cur_pos = buf.Tell();

              DataStructures::CompressedAlphaByte control_byte {static_cast<std::uint8_t>(n_copy_bytes)
                                                                , DataStructures::AlphaCompressionMode::COPY};

              buf.Seek(copy_control_byte_pos);
              buf.Write(control_byte);
              buf.Seek(cur_pos);

            }
          }
        }
      }
    }
  }
  else
  {
    // convert alpha format from 4096 uncompressed to 2048 uncompressed
    std::vector<std::array<std::uint8_t, WorldConstants::N_PIXELS_PER_ALPHAMAP>> temp_layers{};
    temp_layers.resize(temp_layers.size());

    for (std::size_t i = 0; i < WorldConstants::N_PIXELS_PER_ALPHAMAP; ++i)
    {
      std::int32_t max_alpha = 255;

      for (auto&& [alphamap, temp_alphamap] : boost::combine(_data, temp_layers))
      {
        if (max_alpha <= 0) [[unlikely]]
        {
          temp_alphamap[i] = 0;
        }
        else
        {
          std::uint8_t pixel = alphamap[i];

          temp_alphamap[i] = MCAL::NormalizeHighresAlpha(pixel * 255, max_alpha);
          max_alpha -= pixel;
        }
      }
    }

    // actualy write lowres 2048 alpha
    for (auto& alphamap : temp_layers)
    {
      for (std::size_t i = 0; i < WorldConstants::N_BYTES_PER_LOWRES_ALPHA; ++i)
      {
        std::uint8_t nibble_1 = (alphamap[i * 2] & 0xF0) >> 4;
        std::uint8_t nibble_2 = (alphamap[(i * 2) + 1] & 0xF0);

        std::uint8_t lowres_value_byte = nibble_1 | nibble_2;

        buf.Write(lowres_value_byte);
      }
    }
  }

  std::size_t end_pos = buf.Tell();
  std::size_t chunk_size = end_pos - (chunk_pos + 8);
  header.size = static_cast<std::uint32_t>(chunk_size);
  buf.Seek(end_pos);
  buf.Write(header);
}



#include <IO/ADT/Tex/MCAL.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <Validation/Log.hpp>
#include <Validation/Contracts.hpp>
#include <Utils/Meta/Future.hpp>

#include <algorithm>

using namespace IO::ADT;

void MCAL::Read(IO::Common::ByteBuffer const& buf
                , std::size_t size
                , AlphaFormat format
                , Common::DataArrayChunk
                  <
                    DataStructures::SMLayer
                    , ChunkIdentifiers::ADTTexMCNKSubchunks::MCLY
                    , Common::FourCCEndian::LITTLE
                    , 0
                    , 4
                  > const& alpha_layer_params
                , bool fix_alpha)
{

  RequireF(CCodeZones::FILE_IO, alpha_layer_params.Size() > 0 && alpha_layer_params.Size() < 4, "Only 3 alpha layers is supported.");
  RequireF(CCodeZones::FILE_IO, (fix_alpha && format == AlphaFormat::LOWRES) || !fix_alpha,
          "Alpha fixing is only needed for lowres alpha.");

  LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCAL, size: %d.", size);

  if (format == AlphaFormat::HIGHRES)
  {
    for (std::size_t layer_idx = 0; layer_idx < alpha_layer_params.Size(); ++layer_idx)
    {
      auto compression_type = alpha_layer_params[layer_idx].flags.alpha_map_compressed
          ? AlphaCompression::COMPRESSED : AlphaCompression::UNCOMPRESSED;

      // 4096 uncompressed highres alpha
      if (compression_type == AlphaCompression::UNCOMPRESSED)
      {
          auto& alphamap = _alphamap_layers.emplace_back();
          buf.Read(alphamap.begin(), alphamap.end());
      }
      // compressed highres alpha
      else
      {
        auto& alphamap = _alphamap_layers.emplace_back();

        std::size_t pixel = 0;

        while (pixel != 4096)
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
    for (std::size_t layer_idx = 0; layer_idx < alpha_layer_params.Size(); ++layer_idx)
    {
      InvariantF(CCodeZones::FILE_IO, !alpha_layer_params[layer_idx].flags.alpha_map_compressed
        , "Appha compression is not supported for 2048 alpha. Potentially corrupt file.");

      auto& alphamap = _alphamap_layers.emplace_back();

      std::size_t pos = buf.Tell();
      const char* raw_buffer = buf.Data() + pos;

      for (std::size_t i = 0; i < 64; ++i)
      {
        for (std::size_t j = 0; j < 64; j += 2)
        {
          alphamap[i * 64 + j] = ((*raw_buffer & 0x0f) << 4) | (*raw_buffer & 0x0f);
          alphamap[i * 64 + j + 1] = ((*raw_buffer & 0xf0) >> 4) | (*raw_buffer & 0xf0);
          raw_buffer++;
        }
      }
      buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(2048);

      // Fill last row and column from the previous ones
      if (fix_alpha)
      {
        for (std::size_t i = 0; i < 64; ++i)
        {
          alphamap[i * 64 + 63] = alphamap[i * 64 + 62];
          alphamap[63 * 64 + i] = alphamap[62 * 64 + i];
        }
        // handle corner pixel
        alphamap[63 * 64 + 63] = alphamap[62 * 64 + 62];
      }
    }

    // normalize alpha for highres format
    for (std::size_t i = 0; i < 64 * 64; ++i)
    {
      std::uint8_t a = 255;

      for (auto it = _alphamap_layers.rbegin(); it < _alphamap_layers.rend(); ++it)
      {
        std::uint8_t val = MCAL::NormalizeLowresAlpha((*it)[i] * a);
        EnsureF(CCodeZones::FILE_IO, a >= val, "Unexpected underflow.");
        a -= val;
        (*it)[i] = val;
      }
    }
  }
}


void MCAL::Write(IO::Common::ByteBuffer& buf
                 , MCAL::AlphaFormat format
                 ,  Common::DataArrayChunk
                    <
                        DataStructures::SMLayer
                        , ChunkIdentifiers::ADTTexMCNKSubchunks::MCLY
                        , Common::FourCCEndian::LITTLE
                        , 0
                        , 4
                    > const& alpha_layer_params) const
{


  RequireF(CCodeZones::FILE_IO, !_alphamap_layers.empty() && _alphamap_layers.size() < 4
           , "Only alpha layers is supported.");

  Common::ChunkHeader header {Common::FourCC<"MCAL">};
  std::size_t chunk_pos = buf.Tell();
  buf.Write(header);

  // highres 4096 alpha
  if (format == AlphaFormat::HIGHRES)
  {
    for (auto const&& [alphamap, layer_params] : future::zip(_alphamap_layers, alpha_layer_params))
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
        for (std::size_t i = 0; i < 64; ++i)
        {
          // we go line by line and identify contigious blocks in the current line of pixels
          std::vector<std::pair<std::uint8_t, std::size_t>> compression_blocks {};

          auto& cur_block = compression_blocks.emplace_back(std::pair{alphamap[i * 64], 1});

          for (std::size_t j = 1; j < 64; ++j)
          {
            std::uint8_t cur_pixel = alphamap[i * 64 + j];

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
    std::vector<std::array<std::uint8_t, 64 * 64>> temp_layers{};
    temp_layers.resize(temp_layers.size());

    for (std::size_t i = 0; i < 64 * 64; ++i)
    {
      std::int32_t a = 255;

      for (auto&& [alphamap, temp_alphamap] : future::zip(_alphamap_layers, temp_layers))
      {
        if (a <= 0) [[unlikely]]
        {
          temp_alphamap[i] = 0;
        }
        else
        {
          std::uint8_t pixel = alphamap[i];

          temp_alphamap[i] = MCAL::NormalizeHighresAlpha(pixel * 255, a);
          a -= pixel;
        }
      }
    }

    // actualy write lowres 2048 alpha
    for (auto& alphamap : temp_layers)
    {
      for (std::size_t i = 0; i < 2048; ++i)
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
  header.size = chunk_size;
  buf.Seek(end_pos);
  buf.Write(header);
}

MCAL::Alphamap& MCAL::Add()
{
  InvariantF(CCodeZones::FILE_IO, _alphamap_layers.size() < 3, "3 alphamap layers are supported at max.");

  auto& layer = _alphamap_layers.emplace_back();
  std::fill(layer.begin(), layer.end(), 0);

  return layer;
}

MCAL::Alphamap& MCAL::At(std::uint8_t index)
{
  RequireF(CCodeZones::FILE_IO, index < _alphamap_layers.size(), "Out of bounds access.");
  return _alphamap_layers[index];
}

MCAL::Alphamap const& MCAL::At(std::uint8_t index) const
{
  RequireF(CCodeZones::FILE_IO, index < _alphamap_layers.size(), "Out of bounds access.");
  return _alphamap_layers[index];
}

void MCAL::Remove(std::uint8_t index)
{
  RequireF(CCodeZones::FILE_IO, index < _alphamap_layers.size(), "Out of bounds access.");
  _alphamap_layers.erase(_alphamap_layers.begin() + index);
}

MCAL::Alphamap const& MCAL::operator[](std::size_t index) const
{
  RequireF(CCodeZones::FILE_IO, index < _alphamap_layers.size(), "Out of bounds access.");
  return _alphamap_layers[index];
}

MCAL::Alphamap& MCAL::operator[](std::size_t index)
{
  RequireF(CCodeZones::FILE_IO, index < _alphamap_layers.size(), "Out of bounds access.");
  return _alphamap_layers[index];
}



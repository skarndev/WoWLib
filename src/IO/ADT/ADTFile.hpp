#pragma once
#include <IO/Common.hpp>

#include <cstdint>

namespace IO::ADT
{
  class ADTFile : public IO::Common::IChunkedFile
  {
  public:
    ADTFile(std::uint32_t file_data_id);
  };
}
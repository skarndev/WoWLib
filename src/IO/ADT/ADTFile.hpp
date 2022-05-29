#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>

#include <cstdint>

namespace IO::ADT
{
  class ADTFile : public IO::Common::IChunkedFile
  {
  public:
    ADTFile(std::uint32_t file_data_id);

    void Read(std::fstream const& fstream) override;
    void Write(std::fstream const& fstream) const override;

  private:
    std::uint32_t mver_version;

    // all
    DataStructures::MHDR mhdr;
    
    // root

  };
}
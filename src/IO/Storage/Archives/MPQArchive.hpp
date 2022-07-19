#pragma once

#include <IO/Storage/Archives/IArchive.hpp>

#include <string>
#include <stdexcept>
#include <cstdint>

namespace IO::Common
{
  class ByteBuffer;
}

namespace IO::Storage::Archives
{
  namespace Exceptions
  {
    struct MPQOpenFailedError : public std::runtime_error
    {
      explicit MPQOpenFailedError(std::string const& mpq) : std::runtime_error("Failed opening MPQ: " + mpq) {};
    };
  }

  class MPQArchive : public IArchive
  {
  public:
    /**
     * Constructs and opens MPQ archive.
     * @param path Path to MPQ archive in filesystem.
     * @throws IO::Storage::Exceptions::MPQOpenFailedError Thrown when MPQ opening failed.
     */
    explicit MPQArchive(std::string const& path);

    ~MPQArchive() override;

    /**
     * Read file from MPQ archive into buffer.
     * @param file_key File key.
     * @param buf Buffer to read data into.
     * @return Status of operation.
     */
    [[nodiscard]]
    FileKey::FileReadStatus ReadFile(FileKey const& file_key, Common::ByteBuffer& buf) const override;

    /**
     * Check if file exists in MPQ archive.
     * @param file_key File key.
     * @return true if exists, else false.
     */
    [[nodiscard]]
    bool Exists(FileKey const& file_key) const override;

    /**
     * Path of MPQ Archive.
     * @return Path to MPQ Archive in filesystem.
     */
    [[nodiscard]]
    std::string const& Path() const { return _path; };

  private:
    std::string _path;
  };
}
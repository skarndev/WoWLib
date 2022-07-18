#pragma once
#include "IO/Storage/FileKey.hpp"

#include <string>

namespace IO::Common
{
  class ByteBuffer;
}

namespace IO::Storage::Archives
{
  using HANDLE = void*;

  /**
   * Implements a common interface for storage archives (CASC or MPQ for now)
   */
  class IArchive
  {
  public:
    /**
     * Read file from the archive into buffer.
     * @param file_key File key.
     * @param buf Buffer to read data into.
     * @return true if success, else false.
     */
    [[nodiscard]]
    virtual FileKey::FileReadStatus ReadFile(FileKey const& file_key, Common::ByteBuffer& buf) = 0;


    /**
     * Check if file exists in an archive
     * @param file_key File key.
     * @return True if exists, else false.
     */
    [[nodiscard]]
    virtual bool Exists(FileKey const& file_key) = 0;

    virtual ~IArchive() = default;

  protected:
    HANDLE _handle = nullptr;
  };
}
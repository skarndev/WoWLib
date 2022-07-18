#pragma once

#include <IO/Storage/Archives/IArchive.hpp>
#include <IO/Storage/FileKey.hpp>
#include <IO/ByteBuffer.hpp>

#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

namespace IO::Common
{
  class ByteBuffer;
}

namespace IO::Storage
{
  class ClientStorage;
}

namespace IO::Storage::ClientLoaders
{
  namespace Exceptions
  {
    /**
     * Indicates failure to find the locale directory of MPQ based clients (e.g. enUS inside /Data).
     */
    struct LocaleDirNotFoundError : public std::runtime_error
    {
      explicit LocaleDirNotFoundError(std::string const& msg) : std::runtime_error(msg) {};
    };

    /**
     * Indicates failure to find "Data" directory in the client root folder of a WoW client.
     */
    struct DataDirNotFoundError : public std::runtime_error
    {
      explicit DataDirNotFoundError() : std::runtime_error("\"Data\" directory does not exist in provided path.") {};
    };

    /**
     * Indicates failure to load an archive. Either CASC or MPQ clients.
     */
    struct ArchiveLoadingFailureError : public std::runtime_error
    {
      explicit ArchiveLoadingFailureError(std::string const& path)
        : std::runtime_error("Failed loading archive: " + path) {};
    };
  }

  /**
   * BaseLoader implements a common interface for all client loaders.
   */
  class BaseLoader
  {
  public:

    /**
     * Construct BaseLoader.
     * @param storage Client storage instance.
     */
    explicit BaseLoader(ClientStorage& storage) : _storage(&storage) {};
    virtual ~BaseLoader() = default;

    /**
     * Reads file from MPQ to provided buffer
     * @param filepath Filepath in game game format.
     * @return True if file was read succesfully, else False.
     */
    [[nodiscard]]
    FileKey::FileReadStatus ReadFile(FileKey const& file_key, Common::ByteBuffer& buf);

    /**
     * Check if file exists in loaded archives.
     * @param file_key File key.
     * @return true if exists, else false.
     */
    [[nodiscard]]
    bool Exists(FileKey const& file_key) const;

    /**
     * Gets the most possibly complete listfile from MPQ archives. (MPQ only).
     * @return Self-owned ByteBuffer containing listfile contents. May contain duplicated entries.
     */
    virtual Common::ByteBuffer GetListfile() { assert(false); return Common::ByteBuffer(); };

  protected:
    std::vector<std::unique_ptr<IO::Storage::Archives::IArchive>> _archives;
    ClientStorage* const _storage;

  };

}
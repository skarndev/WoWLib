#pragma once
#include <IO/Storage/Archives/IArchive.hpp>

namespace IO::Storage::Archives
{
  namespace Exceptions
  {
    struct CASCStorageOpenFailedError : public std::runtime_error
    {
      explicit CASCStorageOpenFailedError(std::string const& CASCStorage) : std::runtime_error("Failed opening CASCStorage: " + CASCStorage) {};
    };
  }

  using HANDLE = void*;

  /**
   * Implements a wrapper over CASCLib to handle reading from the CASC System.
   * Supports both local and online storages.
   * Constructor overloads define the type of storage to use. Custom CDN URL may be supplied to support custom-hosted
   * TACT systems.
   */
  class CASCArchive : public IArchive
  {
  public:
    /**
    * Initialize local CASC-based CASCArchive.
    * @param path Path to client.
    * @param project_path Product code name.
    * @throws IO::Storage::Archives::Exceptions::CASCStorageOpenFailedError Thrown if CASCLib returned one of its errors.
    */
    CASCArchive(std::string const& path, std::string const& product);

    /**
     * Initialize online CASC-based client loader.
     * @param url Custom CDN URL (if std::nullopt, use Blizzard CDN).
     * @param product Product code name.
     * @param region Region for the product, for example "eu".
     * @throws IO::Storage::Archives::Exceptions::CASCStorageOpenFailedError Thrown if CASCLib returned one of its errors.
     */
    CASCArchive(std::string const& project_path
    , std::optional<std::string> const& cdn_url
    , std::string const& product
    , std::string const& region);

    ~CASCArchive() override;

    /**
     * Read file from CASC archive into buffer.
     * @param file_key File key.
     * @param buf Buffer to read data into.
     * @return Status of operation.
     */
    [[nodiscard]]
    FileKey::FileReadStatus ReadFile(FileKey const& file_key, Common::ByteBuffer& buf) override;

    /**
     * Check if file exists in CASC archive.
     * @param file_key File key.
     * @return true if exists, else false.
     */
    [[nodiscard]]
    bool Exists(FileKey const& file_key) override;

    /**
     * Path of CASC Archive.
     * @return Path to CASC Archive in filesystem.
     */
    [[nodiscard]]
    std::string const& Path() const { return _path; };

  private:
    std::string _path;
    HANDLE _handle;
  };
}
#pragma once
#include <IO/Common.hpp>
#include <IO/Storage/Listfile.hpp>
#include <IO/Storage/FileKey.hpp>

#include <stdexcept>
#include <memory>
#include <filesystem>

namespace IO::Storage
{
  namespace Exceptions
  {
    struct FileNotFoundError : std::runtime_error
    {
      explicit FileNotFoundError(std::string const& msg) : std::runtime_error(msg) {};
    };
  }

  namespace ClientLoaders
  {
    class BaseLoader;
  }

  enum class ClientStorageOpenMode
  {
    CLIENT = 0,
    LOCAL = 1
  };

  class ClientStorage
  {
    friend struct FileKey;
  public:
    /**
     * Constructs and opens an MPQ-based client storage. Implementation depends on client version.
     * In Debug passing a CASC-based client version is asserted, in Release such behavior is undefined.
     * @param path Path to WoW client root directory.
     * @param project_path Path to a local directory used as a project path.
     * @param client_version Version of WoW client (< WOD).
     * @param open_mode Defines whether ClientStorage operates on a client, or an extracted data directory.
     * @param locale Locale of the client.
     * @throws IO::Storage::ClientLoaders::Exceptions::LocaleDirNotFoundError Thrown if no locale dir was found.
     * @throws IO::Storage::Storage::Exceptions::ArchiveLoadingFailureError Thrown if MPQ failed loading.
     */
    ClientStorage(std::string const& path
                  , std::string const& project_path
                  , Common::ClientVersion client_version
                  , Common::ClientLocale locale = Common::ClientLocale::AUTO);

    /**
     * Constructs and opens a local CASC-based client storage.
     * In Debug passing an MPQ-based client version is asserted, in Release such behavior is undefined.
     * @param path Path to WoW client directory containing "Data" folder.
     * @param project_path Path to a local directory used as a project path.
     * @param client_version Version of WoW client (>= WOD).
     * @param product A product code name. See list of known products on WoWDev (https://wowdev.wiki/TACT#Products).
     * @param locale Locale of the client.
     */
    ClientStorage(std::string const& path
                  , std::string const& project_path
                  , Common::ClientVersion client_version
                  , std::string const& product
                  , Common::ClientLocale locale = Common::ClientLocale::AUTO);


    /**
     * Constructs and opens an online custom-hosted CASC-based client storage.
     * In Debug passing an MPQ-based client version is asserted, in Release such behavior is undefined.
     * @param cdn_url A custom CDN URL hosting TACT storage in the following format http://mysite.com:8000.
     * @param project_path Path to a local directory used as a project path.
     * @param client_version Version of WoW client (>= WOD).
     * @param product A product code name. See list of known products on WoWDev (https://wowdev.wiki/TACT#Products).
     * @param region A region for the product.
     * @param locale Locale of the client.
     */
    ClientStorage(std::string const& cdn_url
                  , std::string const& project_path
                  , Common::ClientVersion client_version
                  , std::string const& product
                  , std::string const& region
                  , Common::ClientLocale locale = Common::ClientLocale::AUTO);

    /**
     * Gets underlying listfile.
     * @return Reference to listfile.
     */
    [[nodiscard]]
    Listfile& Listfile() { return _listfile; };

    /**
     * Gets client locale.
     * @return Client locale enum.
     */
    [[nodiscard]]
    Common::ClientLocale Locale() const { return _locale; };

    /**
     * Gets the project path of storage.
     * @return Project path.
     */
    [[nodiscard]]
    std::filesystem::path const& ProjectPath() const { return _project_path; };

    /**
     * Gets the path of storage.
     * @return Storage path.
     */
     [[nodiscard]]
     std::filesystem::path const& Path() const { return _path; };

     /**
      * Gets the client version this storage is constructed for.
      * @return Client version.
      */
     [[nodiscard]]
     Common::ClientVersion ClientVersion() const { return _client_version; };
  private:
    /**
     * Reads the file content into the provided buffer.
     * @param file_key File key.
     * @param buf ByteBuffer instance to read data into.
     * @return Status of the file reading operation.
     */
    [[nodiscard]]
    FileKey::FileReadStatus ReadFile(FileKey const& file_key, Common::ByteBuffer& buf) const;

    /**
     * Writes the file content from the provided buffer into project dir.
     * @param file_key File key.
     * @param buf ByteBuffer instance to read data from.
     * @return Status of the file writing operation.
     */
    [[nodiscard]]
    FileKey::FileWriteStatus WriteFile(FileKey const& file_key, Common::ByteBuffer const& buf) const;

    /**
     * Check if file exists in the storage.
     * @param file_key File key.
     * @return true if exists, else false.
     */
    [[nodiscard]]
    bool Exists(FileKey const& file_key) const;

  private:
    class Listfile _listfile;
    std::filesystem::path _project_path;
    std::filesystem::path _path;
    std::unique_ptr<ClientLoaders::BaseLoader> _loader;
    Common::ClientLocale _locale;
    Common::ClientVersion _client_version;
  };
}


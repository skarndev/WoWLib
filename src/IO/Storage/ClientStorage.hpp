#pragma once
#include <IO/Common.hpp>

namespace IO::Storage
{
  enum class ClientStorageOpenMode
  {
    CLIENT = 0,
    LOCAL = 1
  };

  template<Common::ClientVersion client_version, ClientStorageOpenMode open_mode>
  class ClientStorage
  {
    /**
     * Constructs and opens an MPQ based client storage.
     * @param path Path to WoW client root directory
     * @param project_path Path to a local directory used as a project path
     * @param locale Locale of the client
     */
    ClientStorage(std::string const& path
                  , std::string const& project_path
                  , Common::ClientLocale locale = Common::ClientLocale::AUTO)
    requires (open_mode == ClientStorageOpenMode::CLIENT && client_version < Common::ClientVersion::WOD);

    /**
    * Constructs and opens a CASC based client storage.
    * @param path Path to WoW client root directory
    * @param project_path Path to a local directory used as a project path
    * @param locale Locale of the client
    */
    ClientStorage(std::string const& path
                  , std::string const& project_path
                  , Common::ClientLocale locale = Common::ClientLocale::AUTO)
    requires (open_mode == ClientStorageOpenMode::CLIENT && client_version > Common::ClientVersion::WOD);

    /**
    * Constructs and opens a local storage (directory)
    * @param path Path to WoW client root directory
    * @param project_path Path to a local directory used as a project path (can be the same as "path").
    * @param locale Locale of the client
    */
    ClientStorage(std::string const& path
                  , std::string const& project_path
                  , Common::ClientLocale locale = Common::ClientLocale::AUTO)
    requires (open_mode == ClientStorageOpenMode::LOCAL);

    /**
     * Returns FileDataID for provided filepath.
     * @param filepath Filepath in storage or local project folder.
     * @return FileDataID or 0 if no known FileDataID is associated with this filepath.
     */
    [[nodiscard]]
    std::uint32_t FileDataIDForFilepath(std::string const& filepath) const;

    /**
     * Returns filepath for provided FileDataID.
     * @param file_data_id FileDataID.
     * @return Filepath or generic filepath (stringified FileDataID) if filepath is not known.
     */
    [[nodiscard]]
    std::string const& FilepathForFileDataID(std::uint32_t file_data_id) const;

    /**
     * Create a new FileDataID entry for provided path.
     * Does not check if FileDataID is already present.
     * @param filepath Filepath
     * @return FileDataID
     */
    [[nodiscard]]
    std::uint32_t CreateFileDataIDForPath(std::string const& filepath);

    /**
     * Checks if file exists.
     * @param file_data_id FileDataID.
     * @return True if file exists in storage and project dir.
     */
    [[nodiscard]]
    bool FileExists(std::uint32_t file_data_id) const;

    /**
    * Checks if file exists.
    * @param file_data_id FileDataID.
    * @return True if file exists in storage and project dir.
    */
    [[nodiscard]]
    bool FileExists(std::string const& filepath) const;
  };
}
#ifndef IO_STORAGE_FILEKEY_HPP
#define IO_STORAGE_FILEKEY_HPP

#include <IO/Common.hpp>

#include <stdexcept>
#include <cstdint>

namespace IO::Storage
{
  namespace Exceptions
  {
    class FileNotFoundError : public std::runtime_error
    {
    public:
      FileNotFoundError(std::string const& msg = "") : std::runtime_error(msg) {};
    };
  }

  template<Common::ClientVersion client_version>
  class ClientStorage;

  enum class FileExistPolicy
  {
    /**
     * Throw exception when file is not found.
     */
    STRICT = 0,

    /**
     * Do not throw when file is not found.
     */
    WEAK = 1,

    /**
     * Create file if not found.
     */
    CREATE = 2
  };

  enum class FilePathCorrectionPolicy
  {
    CORRECT = 0,
    TRUST = 1,
  };

  /**
   * FileKey class provides a generalized way to adress files within WoW formats.
   * file can be requested by FileDataID (real CASC one or runtime-generated for pre-CASC clients)
   * or its string filepath. Internally always a FileDataID.
   * @tparam client_version Version of WoW client
   * @tparam exist_policy Controls handling of cases when requested file is not present in the storage.
   * @tparam filepath_correction Filepath correction policy (conversion of / into \\, lower-case).
   */
  template<Common::ClientVersion client_version
            , FileExistPolicy exist_policy = FileExistPolicy::WEAK
            , FilePathCorrectionPolicy filepath_correction = FilePathCorrectionPolicy::TRUST>
  struct FileKey
  {
    /**
     * Construct FileKey based on FileDataID.
     * @param storage WoW client storage.
     * @param file_data_id FileDataID.
     */
    FileKey(ClientStorage<client_version>& storage, std::uint32_t file_data_id)
    requires (exist_policy != FileExistPolicy::CREATE);

    /**
     * Construct FileKey based on filepath.
     * @param storage WoW client storage.
     * @param filepath Filepath (either in project dir, or storage).
     */
    FileKey(ClientStorage<client_version>& storage, std::string const& filepath);

    /**
     * @return Return associated FileDataID.
     */
    [[nodiscard]]
    std::uint32_t FileDataID() const { return _file_data_id; };

    /**
     * @return Return associated filepath. If filepath is not known, stringified path based on FileDataID is returned.
     */
    [[nodiscard]]
    std::string const& FilePath() const;

    /**
     * @return Returns reference to associated client storage.
     */
    [[nodiscard]]
    ClientStorage<client_version>& Storage() const { return _storage; };

    // Filepath helpers

    /**
     * Normalize filepath to match game client rules (all uppercase, using \\ as separator).
     * @param filepath Filepath.
     * @return Normalized filepath.
     */
    static std::string NormalizeFilepathGame(std::string const& filepath);

    /**
     * Normalize filepath to match Unix filesystem requirements (/ as separator) and lowercase it.
     * @param filepath Filepath.
     * @return Normalized filepath.
     */
    static std::string NormalizeFilepathUnix(std::string const& filepath);

  private:
    std::uint32_t _file_data_id;
    ClientStorage<client_version>* const _storage;
  };

}
#include <IO/Storage/FileKey.hpp>
#endif // IO_STORAGE_FILEKEY_HPP
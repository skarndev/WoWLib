#pragma once
#include <IO/Common.hpp>

#include <stdexcept>
#include <cstdint>

namespace IO::Storage
{
  class ClientStorage;

  /**
   * FileKey class provides a generalized way to adress files within WoW formats.
   * file can be requested by FileDataID (real CASC one or runtime-generated for pre-CASC clients)
   * or its string filepath. Internally always a FileDataID.
   */
  struct FileKey
  {
    enum class FileExistPolicy
    {
      CHECKEXISTS, ///< Throw exception when file is not found in listfile.
      WEAK, ///< Do not throw exception if file is not found in listfile.
      CREATE ///< Create file if not found.
    };

    enum class FilePathCorrectionPolicy
    {
      CORRECT, ///< Always format the filepath to the appropriate game format.
      TRUST ///< Trust the filepath to already be preformatted into the appropriate game format. (Still validated in Debug).
    };

    /**
     * Defines possible states of file reading operation.
     */
    enum class FileReadStatus
    {
      SUCCESS, ///< File read was successful.
      FILE_NOT_FOUND, ///< File was not found in client storage.
      FILE_OPEN_FAILED_OS, ///< File open failed from OS.
      FILE_OPEN_FAILED_CLIENT, ///< File open failed from client storage.
      FILE_READ_FAILED, ///< File read failed.
      INVALID_FILEDATAID, ///< FileDataID stored in FileKey is invalid (0).
      NOT_ENOUGH_MEMORY ///< Not enough memory to allocate the file buffer.
    };

    enum class FileWriteStatus
    {
      SUCCESS, ///< File write was successful.
      FILE_WRITE_FAILED ///< File write failed.
    };

    /**
     * Construct FileKey based on FileDataID.
     * @param storage WoW client storage.
     * @param file_data_id FileDataID.
     * @param file_exist_policy File exist policy determines what to do if file is not found.
     * @throws IO::Storage::Exceptions::FileNotFoundError Thrown if FileDataID does not exists in archive (only for STRICT).
     */
    FileKey(ClientStorage& storage
            , std::uint32_t file_data_id
            , FileExistPolicy file_exist_policy = FileExistPolicy::WEAK);

    /**
     * Construct FileKey based on filepath.
     * @param storage WoW client storage.
     * @param filepath Filepath (either in project dir, or storage).
     * @param filepath_correction_policy Determines whether to fix filepath to game format, or trust the user to handle it.
     * @throws IO::Storage::Exceptions::FileNotFoundError Thrown if filepath does not exist in listfile (only for STRICT).
     */
    FileKey(ClientStorage& storage
            , std::string const& filepath
            , FilePathCorrectionPolicy filepath_correction_policy = FilePathCorrectionPolicy::TRUST
            , FileExistPolicy file_exist_policy = FileExistPolicy::WEAK);

    /**
     * @return Return associated FileDataID. 0 is not a valid FileDataID.
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
    ClientStorage& Storage() const { return *_storage; };

    /**
     * Read file from associated storage into an instance of ByteBuffer.
     * @param buf Self-owned ByteBuffer instance.
     * @return Status of operation.
     */
    [[nodiscard]]
    FileReadStatus Read(Common::ByteBuffer& buf) const;

    /**
     * Write file into project directory.
     * @param buf Self-owned ByteBuffer instance.
     * @return Status of operation.
     */
    [[nodiscard]]
    FileWriteStatus Write(Common::ByteBuffer const& buf) const;

    /**
     * Check if file exists in the associated storage.
     * @return true if exists, else false.
     */
     [[nodiscard]]
     bool Exists() const;

  private:
    std::uint32_t _file_data_id = 0;
    ClientStorage* const _storage;
  };
}
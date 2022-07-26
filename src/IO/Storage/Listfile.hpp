#pragma once

#include <boost/bimap.hpp>
#include <string>
#include <stdexcept>

namespace IO::Common
{
  class ByteBuffer;
}

namespace IO::Storage
{
  namespace Exceptions
  {
    class ListFileNotFoundError : public std::runtime_error
    {
    public:
      ListFileNotFoundError(std::string const& msg = "listfile.csv not found.") : std::runtime_error(msg) {};
    };
  }

  /**
   * Controls how FileDataIDs are handled within the Listfile class.
   */
  enum class FileDataIDPolicy
  {
    /**
     * FileDataIDs are real FileDataIDs from the CASC storage.
     */
    REAL = 0,

    /**
     * FileDataIDs are fake FileDataIDs assigned at runtime to reduce string overhead in library code.
     * Used for pre-CASC clients.
     */
    INTERNAL = 1
  };

  /**
   * Used for managing FileDataIDs and paths of the client.
   */
  class Listfile
  {
  public:
    Listfile() = default;

    /**
     * Constructs Listfile provided a path to listfile (CASC-based clients).
     * @param path Path to listfile.csv (FileDataID;path)
     * @param max_file_data_id Maximum FileDataID to start adding new ones from.
     * @throws IO::Storage::Exceptions::ListFileNotFoundError Thrown if listfile.csv is not found.
     */
    explicit Listfile(std::string const& path, std::uint32_t max_file_data_id = 0);

    /**
     * Constructs Listfile provided a ByteBuffer containing listfile (MPQ-based clients).
     * @param listfile_buf ByteBuffer containing array of strings (paths).
     */
    explicit Listfile(Common::ByteBuffer const& listfile_buf);

    /**
     * Returns FileDataID for filepath. (assignes a new one, if does not exist)
     * @param filepath Game format filepath
     * @return FileDataID
     */
    [[nodiscard]]
    std::uint32_t GetOrAddFileDataID(std::string const& filepath);

    /**
     * Returns FileDataID for filepath.
     * @param filepath Game format filepath.
     * @return FileDataID. 0 if does not exist.
     */
    [[nodiscard]]
    std::uint32_t GetFileDatIDForFilepath(std::string const& filepath) const;

    /**
     * Returns filepath for given FileDataID. If FileDataID does not exist, automatic path is returned.
     * @param file_data_id FileDataID.
     * @return Filepath (either real or approximated).
     */
    [[nodiscard]]
    std::string const& GetOrGenerateFilepath(std::uint32_t file_data_id);

    /**
     * Checks if FileDataID already exists in Listfile.
     * @param file_data_id FileDataID.
     * @return true if exists, else false.
     */
    [[nodiscard]]
    bool Exists(std::uint32_t file_data_id) const;

    /**
     * Saves listfile. Works only for CASC-based clients.
     * @throws IO::Storage::Exceptions::ListFileNotFoundError Thrown if listfile writing failed.
     */
    void Save();


  private:
    using bm_type = boost::bimap<std::uint32_t, std::string>;
    bm_type _fdid_path_map;
    std::string _path;
    std::uint32_t _max_file_data_id;
    FileDataIDPolicy _file_data_id_policy;

  };
}
#ifndef IO_STORAGE_CLIENTLOADERS_MPQLOADER_HPP
#define IO_STORAGE_CLIENTLOADERS_MPQLOADER_HPP
#include <IO/Storage/ClientLoaders/BaseLoader.hpp>
#include <IO/Storage/ClientStorage.hpp>

#include <string>
#include <iterator>
#include <filesystem>


namespace IO::Storage::ClientLoaders
{
  /**
   * Implements common functionality for all MPQ loaders.
   */
  class MPQLoader : public BaseLoader
  {
  public:
    /**
     * Initialize MPQ loader.
     * @param storage Client storage constructing this loader.
     */
    explicit MPQLoader(ClientStorage& storage);

    /**
     * Gets the most possibly complete listfile from MPQ archives.
     * @return Self-owned ByteBuffer containing listfile contents. May contain duplicated entries.
     */
    Common::ByteBuffer GetListfile() override;

  protected:
    /**
     * Loads MPQ archive into the internal storage.
     * @param path Path to archive (MPQ or MPQ-like dir).
     * @throws IO::Storage::Storage::Exceptions::ArchiveLoadingFailureError Thrown if MPQ failed loading.
     */
    void LoadArchive(std::string const& path);

    /**
     * Replace the locale component of MPQ path template with specified locale.
     * If the template does not contain {locale}, this method does nothing.
     * @param mpq_path MPQ path template (mutated).
     * @param locale Locale to insert.
     */
    void ReplaceLocale(std::string& mpq_path, std::string_view const& locale) const;

    /**
     * Attempt to load all supported variations of numbered patch templates.
     * patch-{number}.MPQ is the expected template. If does not match, the function has no side effects.
     * Note, that this templates are case sensitive. Alternatively cased filenames considered to be invalid.
     * @param mpq_path MPQ path template (mutated).
     * @return true if template was matched, else false.
     * @throws IO::Storage::ClientLoaders::Exceptions::ArchiveLoadingFailureError Thrown if one of the MPQs failed loading.
     */
    [[nodiscard]]
    bool LoadNumberedPatches(std::string& mpq_path);

    /**
     * Attempt to load all supported variations of character numbered patch templates.
     * patch-{character}.mpq is the expected template. If does not match, the function has no side effects.
     * Note, that this templates are case sensitive. Alternatively cased filenames considered to be invalid.
     * @param mpq_path MPQ path template (mutated).
     * @return true if template was matched, else false.
     * @throws IO::Storage::ClientLoaders::Exceptions::ArchiveLoadingFailureError Thrown if one of the MPQs failed loading.
     */
    [[nodiscard]]
    bool LoadCharacterNumberedPatches(std::string& mpq_path);

    /**
     * Determines locale string based on storage settings.
     * @param data_path Path to "Data" dir in the client.
     * @return String representing locale to load.
     * @throws IO::Storage::ClientLoaders::Exceptions::LocaleDirNotFoundError Thrown if locale cannot be determined.
     */
    [[nodiscard]]
    std::string_view DetermineLocale(std::filesystem::path const& data_path) const;

    /**
     * Loads MPQ archives based on common pattern of loading classic / tbc / wotlk clients.
     * @tparam T std::string_view array iterator.
     * @throws IO::Storage::ClientLoaders::Exceptions::ArchiveLoadingFailureError Thrown if one of the MPQs failed loading.
     * @throws IO::Storage::ClientLoaders::Exceptions::LocaleDirNotFoundError Thrown if locale cannot be determined (tbc/wotlk only).
     * @throws IO::Storage::ClientLoaders::Exceptions::DataDirNotFoundError Thrown if "Data" directory does not exist (malformed client install).
     */
    template<typename T>
    void LoadClassicTBCWotLK(T const& begin, T const& end)
    requires (std::contiguous_iterator<T> && std::same_as<std::iter_value_t<T>, std::string_view>);
  };
}

#endif // IO_STORAGE_CLIENTLOADERS_MPQLOADER_HPP
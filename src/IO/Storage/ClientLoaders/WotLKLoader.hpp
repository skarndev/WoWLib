#pragma once
#include <IO/Storage/ClientLoaders/MPQLoader.hpp>

#include <string>
#include <array>

namespace IO::Storage
{
  class ClientStorage;
}

namespace IO::Storage::ClientLoaders
{
  /**
   * Implements WotLK MPQ loading.
   */
  class WotLKLoader : public MPQLoader
  {
  public:
    /**
     * Initialize WOTLK MPQ client loader.
     * @param storage Client storage constructing this loader.
     * @throws IO::Storage::ClientLoaders::Exceptions::LocaleDirNotFoundError Thrown if no locale dir was found.
     * @throws IO::Storage::Storage::Exceptions::ArchiveLoadingFailureError Thrown if MPQ failed loading.
     */
    explicit WotLKLoader(ClientStorage& storage);

  private:

    inline static constexpr std::array<std::string_view, 14> ArchiveNameTemplates {
        // common archives
        "common.MPQ"
        , "common-2.MPQ"
        , "expansion.MPQ"
        , "lichking.MPQ"
        , "patch.MPQ"
        , "patch-{number}.MPQ"
        , "patch-{character}.MPQ"

        // locale-specific archives
        , "{locale}/locale-{locale}.MPQ"
        , "{locale}/expansion-locale-{locale}.MPQ"
        , "{locale}/lichking-locale-{locale}.MPQ"
        , "{locale}/patch-{locale}.MPQ"
        , "{locale}/patch-{locale}-{number}.MPQ"
        , "{locale}/patch-{locale}-{character}.MPQ"
        , "development.MPQ"
    };
  };
}
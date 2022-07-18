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
   * Implements Classic MPQ loading.
   */
  class ClassicLoader : public MPQLoader
  {
  public:
    /**
     * Initialize Classic MPQ client loader.
     * @param storage Client storage constructing this loader.
     * @throws IO::Storage::Storage::Exceptions::ArchiveLoadingFailureError Thrown if MPQ failed loading.
     */
    explicit ClassicLoader(ClientStorage& storage);

  private:

    inline static constexpr std::array<std::string_view, 15> ArchiveNameTemplates {
        // common archives
        "backup.MPQ"
        , "base.MPQ"
        , "dbc.MPQ"
        , "fonts.MPQ"
        , "interface.MPQ"
        , "misc.MPQ"
        , "model.MPQ"
        , "sound.MPQ"
        , "speech.MPQ"
        , "terrain.MPQ"
        , "texture.MPQ"
        , "wmo.MPQ"
        , "patch.MPQ"
        , "patch-{number}.MPQ"
        , "patch-{character}.MPQ"
    };
  };
}
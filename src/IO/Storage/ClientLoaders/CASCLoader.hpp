#pragma once
#include <IO/Storage/ClientLoaders/BaseLoader.hpp>

#include <optional>

namespace IO::Storage::ClientLoaders
{
  /**
   * Implements a basic translation layer from the loader system to CASC.
   */
  class CASCLoader : public BaseLoader
  {
  public:
    /**
     * Initialize local CASC-based client loader.
     * @param storage Client storage constructing this loader.
     * @param path Path to client.
     * @param product A product code name. See list of known products on WoWDev (https://wowdev.wiki/TACT#Products).
     */
    CASCLoader(ClientStorage& storage, std::string const& product);

    /**
     * Initialize online CASC-based client loader.
     * @param storage Client storage constructing this loader.
     * @param url URL of custom CDN. (If std::nullopt, use Blizzard CDNs).
     * @param product A product code name. Same as the other overload.
     * @param region A region for the product, for example "eu".
     */
    CASCLoader(ClientStorage& storage
               , std::optional<std::string> const& cdn_url
               , std::string const& product
               , std::string const& region);

  };


}
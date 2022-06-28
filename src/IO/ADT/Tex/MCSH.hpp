#pragma once
#include <IO/ByteBuffer.hpp>
#include <IO/Common.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/WorldConstants.hpp>

#include <bitset>

namespace IO::ADT
{
  class MCSH
  {
  public:
    MCSH();

    void Read(Common::ByteBuffer const& buf, std::size_t size, bool fix_last_row_col);
    void Write(Common::ByteBuffer& buf) const;

    [[nodiscard]]
    std::bitset<Common::WorldConstants::N_PIXELS_PER_SHADOWMAP>& Shadowmap() { return _shadowmap; };

    [[nodiscard]]
    std::bitset<Common::WorldConstants::N_PIXELS_PER_SHADOWMAP> const& Shadowmap()const { return _shadowmap; };

    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; };

    void Initialize() { _is_initialized = true; };

  private:
    std::bitset<Common::WorldConstants::N_PIXELS_PER_SHADOWMAP> _shadowmap;
    bool _is_initialized = false;
  };
}
#pragma once
#include <IO/Common.hpp>
#include <IO/ByteBuffer.hpp>

#include <cstdint>

namespace IO::ADT
{
  class MCAL
  {
  public:
    MCAL();

    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer buf);


  private:
    std::array<std::uint8_t, 64 * 64> _alphamap;
  };
}

#include <IO/Common.hpp>

#include <cassert>

using namespace IO::Common;


template<typename T>
void IChunkedFile<T>::Read(std::fstream& fstream)
{
  ByteBuffer buf{ fstream };
  Read(buf);
}

template<typename T>
void IChunkedFile<T>::Write(std::fstream& fstream) const
{
  ByteBuffer buf{};
  Write(buf);
  buf.Flush(fstream);
}


template<typename T>
std::uint32_t DataChunk<T>::byte_size() const 
{
  if constexpr (std::is_trivial<T>::value && std::is_standard_layout<T>::value)
  {
    return _data.size() * sizeof(T);
  }
  else if constexpr(std::derived_from<T, IComplexChunk>)
  {
    assert(_data.size() == 1);
    return _data[0].byte_size();
  }
  else
  {
    static_assert(!sizeof(T), "Invalid type.");
  }
}

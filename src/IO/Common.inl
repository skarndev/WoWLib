#include <IO/Common.hpp>
#include <cassert>

using namespace IO::Common;

template<typename T>
void DataChunk<T>::Read(ByteBuffer const& buf, std::uint32_t size)
{
  if constexpr (std::is_trivial<T>::value && std::is_standard_layout<T>::value)
  {
    std::size_t n_elements = size / sizeof(T);

    _data.reserve(n_elements);
    for (int i = 0; i < n_elements; ++i)
    {
      T& element = _data.emplace_back();
      buf.Read(element);
    }
  }
  else if constexpr (std::derived_from<T, IComplexChunk>)
  {
    T& element = _data.emplace_back();
    element.Read(buf, size);
  }
  else
  {
    static_assert(!sizeof(T), "Invalid type.");
  }
   
}

template<typename T>
void DataChunk<T>::Write(ByteBuffer& buf)
{
  if constexpr (std::is_trivial<T>::value && std::is_standard_layout<T>::value)
  {
    for (T const& element : _data)
    {
      buf.Write(element);
    }
  }
  else if constexpr (std::derived_from<T, IComplexChunk>)
  {
    assert(_data.size() == 1);
    _data[0].Write(buf);
  }
  else
  {
    static_assert(!sizeof(T), "Invalid type.");
  }
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

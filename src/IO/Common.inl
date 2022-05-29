#include <IO/Common.hpp>
#include <cassert>

using namespace IO::Common;

template<typename T>
void DataChunk<T>::Read(std::fstream const& fstream, std::uint32_t size) requires (std::is_trivial<T>::value && std::is_standard_layout<T>::value)
{
   std::size_t n_elements = size / sizeof(T);

   _data.reserve(n_elements);
   for (int i = 0; i < n_elements; ++i)
   {
     T& element = _data.emplace_back();
     fstream.read(&element, sizeof(T));
   }
}

template<typename T>
void DataChunk<T>::Write(std::fstream const& fstream) const requires (std::is_trivial<T>::value && std::is_standard_layout<T>::value)
{
  for (T const& element : _data)
  {
    fstream.write(&element, sizeof(T));
  }
}

template<typename T>
std::uint32_t DataChunk<T>::byte_size() const requires (std::is_trivial<T>::value && std::is_standard_layout<T>::value)
{
  return _data.size() * sizeof(T);
}

template<typename T>
void DataChunk<T>::Read(std::fstream const& fstream, std::uint32_t size) requires std::derived_from<T, IComplexChunk>
{
  T& element = _data.emplace_back();
  element.Read(fstream, size);
}

template<typename T>
void DataChunk<T>::Write(std::fstream const& fstream) const requires std::derived_from<T, IComplexChunk>
{
  assert(_data.size() == 1);
  _data[0].Write(fstream);
}

template<typename T>
std::uint32_t DataChunk<T>::byte_size() const requires std::derived_from<T, IComplexChunk>
{
  assert(_data.size() == 1);
  return _data[0].byte_size();
}


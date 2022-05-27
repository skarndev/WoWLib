#include <IO/Common.hpp>

#include <type_traits>
#include <concepts>
#include <cassert>

using namespace IO::Common;


template<typename T>
concept POD = std::is_pod<T>::value;

template<typename T>
concept IsComplexChunk = std::derived_from<T, IComplexChunk>;

template<typename T>
requires POD<T>
void DataChunk<T>::Read(std::fstream const& fstream, std::uint32_t size) 
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
requires POD<T>
void DataChunk<T>::Write(std::fstream const& fstream) const
{
  for (T const& element : _data)
  {
    fstream.write(&element, sizeof(T));
  }
}

template<typename T>
requires POD<T>
std::uint32_t DataChunk<T>::byte_size() const
{
  return _data.size() * sizeof(T);
}

template<typename T>
requires IsComplexChunk<T>
void DataChunk<T>::Read(std::fstream const& fstream, std::uint32_t size)
{
  T& element = _data.emplace_back();
  element.Read(fstream, size);
}

template<typename T>
requires IsComplexChunk<T>
void DataChunk<T>::Write(std::fstream const& fstream) const
{
  assert(_data.size() == 1);
  _data[0].Write(fstream);
}

template<typename T>
requires IsComplexChunk<T>
std::uint32_t DataChunk<T>::byte_size() const
{
  assert(_data.size() == 1);
  return _data[0].byte_size();
}

IChunkedFile::IChunkedFile(std::uint32_t file_data_id)
: _file_data_id(file_data_id)
{
}

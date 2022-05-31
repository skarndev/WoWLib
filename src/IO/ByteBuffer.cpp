#include <IO/ByteBuffer.hpp>

using namespace IO::Common;

ByteBuffer::ByteBuffer(const char* data, std::size_t size)
  : _size((Require(size, "Size can't be 0 for initializing the buffer."), size))
  , _buf_size(size)
  , _cur_pos(0)
  , _data((Require(data != nullptr, "Data can't be null for initializing the buffer."), new char[size]))
  , _is_data_owned(true)
{
  std::memcpy(_data.get(), data, size);
}

ByteBuffer::ByteBuffer(char* data, std::size_t size)
  : _size((Require(size, "Size can't be 0 for initializing the buffer."), size))
  , _buf_size(size)
  , _cur_pos(0)
  , _is_data_owned(false)
  , _data((Require(data != nullptr, "Data can't be null for initializing the buffer."), data))
{
}

ByteBuffer::ByteBuffer(std::fstream& stream, std::size_t size)
  : _size((Require(size, "Size can't be 0 for initializing the buffer."), size))
  , _buf_size(size)
  , _cur_pos(0)
  , _is_data_owned(true)
  , _data(new char[size])
{
  stream.read(_data.get(), size);
}

ByteBuffer::ByteBuffer(std::size_t size)
  : _size((Require(size, "Size can't be 0 for initializing the buffer."), size))
  , _buf_size(size)
  , _cur_pos(0)
  , _is_data_owned(true)
  , _data(new char[size])
{
}

ByteBuffer::~ByteBuffer()
{
  if (!_is_data_owned)
  {
    _data.release();
  }
}

void ByteBuffer::Read(char* dest, std::size_t offset, std::size_t n) const
{
  Require(dest != nullptr, "Can't read to nullptr.");
  Require(offset <= _size && n <= _size && std::numeric_limits<std::size_t>::max() - n >= _size, "Buffer offset overflow.");
  std::memcpy(dest, _data.get() + offset, n);
}

void ByteBuffer::Read(char* dest, std::size_t n) const
{
  Require(dest != nullptr, "Can't read to nullptr.");
  Require(n <= _size && std::numeric_limits<std::size_t>::max() - n >= _size - _cur_pos, "Buffer offset overflow.");
  std::memcpy(dest, _data.get() + _cur_pos, n);
}

void ByteBuffer::Write(char* src, std::size_t n, std::size_t offset)
{
  Require(std::numeric_limits<std::size_t>::max() - offset >= n, "Buffer size overflow on writing.");

  if ((offset + n) > _size)
  {
    Reserve(offset + n - _size);
  }

  std::memcpy(_data.get() + offset, src, n);
}

void IO::Common::ByteBuffer::Write(char* src, std::size_t n)
{
  Require(std::numeric_limits<std::size_t>::max() - _cur_pos >= n, "Buffer size overflow on writing.");

  if ((_cur_pos + n) > _size)
  {
    Reserve(_cur_pos + n - _size);
  }

  std::memcpy(_data.get() + _cur_pos, src, n);

  _cur_pos += n;
}


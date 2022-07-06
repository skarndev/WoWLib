#include <IO/ByteBuffer.hpp>

using namespace IO::Common;

ByteBuffer::ByteBuffer(const char* data, std::size_t size)
  : _size((RequireFE(CCodeZones::FILE_IO, size, "Size can't be 0 for initializing the buffer."), size))
  , _buf_size(size)
  , _cur_pos(0)
  , _data((RequireFE(CCodeZones::FILE_IO, data != nullptr, "Data can't be null for initializing the buffer.")
      , new char[size]))
  , _is_data_owned(true)
{
  std::memcpy(_data.get(), data, size);
}

ByteBuffer::ByteBuffer(char* data, std::size_t size)
  : _size((RequireFE(CCodeZones::FILE_IO, size, "Size can't be 0 for initializing the buffer."), size))
  , _buf_size(size)
  , _cur_pos(0)
  , _is_data_owned(false)
  , _data((RequireFE(CCodeZones::FILE_IO, data != nullptr, "Data can't be null for initializing the buffer."), data))
{
}

ByteBuffer::ByteBuffer(std::fstream& stream, std::size_t size)
  : _size((RequireFE(CCodeZones::FILE_IO, size, "Size can't be 0 for initializing the buffer."), size))
  , _buf_size(size)
  , _cur_pos(0)
  , _is_data_owned(true)
  , _data(new char[size])
{
  stream.read(_data.get(), size);
}

ByteBuffer::ByteBuffer(std::fstream& stream)
  : _cur_pos(0)
  , _is_data_owned(true)
{
  stream.seekg(0, std::ios::end);
  _size = static_cast<std::size_t>(stream.tellg());
  _buf_size = _size;
  EnsureF(CCodeZones::FILE_IO, _size, "Size can't be 0 for initializing the buffer.");
  _data.reset(new char[_size]);
  stream.seekg(0, std::ios::beg);

  stream.read(_data.get(), _size);
}

ByteBuffer::ByteBuffer(std::size_t size)
  : _size(size)
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

bool ByteBuffer::IsEof() const
{
  InvariantF(CCodeZones::FILE_IO, _cur_pos <= _size, "Current pos is never supposed to be past EOF.");
  return _cur_pos == _size;
}

void ByteBuffer::Read(char* dest, std::size_t offset, std::size_t n) const
{
  RequireF(CCodeZones::FILE_IO, dest != nullptr, "Can't read to nullptr.");
  RequireF(CCodeZones::FILE_IO, offset <= _size && n <= _size && std::numeric_limits<std::size_t>::max() - n >= _size
           , "Buffer offset overflow.");
  std::memcpy(dest, _data.get() + offset, n);
}

void ByteBuffer::Read(char* dest, std::size_t n) const
{
  RequireF(CCodeZones::FILE_IO, dest != nullptr, "Can't read to nullptr.");
  RequireF(CCodeZones::FILE_IO, n <= _size && std::numeric_limits<std::size_t>::max() - n >= _size - _cur_pos
           , "Buffer offset overflow.");
  std::memcpy(dest, _data.get() + _cur_pos, n);
}

void ByteBuffer::Write(const char* src, std::size_t n, std::size_t offset)
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - offset >= n
           , "Buffer size overflow on writing.");

  if ((offset + n) > _size) [[likely]]
  {
    Reserve(offset + n - _size);
  }

  std::memcpy(_data.get() + offset, src, n);
}

void IO::Common::ByteBuffer::Write(const char* src, std::size_t n)
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - _cur_pos >= n
           , "Buffer size overflow on writing.");

  if ((_cur_pos + n) > _size) [[likely]]
  {
    Reserve(_cur_pos + n - _size);
  }

  std::memcpy(_data.get() + _cur_pos, src, n);

  _cur_pos += n;
}

void ByteBuffer::Flush(std::fstream& stream) const
{
  stream.write(_data.get(), _size);
}

void ByteBuffer::Flush(std::ostream& stream) const
{
  stream.write(_data.get(), _size);
}


void ByteBuffer::WriteString(std::string const& data)
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - _cur_pos >= data.size() + 1
           , "Buffer size overflow on writing.");

  if ((_cur_pos + data.size() + sizeof(char)) > _size)
  {
    Reserve(_cur_pos + data.size() + sizeof(char) - _size);
  }

  std::memcpy(_data.get() + _cur_pos, data.data(), data.size());
  char null_term = 0;
  Write(null_term);

  _cur_pos += data.size() + sizeof(char);
}

std::string ByteBuffer::ReadString() const
{
  std::string cur_string {};

  char cur_char;
  do
  {
    EnsureF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - sizeof(char) >= _cur_pos
             , "Buffer pos overflow.");
    EnsureF(CCodeZones::FILE_IO, _cur_pos + sizeof(char) <= _size, "Requested read larger than EOF.");

    cur_char = Read<char>();

    if (cur_char)
    {
      cur_string += cur_char;
    }

  } while(cur_char);

  _cur_pos += (cur_string.size() + sizeof(char));

  return cur_string;
}

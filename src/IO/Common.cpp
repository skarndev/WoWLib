#include <IO/Common.hpp>

using namespace IO::Common;


IChunkedFile::IChunkedFile(std::uint32_t file_data_id)
  : _file_data_id(file_data_id)
{
}

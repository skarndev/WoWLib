#include <IO/ADT/ADTFile.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

using namespace IO::ADT;

ADTFile::ADTFile(std::uint32_t file_data_id)
: IChunkedFile(file_data_id)
{

}

void ADTFile::Read(std::fstream const& fstream)
{

}

void ADTFile::Write(std::fstream const& fstream) const
{

}

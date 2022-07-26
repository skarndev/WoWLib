#include <IO/ADT/Obj/ADTObj.hpp>

using namespace IO::ADT;

bool ADTLodMapObjectBatches::Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
{
  if (chunk_header.fourcc == ChunkIdentifiers::ADTObjCommonChunks::MLMB)
  {
    _lod_map_object_batches.Read(buf, chunk_header.size);
    return true;
  }

  return false;
}

void ADTLodMapObjectBatches::Write(Common::ByteBuffer& buf) const
{
  _lod_map_object_batches.Write(buf);
}

bool ADTDoodadsetOverrides::Read(IO::Common::ByteBuffer const& buf, IO::Common::ChunkHeader const& chunk_header)
{
  switch (chunk_header.fourcc)
  {
    case ChunkIdentifiers::ADTObjCommonChunks::MWDS:
    {
      this->_wmo_doodadset_overrides.Read(buf, chunk_header.size);
      return true;
    }
    case ChunkIdentifiers::ADTObjCommonChunks::MWDR:
    {
      this->_wmo_doodadset_overrides_ranges.Read(buf, chunk_header.size);
      return true;
    }
  }

  return false;
}

void ADTDoodadsetOverrides::Write(IO::Common::ByteBuffer& buf) const
{
  _wmo_doodadset_overrides_ranges.Write(buf);
  _wmo_doodadset_overrides.Write(buf);
}

bool LodModelBatches::Read(IO::Common::ByteBuffer const& buf, IO::Common::ChunkHeader const& chunk_header)
{
  if (chunk_header.fourcc == ChunkIdentifiers::ADTObj1Chunks::MLDB)
  {
    _lod_model_batches.Read(buf, chunk_header.size);
    return true;
  }

  return false;
}

void LodModelBatches::Write(IO::Common::ByteBuffer& buf) const
{
 _lod_model_batches.Write(buf);
}

bool ADTObj0ModelStorageFilepath::Read(IO::Common::ByteBuffer const& buf, IO::Common::ChunkHeader const& chunk_header)
{
  switch(chunk_header.fourcc)
  {
    case ChunkIdentifiers::ADTObj0Chunks::MMDX:
      _model_filenames.Read(buf, chunk_header.size);
      return true;
    case ChunkIdentifiers::ADTObj0Chunks::MMID:
      _model_filename_offsets.Read(buf, chunk_header.size);
      return true;
    case ChunkIdentifiers::ADTObj0Chunks::MWMO:
      _map_object_filenames.Read(buf, chunk_header.size);
      return true;
    case ChunkIdentifiers::ADTObj0Chunks::MWID:
      _map_object_filename_offsets.Read(buf, chunk_header.size);
      return true;
  }

  return false;
}

void ADTObj0ModelStorageFilepath::Write(IO::Common::ByteBuffer& buf) const
{
  // contracts
  {
    InvariantF(CCodeZones::FILE_IO, _model_filename_offsets.IsInitialized()
                                    && _model_filenames.IsInitialized()
                                    && _map_object_filename_offsets.IsInitialized()
                                    && _map_object_filenames.IsInitialized()
               , "Essential chunks MMDX, MMID, MWMO, MWID are not initialized.");

    InvariantF(CCodeZones::FILE_IO, _model_filename_offsets.Size() == _model_filenames.Size()
                                    && _map_object_filename_offsets.Size()== _map_object_filenames.Size()
               , "Filename storage should match with offsets map in size.");
  }

  _model_filenames.Write(buf);
  _model_filename_offsets.Write(buf);
  _map_object_filenames.Write(buf);
  _map_object_filename_offsets.Write(buf);
}

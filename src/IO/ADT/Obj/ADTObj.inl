#pragma once
#include <IO/ADT/Obj/ADTObj.hpp>
#include <Utils/Meta/Future.hpp>

namespace IO::ADT
{
  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  inline void ADTObj<client_version, lod_level>::Read(IO::Common::ByteBuffer const& buf)
  {
    LogDebugF(LCodeZones::FILE_IO, "Reading ADT Obj%d. Filedata ID: %d."
              , static_cast<std::uint8_t>(lod_level), _file_data_id);
    LogIndentScoped;

    RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
    RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");

    std::uint32_t chunk_counter = 0;
    while (!buf.IsEof())
    {
      auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

      // Common chunks
      if (chunk_header.fourcc == ChunkIdentifiers::ADTCommonChunks::MVER)
      {
        Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> version{};
        version.Read(buf, chunk_header.size);
        EnsureF(CCodeZones::FILE_IO, static_cast<std::uint32_t>(version) == 18, "Version must be 18.");
        continue;
      }

      if (this->template InvokeExistingCommonReadFeatures
        < &ADTLodMapObjectBatches::Read
        , &ADTDoodadsetOverrides::Read
        , &AdtObj0SpecificData<client_version>::Read
        , &AdtObj1SpecificData<client_version>::Read
        >(buf, chunk_header, chunk_counter))
      {
        continue;
      }

      buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
      LogError("Encountered unknown ADT Obj1 chunk %s.", Common::FourCCToStr(chunk_header.fourcc).c_str());
    }

    EnsureF(CCodeZones::FILE_IO
            , (lod_level == ADTObjLodLevel::NORMAL && chunk_counter == Common::WorldConstants::CHUNKS_PER_TILE)
              || lod_level == ADTObjLodLevel::LOD, "Expected to read exactly 256 chunks, got %d.", chunk_counter);

    // fix filename references
    if constexpr (lod_level == ADTObjLodLevel::NORMAL && client_version < Common::ClientVersion::BFA)
    {
      PatchObjectFilenameReferences();
    }
  }

  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  inline void ADTObj<client_version, lod_level>::Write(Common::ByteBuffer& buf) const
  {
    LogDebugF(LCodeZones::FILE_IO, "Writing ADT Obj%d. Filedata ID: %d"
              , static_cast<std::uint8_t>(lod_level), _file_data_id);
    LogIndentScoped;

    Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> version{18};
    version.Write(buf);

    this->template InvokeExistingCommonWriteFeatures
      <
        &AdtObj0SpecificData<client_version>::Write
        , &AdtObj1SpecificData<client_version>::Write
        , &ADTLodMapObjectBatches::Write
        , &ADTDoodadsetOverrides::Write
      >(buf);
  }

  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  void ADTObj<client_version, lod_level>::PatchObjectFilenameReferences()
  requires (lod_level == ADTObjLodLevel::NORMAL)
  {
    InvariantF(CCodeZones::FILE_IO, this->_model_filename_offsets.IsInitialized()
      && this->_model_filenames.IsInitialized()
      && this->_map_object_filename_offsets.IsInitialized()
      && this->_map_object_filenames.IsInitialized()
      , "Essential chunks MMDX, MMID, MWMO, MWID were not read from file.");

    InvariantF(CCodeZones::FILE_IO, this->_model_filename_offsets.Size() == this->_model_filenames.Size()
      && this->_map_object_filename_offsets.Size() == this->_map_object_filenames.Size()
      , "Filename storage should match with offsets map in size.");

    // M2
    PatchObjectFilenameReferences_detail(this->_model_filename_offsets, this->_model_filenames
                                         , this->_model_placements);


    // WMO
    PatchObjectFilenameReferences_detail(this->_map_object_filename_offsets, this->_map_object_filenames
                                         , this->_map_object_placements);

  }

  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  template<Common::Concepts::DataArrayChunkProtocol FilepathOffsetStorage
            , Common::Concepts::StringBlockChunkProtocol FilepathStorage
            , Common::Concepts::DataArrayChunkProtocol InstanceStorage
          >
  void ADTObj<client_version, lod_level>::PatchObjectFilenameReferences_detail(FilepathOffsetStorage& offset_storage
                                                                               , FilepathStorage& filepath_storage
                                                                               , InstanceStorage& instance_storage)
  requires (lod_level == ADTObjLodLevel::NORMAL)
  {
    for(auto&& [i, model_placement] : future::enumerate(instance_storage))
    {
      EnsureF(CCodeZones::FILE_IO, !model_placement.flags.use_filedata_id
              , "Filedata ID loading is not supported for this client version.");

      if (offset_storage[model_placement.name_id]
          != filepath_storage[model_placement.name_id].first)
      {
        std::uint32_t offset = offset_storage[model_placement.name_id];

        auto it = std::find_if(filepath_storage.cbegin(), filepath_storage.cend()
                               , [offset](auto& pair) -> bool { return pair.first == offset; });

        EnsureF(CCodeZones::FILE_IO, it != filepath_storage.cend(), "Offset referenced not found. Corrupted file.");
        model_placement.name_id = static_cast<std::uint32_t>(std::distance(filepath_storage.cbegin(), it));
      }
    }
  }

  template<Common::ClientVersion client_version>
  inline AdtObj0SpecificData<client_version>::AdtObj0SpecificData()
  {
    _model_placements.Initialize();
    _map_object_placements.Initialize();
  }

  template<Common::ClientVersion client_version>
  bool AdtObj0SpecificData<client_version>::Read(Common::ByteBuffer const& buf
                                                 , Common::ChunkHeader const& chunk_header
                                                 , std::uint32_t& chunk_counter)
  {
    // common
    switch (chunk_header.fourcc)
    {
      case ChunkIdentifiers::ADTObj0Chunks::MCNK:
      LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCNK (obj0) (%d / 255), size: %d."
                , chunk_counter, chunk_header.size);
        _chunks[chunk_counter++].Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj0Chunks::MDDF:
        _model_placements.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj0Chunks::MODF:
        _map_object_placements.Read(buf, chunk_header.size);
        return true;
    }

    if (this->template InvokeExistingTraitFeature<&ADTObj0ModelStorageFilepath::Read>(buf, chunk_header))
      return true;

    return false;
  }

  template<Common::ClientVersion client_version>
  void AdtObj0SpecificData<client_version>::Write(Common::ByteBuffer& buf) const
  {
    this->template InvokeExistingTraitFeature<&ADTObj0ModelStorageFilepath::Write>(buf);
    _model_placements.Write(buf);
    _map_object_placements.Write(buf);

    for (std::size_t i = 0; i < Common::WorldConstants::CHUNKS_PER_TILE; ++i)
    {
      LogDebugF(LCodeZones::FILE_IO, "Writing chunk: MCNK (obj0) (%d / 255).", i);
      _chunks[i].Write(buf);
    }

  }

  template<Common::ClientVersion client_version>
  inline AdtObj1SpecificData<client_version>::AdtObj1SpecificData()
  {
    _lod_map_object_placements.Initialize();
    _lod_map_object_extents.Initialize();
    _lod_model_placements.Initialize();
    _lod_mapping.Initialize();
  }

  template<Common::ClientVersion client_version>
  bool AdtObj1SpecificData<client_version>::Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
  {
    switch (chunk_header.fourcc)
    {
      case ChunkIdentifiers::ADTObj1Chunks::MLMD:
        _lod_map_object_placements.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLMX:
        _lod_map_object_extents.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLDD:
        _lod_model_placements.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLDX:
        _lod_model_extents.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLDL:
        _lod_model_unknown.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLFD:
        _lod_mapping.Read(buf, chunk_header.size);
        return true;
    }

    if (this->template InvokeExistingTraitFeature<&LodModelBatches::Read>(buf, chunk_header))
      return true;

    return false;
  }

  template<Common::ClientVersion client_version>
  void AdtObj1SpecificData<client_version>::Write(Common::ByteBuffer& buf) const
  {
    InvariantF(CCodeZones::FILE_IO, _lod_map_object_placements.IsInitialized()
                                    && _lod_map_object_extents.IsInitialized()
                                    && _lod_model_placements.IsInitialized()
                                    && _lod_model_extents.IsInitialized()
                                    && _lod_mapping.IsInitialized()
               , "Essential chunk(s) not initialized.");

    _lod_map_object_placements.Write(buf);
    _lod_map_object_extents.Write(buf);
    _lod_model_placements.Write(buf);
    _lod_model_extents.Write(buf);
    _lod_mapping.Write(buf);

    if (_lod_model_unknown.IsInitialized())
    {
      _lod_model_unknown.Write(buf);
    }
  }
}
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

      // bfa+
      if constexpr (client_version >= Common::ClientVersion::BFA)
      {
        if (chunk_header.fourcc == ChunkIdentifiers::ADTObjCommonChunks::MLMB)
        {
          this->_lod_map_object_batches.Read(buf, chunk_header.size);
          continue;
        }
      }

      // sl+
      if constexpr (client_version >= Common::ClientVersion::SL)
      {
        switch (chunk_header.fourcc)
        {
          case ChunkIdentifiers::ADTObjCommonChunks::MWDS:
          {
            this->_wmo_doodadset_overrides.Read(buf, chunk_header.size);
            continue;
          }
          case ChunkIdentifiers::ADTObjCommonChunks::MWDR:
          {
            this->_wmo_doodadset_overrides_ranges.Read(buf, chunk_header.size);
            continue;
          }
        }
      }

      // handle the obj0-specific chunks here
      if constexpr (lod_level == ADTObjLodLevel::NORMAL)
      {
        if (ReadObj0SpecificChunk(buf, chunk_header, chunk_counter))
          continue;
      }
      else
      // handle the obj1-specific chunks here
      {
        if (ReadObj1SpecificChunk(buf, chunk_header))
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

    // handle obj0 specific chunks
    if constexpr (lod_level == ADTObjLodLevel::NORMAL)
    {
      WriteObj0SpecificChunks(buf);
    }
      // handle obj1 specific chunks
    else
    {
      WriteObj1SpecificChunks(buf);
    }

    // handle other common chunks
    if constexpr(client_version >= Common::ClientVersion::SL)
    {
      this->_wmo_doodadset_overrides_ranges.Write(buf);
      this->_wmo_doodadset_overrides.Write(buf);
      this->_lod_map_object_batches.Write(buf);
    }
  }


  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  inline bool ADTObj<client_version, lod_level>::ReadObj0SpecificChunk(Common::ByteBuffer const& buf
                                                           , Common::ChunkHeader const& chunk_header
                                                           , std::uint32_t& chunk_counter)
  requires (lod_level == ADTObjLodLevel::NORMAL)
  {
    // common
    switch (chunk_header.fourcc)
    {
      case ChunkIdentifiers::ADTObj0Chunks::MCNK:
        LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCNK (obj0) (%d / 255), size: %d."
                  , chunk_counter, chunk_header.size);
        this->_chunks[chunk_counter++].Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj0Chunks::MDDF:
        this->_model_placements.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj0Chunks::MODF:
        this->_map_object_placements.Read(buf, chunk_header.size);
        return true;
    }

    if constexpr (client_version < Common::ClientVersion::BFA)
    {
      switch(chunk_header.fourcc)
      {
        case ChunkIdentifiers::ADTObj0Chunks::MMDX:
          this->_model_filenames.Read(buf, chunk_header.size);
          return true;
        case ChunkIdentifiers::ADTObj0Chunks::MMID:
          this->_model_filename_offsets.Read(buf, chunk_header.size);
          return true;
        case ChunkIdentifiers::ADTObj0Chunks::MWMO:
          this->_map_object_filenames.Read(buf, chunk_header.size);
          return true;
        case ChunkIdentifiers::ADTObj0Chunks::MWID:
          this->_map_object_filename_offsets.Read(buf, chunk_header.size);
          return true;
      }
    }

    return false;
  }

  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  inline void ADTObj<client_version, lod_level>::WriteObj0SpecificChunks(Common::ByteBuffer& buf) const
  requires (lod_level == ADTObjLodLevel::NORMAL)
  {
    if constexpr (client_version < Common::ClientVersion::BFA)
    {
      // contracts
      {
        InvariantF(CCodeZones::FILE_IO, this->_model_filename_offsets.IsInitialized()
                                        && this->_model_filenames.IsInitialized()
                                        && this->_map_object_filename_offsets.IsInitialized()
                                        && this->_map_object_filenames.IsInitialized()
                   , "Essential chunks MMDX, MMID, MWMO, MWID are not initialized.");

        InvariantF(CCodeZones::FILE_IO, this->_model_filename_offsets.Size() == this->_model_filenames.Size()
                   && this->_map_object_filename_offsets.Size()== this->_map_object_filenames.Size()
                   , "Filename storage should match with offsets map in size.");


        InvariantF(CCodeZones::FILE_IO
                   , this->_model_placements.IsInitialized() && this->_map_object_placements.IsInitialized()
                   , "Model and map object placements must be initialized.");

      }

      this->_model_filenames.Write(buf);
      this->_model_filename_offsets.Write(buf);
      this->_map_object_filenames.Write(buf);
      this->_map_object_filename_offsets.Write(buf);
    }

    this->_model_placements.Write(buf);
    this->_map_object_placements.Write(buf);

    for (std::size_t i = 0; i < Common::WorldConstants::CHUNKS_PER_TILE; ++i)
    {
      LogDebugF(LCodeZones::FILE_IO, "Writing chunk: MCNK (obj0) (%d / 255).", i);
      this->_chunks[i].Write(buf);
    }
  }

  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  inline bool ADTObj<client_version, lod_level>::ReadObj1SpecificChunk(Common::ByteBuffer const& buf
                                                                       , Common::ChunkHeader const& chunk_header)
  requires (lod_level == ADTObjLodLevel::LOD)
  {
    switch (chunk_header.fourcc)
    {
      case ChunkIdentifiers::ADTObj1Chunks::MLMD:
        this->_lod_map_object_placements.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLMX:
        this->_lod_map_object_extents.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLDD:
        this->_lod_model_placements.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLDX:
        this->_lod_model_extents.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLDL:
        this->_lod_model_unknown.Read(buf, chunk_header.size);
        return true;
      case ChunkIdentifiers::ADTObj1Chunks::MLFD:
        this->_lod_mapping.Read(buf, chunk_header.size);
        return true;
    }

    if constexpr (client_version >= Common::ClientVersion::SL)
    {
      if (chunk_header.fourcc == ChunkIdentifiers::ADTObj1Chunks::MLDB)
      {
        this->_lod_model_batches.Read(buf, chunk_header.size);
        return true;
      }
    }

    return false;
  }

  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  void ADTObj<client_version, lod_level>::WriteObj1SpecificChunks(Common::ByteBuffer& buf) const
  requires (lod_level == ADTObjLodLevel::LOD)
  {
    InvariantF(CCodeZones::FILE_IO, this->_lod_map_object_placements.IsInitialized()
                                    && this->_lod_map_object_extents.IsInitialized()
                                    && this->_lod_model_placements.IsInitialized()
                                    && this->_lod_model_extents.IsInitialized()
                                    && this->_lod_mapping.IsInitialized()
              , "Essential chunk(s) not initialized.");

    this->_lod_map_object_placements.Write(buf);
    this->_lod_map_object_extents.Write(buf);
    this->_lod_model_placements.Write(buf);
    this->_lod_model_extents.Write(buf);
    this->_lod_mapping.Write(buf);

    if (this->_lod_model_unknown.IsInitialized())
    {
      this->_lod_model_unknown.Write(buf);
    }

    if constexpr (client_version >= Common::ClientVersion::SL)
    {
      this->_lod_model_batches.Write(buf);
    }
  }

  template<Common::ClientVersion client_version, ADTObjLodLevel lod_level>
  template<Common::ClientVersion client_v>
  inline void ADTObj<client_version, lod_level>::GenerateLod(ADTObj<client_v, ADTObjLodLevel::NORMAL> const& tile_obj)
  requires (lod_level == ADTObjLodLevel::LOD)
  {
    // TODO: generate LOD data here
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
  inline AdtObj1SpecificData<client_version>::AdtObj1SpecificData()
  {
    _lod_map_object_placements.Initialize();
    _lod_map_object_extents.Initialize();
    _lod_model_placements.Initialize();
    _lod_mapping.Initialize();
  }
}
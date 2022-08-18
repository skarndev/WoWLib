#pragma once
#include <IO/ADT/Obj/ADTObj.hpp>
#include <Utils/Meta/Future.hpp>

namespace IO::ADT
{
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
  template<Common::Concepts::DataArrayChunkProtocol FilepathOffsetStorage, Common::Concepts::StringBlockChunkProtocol FilepathStorage, Common::Concepts::DataArrayChunkProtocol InstanceStorage
  >
  void ADTObj<client_version, lod_level>::PatchObjectFilenameReferences_detail(FilepathOffsetStorage& offset_storage
                                                                               , FilepathStorage& filepath_storage
                                                                               , InstanceStorage& instance_storage)
  requires (lod_level == ADTObjLodLevel::NORMAL)
  {
    for (auto&& [i, model_placement]: future::enumerate(instance_storage))
    {
      EnsureF(CCodeZones::FILE_IO, !model_placement.flags.use_filedata_id
              , "Filedata ID loading is not supported for this client version.");

      if (offset_storage[model_placement.name_id]
          != filepath_storage[model_placement.name_id].first)
      {
        std::uint32_t offset = offset_storage[model_placement.name_id];

        auto it = std::find_if(filepath_storage.cbegin(), filepath_storage.cend()
                               , [offset](auto& pair) -> bool
          { return pair.first == offset; });

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
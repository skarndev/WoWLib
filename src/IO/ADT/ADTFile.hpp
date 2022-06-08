#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

#include <unordered_map>
#include <cstdint>
#include <functional>
#include <fstream>

namespace IO::ADT
{
  class ADTFile
  {
  public:
    ADTFile(std::uint32_t file_data_id);


  private:
   
    //std::unordered_map<ChunkIdentifiers::ADTTexChunks::eADTTexChunks, Common::IDataChunk*> _tex_chunks;
    //std::unordered_map<ChunkIdentifiers::ADTObj0Chunks::eADTObj0Chunks, Common::IDataChunk*> _obj0_chunks;
    //std::unordered_map<ChunkIdentifiers::ADTObj1Chunks::eADTObj1Chunks, Common::IDataChunk*> _obj1_chunks;
    //std::unordered_map<ChunkIdentifiers::ADTLodChunks::eADTLodChunks, Common::IDataChunk*> _lod_chunks;

    // common
  
    /*
    // tex
    Common::DataChunk<std::uint32_t> _diffuse_texture_ids;
    Common::DataChunk<std::uint32_t> _height_texture_ids;
    Common::DataChunk<DataStructures::SMTextureFlags> _texture_flags;
    Common::DataChunk<DataStructures::SMTextureParams> _texture_params;
    Common::DataChunk<DataStructures::MTCG> _texture_color_grading;
    Common::DataChunk<char> _texture_amplifier;
    // todo: mcnks

    // obj0
    Common::DataChunk<DataStructures::MODF> _wmo_instances;
    Common::DataChunk<DataStructures::MDDF> _model_instances;
    // todo: mcnks
    Common::DataChunk<DataStructures::MLMB> _mlmb; // TODO: what is this one for?
    Common::DataChunk<DataStructures::MWDR> _wmo_doodadset_ranges;
    Common::DataChunk<std::uint16_t> _wmo_dooodad_set_indices;

    // obj1
    // todo: mcnks
    Common::DataChunk<DataStructures::MLMD> _wmo_instanced_lod;
    Common::DataChunk<DataStructures::MLMX> _wmo_instances_lod_extents;
    Common::DataChunk<DataStructures::MDDF> _model_instances_lod; // same structure as MDDF
    Common::DataChunk<DataStructures::MLDX> _model_instances_lod_extents;
    Common::DataChunk<std::uint32_t> _mldl;  // TODO: what is this one for?
    Common::DataChunk<DataStructures::MLFD> _object_lod_levels;
    Common::DataChunk<DataStructures::MLMB> _mlmb_obj1; // TODO: what is this one for?
    Common::DataChunk<char> _mldb; // TODO: what is this one for?
    Common::DataChunk<DataStructures::MWDR> _wmo_doodadset_ranges_lod;
    Common::DataChunk<std::uint16_t> _wmo_doodad_set_indices_lod;
    */
  };
}
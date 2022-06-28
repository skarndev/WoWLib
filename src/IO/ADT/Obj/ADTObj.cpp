#include <IO/ADT/Obj/ADTObj.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

using namespace IO::ADT;


AdtObj0SpecificData::AdtObj0SpecificData()
{
  _model_placements.Initialize();
  _map_object_placements.Initialize();
}

AdtObj1SpecificData::AdtObj1SpecificData()
{
  // TODO: verify which one of these are essential
  _lod_map_object_placements.Initialize();
  _lod_map_object_extents.Initialize();
  _lod_model_placements.Initialize();
  _lod_model_extents.Initialize();
  _lod_model_unknown.Initialize();
  _lod_mapping.Initialize();
  _map_object_lod_batches.Initialize();
}

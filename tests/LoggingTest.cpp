#include <Validation/Log.hpp>
#include <Config/CodeZones.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <backward.hpp>
#include <cstdlib>
#include <cassert>
#include <cstdint>

int main()
{
  backward::SignalHandling sh;
  Validation::Log::InitLoggers();
  LogDebug("Standard debug log %s, %d.", "123", 1);

  {
    LogIndentScoped;
    LogDebugV("Verbose debug log.");
    LogError("Standard error log.");
    LogErrorV("Verbose error log.");

    {
      LogIndentScoped;
      Log("Default log.");
      LogErrorV("Another verbose error log.");

      {
        LogIndentScoped;
        LogErrorV("Another deeply indented verbose error log.");
      }
    }

  }

  Log("Log return to scope 0.");
 

  LogDebugVF(LCodeZones::GRAPHICS, "This is a graphics debug %s", "print.");

  std::uint32_t fourcc_int = IO::ADT::ChunkIdentifiers::ADTCommonChunks::MVER;
  char* fourcc = reinterpret_cast<char*>(&fourcc_int);
  LogDebug("Encountered unknown chunk %c%c%c%c.", fourcc[0], fourcc[1], fourcc[2], fourcc[3]);

  return 0;
}
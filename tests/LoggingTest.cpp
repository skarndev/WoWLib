#include <Validation/Log.hpp>
#include <Config/CodeZones.hpp>
#include <backward.hpp>
#include <cstdlib>
#include <cassert>

int main()
{
  backward::SignalHandling sh;
  Validation::Log::InitLoggers();
  LogDebug("Standard debug log %s, %d.", "123", 1);
  LogDebugV("Verbose debug log.");
  LogError("Standard error log.");
  LogErrorV("Verbose error log.");
  Log("Default log.");

  LogDebugVF(LCodeZones::GRAPHICS, "This is a graphics debug %s", "print.");

  // not supposed to work here, as network debug prints are not enabled in Cmake
  // compiles to empty body function
  LogDebugF(LCodeZones::NETWORK, "This is a network debug print.");

  assert(false);

  return 0;
}
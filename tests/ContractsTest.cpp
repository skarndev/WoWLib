#include <Validation/Contracts.hpp>
#include <Config/CodeZones.hpp>
#include <StormLib.h>

#if defined(_MSC_VER) & !defined(__INTEL_COMPILER)
  #pragma warning(push)
  #pragma warning(disable : 4267)
  #pragma warning(disable : 4996)

  #include <backward.hpp>

  #pragma warning(pop) 
#else
  #include <backward.hpp>
#endif


int ContractedFunctionExample(int x, int y)
{
  RequireM((y == 0, x == 1), "Not supposed to abort here."); // example using multiple conditions
  Require([=]() { return y != 0; }, "test"); // alternative multiliner with lambda, and perhaps complex logic
  RequireF(CCodeZones::FILE_IO, y != 0, "test"); // for one-liners use expr
  return x / y;
}

int main()
{
  backward::SignalHandling sh;
  SFileOpenArchive(nullptr, 0, 0, nullptr);
  ContractedFunctionExample(1, 0);
  return 0;
}
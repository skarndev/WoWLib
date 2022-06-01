#include <Validation/Contracts.hpp>
#include <Config/CodeZones.hpp>

int ContractedFunctionExample(int x, int y)
{
  RequireF(CCodeZones::FILE_IO, y != 0, "test");
  return x / y;
}

int main()
{
  ContractedFunctionExample(1, 0);
  return 0;
}
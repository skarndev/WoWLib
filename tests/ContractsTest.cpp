#include <Validation/Contracts.hpp>
#include <Config/CodeZones.hpp>

int ContractedFunctionExample(int x, int y)
{
  RequireF(CCodeZones::FILE_IO, [=]() { return y != 0; }, "test"); // for multi-liners used lambda
  RequireF(CCodeZones::FILE_IO, y != 0, "test"); // for one-liners use expr
  return x / y;
}

int main()
{
  ContractedFunctionExample(1, 0);
  return 0;
}
# CODING GUIDELINES #
File naming rules:

```.hpp``` - is used for header files (C++ language).

```.h``` - is used **only** for header files or modules written in C language.

```.c``` - is used **only** for implementation files or modules written in C language.

```.cpp``` - is used for project implementation files. 

```.inl``` - is used for include files providing template instantiations. .inl files are to be included at the end of the corresponding .hpp header file.


### Project structure: ###

```/src``` - is the main directory hosting .cpp, .hpp, .inl files of the project.

File names should use PascalCase (e.g. ```FooBan.hpp```) and either correspond to the type defined in the file,
or represent sematics of the module.

```/external``` - is the directory of hosting included libraries and subprojects. This is external or modified
external code, so no style rules from the project apply to its content.

```/src/shaders``` - is the directory to store .hlsl shaders for the DirectX renderers. It is not recommended, 
but not strictly prohibited to inline shader code as strings to ```.cpp``` implementation files.


### Code style ###

Following is an example for file `src/ModuleName/FooBan.hpp`. 

```cpp
#pragma once
// We use #pragma once in all headers instead of include guards, as the project is Windows oriented
// and does not strive to be cross-platform due to its specific nature.

// <> are prefered for includes. Relative including is not allowed.
// Local imports go here
#include <SomeLocalFile.hpp>

// Lib imports go here
#include <external/SomeLibCode.hpp>

// STL imports go here
#include <string>
#include <mutex>
#include <cstdint>
#include <vector> // etc

// Forward declarations in headers instead of includes are encouraged. That prevents type leaking into bigger scopes
// as well as reduces compile time
namespace Parent::SomeOtherChild
{
  class ForwardDeclaredClass;
}

// Namespaces are defined as PascalCase names. Namespace concatenation for nested namespaces
// is adviced, but not strictly enforced.
namespace Parent::Child
{
  // types are name in PascalCase,
  class Test : public TestBase
  {
    public:
      Test();
     
      int x; // -> wrong!
      // public fields like that are discourged, use getter/setter interface.
      
      // methods and freestanding functions are also named in PascalCase to keep consistency with DirectX and Dear ImGui naming conventions.
      // trivial one-liner getters must be declared in the header file to not pollute the implementation files.
      int SomePrivateMember() { return _some_private_member; } const; // const correctness is encourage on method definition level

      // trivial one-liner setters without other side effects other than setting the corresponding value must be declared in the header file. Preceded by "Set" prefix.
      void SetSomePrivateMember(int a) { _some_private_member = a; };
    
    // private members are snake lower case, separated by underscore, preceded by underscore to indicate they're private.
    private:
      std::int32_t _some_private_member; // std:: prefix is required for all STL types, no using namespace std 
      // or relying on them being global by default due to MSVC specifics. That ensures easier reuse of code components in other projects that may be cross-platform.
      
      ForwardDeclaredClass* _some_other_private_member_using_forward_decl; // "_" prefix must indicate the private nature of the field. Member fields must use snake_case.
      std::mutex _mutex;

    // static methods are separate away from non-static methods.
    // the use of static methods is encouraged if a static function is strictly sematically bound to the nature of the type.
    // for general-purpose functions use free-standing definitions enclosed in a namespace

    private:
      static void _SomeStaticMethod(); // private methods are prefixed with _
    
  };
}

#endif
```

Following is an example for file `src/ModuleName/FooBan.cpp`.

```cpp
// the header of this .cpp comes first
// <> are prefered for includes.
#include <src/ModuleName/FooBan.hpp>

// same order of includes as in the header file.

using namespace Parent::Child;

Test::Test()
: TestBase("some_arg")
, _some_private_member(0)
, _some_other_private_member_using_forward_decl(new ForwardDeclaredClass()) // do not forget to import ForwardDeclaredClass in .cpp
{
  // body of ctor
}

void Test::SomeStaticMethod()
{
  // local variables are named in snake_case, no preceding underscore.
  int local_var = 0;
  // const int local_var = 0; Const correctness is not required for local variables, use as preferred. 

  // preceding underscore is used on variables that are used for RAII patterns, such as scoped stuff (e.g. a scoped mutex)
  std::lock_guard<std::mutex> _lock (_mutex); // _lock is never accessed later, it just needs to live as long as the scope lives.
  // So, it has an underscore prefix.

  SomeFunc(local_var); // free floating functions use the same naming rules as methods
}
```

Additional examples:

```cpp

constexpr unsigned SOME_CONSTANT = 10; // constants are named in SCREAMING_CASE
#define SOME_MACRO // macro definitions are named in SCREAMING_CASE

// usage of enum class for enumerated constants which are integer in nature is discouraged due to inability to implicitly being cast to integers
// in order to prevent leaking regular enum members into the outer namespace use the following pattern

namespace MyEnum
{
  enum eMyEnum
  {
    DO_SOMETHING = 0x1,
    DO_SOMETHING_ELSE = 0x2
  };
}

// access as follows
MyEnum::DO_SOMETHING
// this way of access is considered as redunancy
MyEnum::eMyEnum::DO_SOMETHING

// If enumeration does not represent an integer type in its sematic nature and is only used to distinguish one thing from another,
// prefer enum class declarations.
// Example:

enum class GrowthPolicy
{
  STRICT,
  SMART
}

template<GrowthPolicy policy = GrowthPolicy::SMART>
void SomeFunc()
{
  if constexpr (policy == GrowthPolicy::SMART)
  {
    // ...
  }
  else
  {
    // ...
  }
}

// WoW fileformats often utilize bit flags in different data structures.
// The ultimate form of the flag implementations preferred is as follows:

namespace Flags
{
    enum eFlags : std::uint32_t
    {
        DO_SOMETHING = 0x1,
        DO_SOMETHING_ELSE = 0x2
    };
}

union FlagsAccessor
{
    struct
    {
        std::uint32_t do_something : 1;
        std::uint32_t do_something_else : 1;
        std::uint32_t _unk : 30;
    } params;

    std::uint32_t value;
};

FlagsAccessor flags;
flags.params.do_something = true;
flags.value & Flags::DO_SOMETHING_ELSE;

```

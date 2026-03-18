# simply-cpp

Library to simplify a variety of C++ libraries into a standard usage methodology.

The idea of this library is to make it easier to use C++, especially for those who are new to C++.
The concept is to create wrappers around existing libraries, not to implement them from scratch.
This should keep maintenance to the minimum.

In my opinion, ease of use means that there should not be a need to maintain state or have a lot of work to do for setting up.
All functionality is run from a global or static function if no state is required. Otherwise by creating an object with a simple constructor and methods.
Most objects should be able to be streamed as a string. (eg cout << obj).

Example for a static function of the "base64" class:

```cpp
sc::base64::encode("Hello World!");
```

Example for using a simple class:

```cpp
sc::timer t, t2;
// Do some work
cout << "Some work took " << t << endl;
t2.reset();
// Do more work
t.stop();
t2.stop();
// Some cleanup that we don't care about the timing
cout << "More work took " << t2 << endl;
cout << "Some work and more work together took " << t << endl;
```

## Factors considered to keep things simple:

1. Headers should not include third party headers so that users don't have issues with needing 3rd party headers or include paths. (only use forward declarations)
1. Headers should not affect the global namespace.
1. Headers should not add namespace usages.
1. Headers should include comments to document the public methods.

## Usage

Every library can be used on its own, or you can include the common library to include everything to get going.

### CMake

1. You can use "FetchContent" to include the library in your CMake project:

```cmake
include(FetchContent)
FetchContent_Declare(
        sc
        GIT_REPOSITORY https://github.com/roelofrossouw/simply-cpp.git
        GIT_TAG origin/main # Or a specific tag to stay stable #
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(sc)

add_executable(trysc main.cpp)
target_link_libraries(trysc sc)
```

2. Or you can simply clone the repository and then include the library in your CMake project:

```bash
# If your project is a git repo, then
git remote add sc https://github.com/roelofrossouw/simply-cpp.git
git subtree add --prefix=third_party/sc sc main --squash
# otherwise you can clone it
git clone https://github.com/roelofrossouw/simply-cpp.git third_party/sc
```

```cmake
add_subdirectory(third_party/sc sc EXCLUDE_FROM_ALL)
```

Once you have simply-cpp included in your project include the header file:

```cpp
#include "sc.h"
```

And access any part of the library:

```cpp
auto encoded = sc::base64::encode("Hello");
```

## Requirements
### Base requirements for compiling
1. CMake – install with your package manager or download from https://cmake.org/download/
1. A C++ compiler that supports C++17 or higher.
### Third party library dependencies
Ideally dependencies are included and build as source, but in some cases it is too complicated when e.g. requirements don't use cmake etc.
These dependencies are however mostly only required for building, most systems should have these libraries already installed for end users.
1. Curl – install with your package manager (e.g. `apt -y install libcurl4-openssl-dev`) or download from https://curl.haxx.se/download.html

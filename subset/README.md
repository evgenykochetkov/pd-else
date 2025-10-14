Compiles a subset of ELSE externals into a single static library to use with libpd
**excluding** GPL-licensed and/or heavyweight externals. See the full list with reasons and explanations in ./picked_externals.cmake.

**Note:** You are responsible for managing license files and abstractions manually.

## Usage example

```cmake
set(PD_MULTI ON CACHE BOOL "") # this will affect both libpd and pd-else/subset
add_subdirectory(libpd)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libpd/cpp)

set(PD_SOURCES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libpd/pure-data/src" CACHE STRING "")
add_subdirectory(pd-else/subset)

# ...

target_link_libraries(
  your-target
  PRIVATE
    libpd_static
    else_subset
    # ...
)
```

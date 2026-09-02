# Lunaris Event Pool Library

This is a pool library that you can add on your project! It works on both Windows and Linux.

There are some tests to check if it builds correctly and they can be disabled with `BUILD_TESTS OFF`.

## How to add the project to your project

### Using FetchContent

You can create a file like `cmake/installLibrary.cmake` and put in there:

```cmake
include(FetchContent)

FetchContent_Declare(
    lunaris-event-pool
    GIT_REPOSITORY https://github.com/LunarisLib/Lunaris-Event-Pool.git
    GIT_TAG        (put version here)
)
FetchContent_MakeAvailable(lunaris-event-pool)
```

This will allow you to download and link the library like:

```cmake
# ...

include(cmake/installLibrary.cmake) # does the FetchContent

target_link_libraries(YourProjectName PRIVATE
    lunaris::lunaris-event-pool
)
```

### Using find_package()

If you get the install version with the lib and headers and want to avoid recompiling the library yourself, you can do

```cmake
# ...

find_package(lunaris-event-pool REQUIRED)

target_link_libraries(YourProjectName PRIVATE
    lunaris::lunaris-event-pool
)
```

The find_package will try to find the `lunaris-event-pool-config.cmake` or similar files that should be available to download in the Release tab.
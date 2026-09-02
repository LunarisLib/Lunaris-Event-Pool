cmake_minimum_required(VERSION 3.19)

project(lunaris-event-pool
    DESCRIPTION "A simple event pool library"
    VERSION "1"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE PROJECT_SOURCES
     CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

message(STATUS "Files used for compilation of lunaris::${PROJECT_NAME}:")
foreach(file IN LISTS PROJECT_SOURCES)
    message(STATUS " - ${file}")
endforeach()

add_library(${PROJECT_NAME})
add_library(lunaris::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:LUNARIS_DEBUG>
        $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0A00>
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
)

option(BUILD_TESTS "Build tests" ON)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    if (BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()

    include(cmake/install.cmake)
endif()
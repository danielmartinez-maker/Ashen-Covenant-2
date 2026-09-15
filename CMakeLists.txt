cmake_minimum_required(VERSION 3.28)
project(AshenCovenant2 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.12
    GIT_SHALLOW TRUE)
FetchContent_Declare(EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v4.0.0
    GIT_SHALLOW TRUE)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.15.3
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(SDL3 EnTT Catch2)

function(ac2_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    endif()
endfunction()

add_library(ac2_core STATIC
    src/core/FixedStepClock.cpp)
target_include_directories(ac2_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
ac2_enable_warnings(ac2_core)

add_library(ac2_input INTERFACE)
target_include_directories(ac2_input INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(ac2_simulation INTERFACE)
target_include_directories(ac2_simulation INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ac2_simulation INTERFACE ac2_core ac2_input EnTT::EnTT)

add_library(ac2_render_model INTERFACE)
target_include_directories(ac2_render_model INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ac2_render_model INTERFACE ac2_simulation)

add_library(ac2_platform_sdl INTERFACE)
target_include_directories(ac2_platform_sdl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ac2_platform_sdl INTERFACE ac2_input SDL3::SDL3)

add_library(ac2_render_sdlgpu INTERFACE)
target_include_directories(ac2_render_sdlgpu INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ac2_render_sdlgpu INTERFACE ac2_render_model SDL3::SDL3)

add_executable(ashen_covenant_2 src/app/main.cpp)
target_link_libraries(ashen_covenant_2 PRIVATE ac2_core ac2_simulation ac2_platform_sdl ac2_render_sdlgpu)
ac2_enable_warnings(ashen_covenant_2)

include(CTest)
if(BUILD_TESTING)
    add_executable(ac2_tests
        tests/smoke/BootstrapTests.cpp
        tests/core/FixedStepClockTests.cpp)
    target_link_libraries(ac2_tests PRIVATE Catch2::Catch2WithMain ac2_core ac2_input ac2_simulation ac2_render_model)
    ac2_enable_warnings(ac2_tests)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
    catch_discover_tests(ac2_tests)
endif()

cmake_minimum_required(VERSION 3.24)
project(AshenCovenant2 LANGUAGES CXX)

option(AC2_BUILD_TESTS "Build AC2 tests" ON)

add_library(ac2_vfx
    src/vfx/VfxCatalog.cpp
    src/vfx/VfxComposer.cpp
    src/vfx/VfxPolicy.cpp
    src/vfx/VfxValidation.cpp
)
target_include_directories(ac2_vfx PUBLIC src)
target_compile_features(ac2_vfx PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(ac2_vfx PRIVATE /W4 /permissive-)
else()
  target_compile_options(ac2_vfx PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(AC2_BUILD_TESTS)
  enable_testing()
  add_executable(ac2_vfx_tests tests/vfx/VfxTests.cpp)
  target_link_libraries(ac2_vfx_tests PRIVATE ac2_vfx)
  target_compile_features(ac2_vfx_tests PRIVATE cxx_std_20)
  add_test(NAME ac2_vfx_tests COMMAND ac2_vfx_tests)

  find_package(Python3 COMPONENTS Interpreter QUIET)
  if(Python3_Interpreter_FOUND)
    add_test(NAME ac2_vfx_catalog_validation
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/validate_vfx_catalog.py ${CMAKE_SOURCE_DIR}/assets/data/vfx/catalog.json)
  endif()
endif()

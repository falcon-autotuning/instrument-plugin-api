cmake_minimum_required(VERSION 3.20)

project(
  instrument-plugin-api
  VERSION 2.0.2
  DESCRIPTION "API for instrument plugin communication"
  HOMEPAGE_URL "https://github.com/falcon-autotuning/instrument-plugin-api"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

option(BUILD_TESTS "Enable building tests" OFF)
option(USE_CCACHE "Enable ccache or sccache if available" ON)

option(INSTRUMENT_PLUGIN_ENABLE_PLUGIN "Build plugin API" ON)
option(INSTRUMENT_PLUGIN_ENABLE_HOST "Build host API" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ------------------------------------------------------------
# Compiler launcher
# ------------------------------------------------------------
if(USE_CCACHE)
  find_program(SCCACHE_PROGRAM sccache)
  if(SCCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
  else()
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()
  endif()
endif()

if(WIN32)
  add_compile_definitions(NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()

add_library(instrument-plugin-api-core INTERFACE)

target_include_directories(instrument-plugin-api-core
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(instrument-plugin-api::core ALIAS instrument-plugin-api-core)

if(INSTRUMENT_PLUGIN_ENABLE_PLUGIN)

  add_library(instrument-plugin-api-plugin SHARED
    src/plugin-api.c
  )

  target_include_directories(instrument-plugin-api-plugin
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  target_link_libraries(instrument-plugin-api-plugin
    PUBLIC instrument-plugin-api-core
  )

  target_compile_definitions(instrument-plugin-api-plugin
    PRIVATE INSTRUMENT_PLUGIN_BUILD
  )

  set_target_properties(instrument-plugin-api-plugin
    PROPERTIES POSITION_INDEPENDENT_CODE ON
  )

  add_library(instrument-plugin-api::plugin ALIAS instrument-plugin-api-plugin)

endif()

if(INSTRUMENT_PLUGIN_ENABLE_HOST)

  add_library(instrument-plugin-api-host SHARED
    src/plugin-host.c
  )

  target_include_directories(instrument-plugin-api-host
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  target_link_libraries(instrument-plugin-api-host
    PUBLIC instrument-plugin-api-core
  )

  target_compile_definitions(instrument-plugin-api-host
    PRIVATE INSTRUMENT_PLUGIN_BUILD
  )

  set_target_properties(instrument-plugin-api-host
    PROPERTIES POSITION_INDEPENDENT_CODE ON
  )

  add_library(instrument-plugin-api::host ALIAS instrument-plugin-api-host)

endif()

# ============================================================
# Install
# ============================================================

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(EXPORT_TARGETS instrument-plugin-api-core)

if(INSTRUMENT_PLUGIN_ENABLE_PLUGIN)
  list(APPEND EXPORT_TARGETS instrument-plugin-api-plugin)
endif()

if(INSTRUMENT_PLUGIN_ENABLE_HOST)
  list(APPEND EXPORT_TARGETS instrument-plugin-api-host)
endif()

install(TARGETS ${EXPORT_TARGETS}
  EXPORT instrument-plugin-api-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT instrument-plugin-api-targets
  FILE instrument-plugin-api-targets.cmake
  NAMESPACE instrument-plugin-api::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/instrument-plugin-api
)

configure_package_config_file(
  cmake/instrument-plugin-api-config.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/instrument-plugin-api-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/instrument-plugin-api
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/instrument-plugin-api-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/instrument-plugin-api-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/instrument-plugin-api-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/instrument-plugin-api
)

# ============================================================
# Tests
# ============================================================

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

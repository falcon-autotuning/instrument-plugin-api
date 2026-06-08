cmake_minimum_required(VERSION 3.20)

project(
  instrument-plugin-api
  VERSION 1.0.5
  DESCRIPTION "The API that the instrument-script-server uses to communicate with instruments implemented by users"
  HOMEPAGE_URL "https://github.com/falcon-autotuning/instrument-plugin-api"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
add_library(instrument-plugin-api INTERFACE)

target_include_directories(instrument-plugin-api
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS instrument-plugin-api
    EXPORT instrument-plugin-api-targets
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

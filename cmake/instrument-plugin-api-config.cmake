include("${CMAKE_CURRENT_LIST_DIR}/instrument-plugin-api-targets.cmake")

function(add_instrument_plugin TARGET_NAME)
  set(options "")
  set(oneValueArgs "")
  set(multiValueArgs SOURCES LINK_LIBRARIES INCLUDE_DIRS)

  cmake_parse_arguments(PLUGIN "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  if(NOT PLUGIN_SOURCES)
    message(FATAL_ERROR "add_instrument_plugin: SOURCES argument is required")
  endif()

  # Create plugin module
  add_library(${TARGET_NAME} MODULE ${PLUGIN_SOURCES})

  set_target_properties(${TARGET_NAME}
    PROPERTIES
      PREFIX ""
      POSITION_INDEPENDENT_CODE ON
  )

  # Include user-provided includes
  target_include_directories(${TARGET_NAME}
    PRIVATE
      ${PLUGIN_INCLUDE_DIRS}
  )

  target_link_libraries(${TARGET_NAME}
    PRIVATE
      instrument-plugin-api::plugin
      ${PLUGIN_LINK_LIBRARIES}
  )

  # Windows export macro
  target_compile_definitions(${TARGET_NAME}
    PRIVATE INSTRUMENT_PLUGIN_BUILD
  )

  message(STATUS "Configured instrument plugin: ${TARGET_NAME}")
endfunction()

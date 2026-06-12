find_package(cmocka REQUIRED)

add_executable(test_plugin_response
  test-storage.c
)

target_link_libraries(test_plugin_response
  PRIVATE
    cmocka::cmocka
    instrument-plugin-api::plugin
    instrument-plugin-api::host
)

target_include_directories(test_plugin_response
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

add_test(NAME test_plugin_response
  COMMAND test_plugin_response
)

#pragma once

/**
 * @file instrument_plugin.h
 * @brief C API for implementing instrument plugins.
 *
 * This header defines the ABI contract between the instrument script server
 * and dynamically loaded plugins. Plugins must implement the required
 * functions and use the data structures defined here.
 */

#if defined(_WIN32)
#if defined(INSTRUMENT_PLUGIN_BUILD)
/// Export symbols when building plugin
#define INSTRUMENT_PLUGIN_API __declspec(dllexport)
#else
/// Import symbols when consuming plugin
#define INSTRUMENT_PLUGIN_API __declspec(dllimport)
#endif
#else
/// Default visibility for non-Windows platforms
#define INSTRUMENT_PLUGIN_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @def INSTRUMENT_PLUGIN_API_VERSION
 * @brief Version of the plugin API.
 *
 * Plugins must set PluginMetadata.api_version to this value.
 * The host will reject plugins with incompatible versions.
 */
#define INSTRUMENT_PLUGIN_API_VERSION 1

/// Maximum length for string fields (including null terminator)
#define PLUGIN_MAX_STRING_LEN 128

/// Maximum number of parameters per command
#define PLUGIN_MAX_PARAMS 16

/// Maximum payload size for text/binary responses
#define PLUGIN_MAX_PAYLOAD 4096

/**
 * @brief Supported parameter value types.
 */
typedef enum {
  PARAM_TYPE_NONE = 0,     /**< No value */
  PARAM_TYPE_INT32,        /**< 32-bit signed integer */
  PARAM_TYPE_INT64,        /**< 64-bit signed integer */
  PARAM_TYPE_UINT32,       /**< 32-bit unsigned integer */
  PARAM_TYPE_UINT64,       /**< 64-bit unsigned integer */
  PARAM_TYPE_FLOAT,        /**< 32-bit float */
  PARAM_TYPE_DOUBLE,       /**< 64-bit float */
  PARAM_TYPE_BOOL,         /**< Boolean value */
  PARAM_TYPE_STRING,       /**< Null-terminated string */
  PARAM_TYPE_BINARY,       /**< Arbitrary binary data */
  PARAM_TYPE_ARRAY_DOUBLE, /**< Array of double values */
  PARAM_TYPE_ARRAY_INT32   /**< Array of int32 values */
} PluginParamType;

/**
 * @brief Represents a typed parameter value.
 *
 * The active field in the union is determined by @ref type.
 *
 * @note Ownership rules:
 * - For pointer-based types (binary, arrays), memory is owned by the caller.
 * - Data is only valid for the duration of the API call.
 * - Plugins must NOT free or retain these pointers.
 */
typedef struct {
  PluginParamType type; /**< Type of the value */

  union {
    int32_t i32_val;
    int64_t i64_val;
    uint32_t u32_val;
    uint64_t u64_val;
    float f_val;
    double d_val;
    bool b_val;

    /// Null-terminated string
    char str_val[PLUGIN_MAX_STRING_LEN];

    /// Binary data buffer
    struct {
      uint8_t *data; /**< Pointer to data */
      size_t size;   /**< Size in bytes */
    } binary;

    /// Array of doubles
    struct {
      double *data; /**< Pointer to data */
      size_t size;  /**< Number of elements */
    } array_double;

    /// Array of int32
    struct {
      int32_t *data; /**< Pointer to data */
      size_t size;   /**< Number of elements */
    } array_int32;

  } value;
} PluginParamValue;

/**
 * @brief Named command parameter.
 */
typedef struct {
  char name[PLUGIN_MAX_STRING_LEN]; /**< Parameter name */
  PluginParamValue value;           /**< Parameter value */
} PluginParam;

/**
 * @brief Command sent from the host to the plugin.
 *
 * Represents a single operation requested by the system.
 */
typedef struct {
  char id[PLUGIN_MAX_STRING_LEN];              /**< Unique command ID */
  char instrument_name[PLUGIN_MAX_STRING_LEN]; /**< Target instrument */
  char verb[PLUGIN_MAX_STRING_LEN];            /**< Command/operation name */

  PluginParam params[PLUGIN_MAX_PARAMS]; /**< Input parameters */
  uint32_t param_count;                  /**< Number of valid parameters */

  uint32_t timeout_ms;   /**< Requested timeout in milliseconds */
  bool expects_response; /**< Whether a response is expected */
} PluginCommand;

/**
 * @brief Response returned from plugin to host.
 *
 * The plugin must populate this structure in-place.
 *
 * @note Ownership:
 * - The structure is allocated by the caller.
 * - Plugins must NOT free it.
 * - All pointers inside must refer to valid memory during return.
 */
typedef struct {
  char command_id[PLUGIN_MAX_STRING_LEN];      /**< ID of the command */
  char instrument_name[PLUGIN_MAX_STRING_LEN]; /**< Instrument name */

  bool success; /**< Whether command succeeded */

  PluginParamValue return_value; /**< Structured return value */

  /// Optional human-readable text response
  char text_response[PLUGIN_MAX_PAYLOAD];

  /// Optional binary response (inline)
  uint8_t binary_response[PLUGIN_MAX_PAYLOAD];
  uint32_t binary_response_size; /**< Bytes used in binary_response */

  int32_t error_code; /**< Error code (if failure) */

  /// Short error message (null-terminated)
  char error_message[PLUGIN_MAX_STRING_LEN];

  /**
   * @brief Large data support (external buffer)
   *
   * If true, data is stored externally and referenced by ID.
   */
  bool has_large_data;

  char data_buffer_id[PLUGIN_MAX_STRING_LEN]; /**< Buffer identifier */
  uint64_t data_element_count;                /**< Number of elements */

  /**
   * @brief Data type identifier (implementation-defined)
   *
   * Example:
   * - 0 = float32
   * - 1 = float64
   */
  uint8_t data_type;

} PluginResponse;

/**
 * @brief Configuration passed during plugin initialization.
 */
typedef struct {
  char instrument_name[PLUGIN_MAX_STRING_LEN]; /**< Instrument name */

  /// JSON string with connection parameters
  char connection_json[PLUGIN_MAX_PAYLOAD];

  /// JSON string describing API/command definitions
  char api_definition_json[PLUGIN_MAX_PAYLOAD];

} PluginConfig;

/**
 * @brief Plugin metadata returned during discovery.
 */
typedef struct {
  uint32_t api_version; /**< Must match INSTRUMENT_PLUGIN_API_VERSION */

  char name[PLUGIN_MAX_STRING_LEN];          /**< Plugin name */
  char version[PLUGIN_MAX_STRING_LEN];       /**< Plugin version */
  char protocol_type[PLUGIN_MAX_STRING_LEN]; /**< Protocol identifier */
  char description[PLUGIN_MAX_STRING_LEN];   /**< Human-readable description */

} PluginMetadata;

/* ============================================================
 * Plugin interface functions
 * ============================================================ */

/**
 * @brief Retrieve plugin metadata.
 *
 * Called before initialization to:
 * - Verify API compatibility
 * - Identify plugin capabilities
 *
 * @return PluginMetadata structure (by value)
 */
INSTRUMENT_PLUGIN_API PluginMetadata plugin_get_metadata(void);

/**
 * @brief Initialize the plugin.
 *
 * Called once after loading.
 *
 * @param config Configuration data (must not be modified)
 * @return 0 on success, non-zero on failure
 */
INSTRUMENT_PLUGIN_API int32_t plugin_initialize(const PluginConfig *config);

/**
 * @brief Execute a command.
 *
 * Called for each operation requested by the host.
 *
 * @param cmd Input command (read-only)
 * @param resp Output response (must be populated by plugin)
 *
 * @return 0 on success, non-zero on failure
 *
 * @note
 * - Must set resp->success
 * - Must copy command_id and instrument_name into response
 */
INSTRUMENT_PLUGIN_API int32_t plugin_execute_command(const PluginCommand *cmd,
                                                     PluginResponse *resp);

/**
 * @brief Shutdown the plugin.
 *
 * Called before unloading.
 *
 * Use this to release:
 * - Connections
 * - Memory
 * - Hardware resources
 */
INSTRUMENT_PLUGIN_API void plugin_shutdown(void);

#ifdef __cplusplus
}
#endif

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
#define INSTRUMENT_PLUGIN_API
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
#define INSTRUMENT_PLUGIN_API_VERSION 2

/// Maximum length for string fields (including null terminator)
#define PLUGIN_MAX_STRING_LEN 128

/**
 * @brief Identifier for parameter value types (ABI-stable).
 *
 * @note Backed by uint8_t to ensure consistent size across compilers.
 */
typedef uint8_t VariableType;

/**
 * @brief Supported parameter value type constants.
 */
enum {
  PARAM_TYPE_NONE = 0, // No value
  PARAM_TYPE_INT64,    // 64-bit signed integer
  PARAM_TYPE_DOUBLE,   // 64-bit float
  PARAM_TYPE_BOOL,     // Boolean value
  PARAM_TYPE_STRING,   // Null-terminated string
  PARAM_TYPE_BUFFER    // Identifier for externally managed buffer (stored as
                       // str_val)
};

/**
 * @brief Named and typed command parameter.
 *
 * The active field in the union is determined by @ref type.
 *
 * The name field is determined by @ref name.
 *
 * @note Ownership rules:
 * - Variable values are only for the duration of the API call * - Variable
 * values are stored inline and copied by value
 * - Plugins must NOT retain pointers to provided data
 */
typedef struct {
  VariableType type; // Runtime type of the value
  char name[PLUGIN_MAX_STRING_LEN];
  // Compile time types
  union {
    int64_t i64_val;
    double d_val;
    bool b_val;
    char str_val[PLUGIN_MAX_STRING_LEN];
  } value;
} Variable;

/**
 * @brief Command sent from the host to the plugin.
 *
 * Parameters can be accessed using:
 * - @ref param_storage_count
 * - @ref param_storage_get
 *
 * @note
 * - The params field is read-only for plugins
 * - The underlying storage is owned by the host
 *
 * @section usage_example Typical Usage
 *
 * @code
 * uint8_t count = param_storage_count(cmd->params);
 * for (uint8_t i = 0; i < count; i++) {
 *     const Variable *v = param_storage_get(cmd->params, i);
 *     // process v
 * }
 * @endcode
 */
typedef struct ParamStorage ParamStorage;

/**
 * @brief Command sent from the host to the plugin.
 *
 * Represents a single operation requested by the system.
 */
typedef struct {
  char id[PLUGIN_MAX_STRING_LEN]; // Unique ID
  char command[PLUGIN_MAX_STRING_LEN];
  const ParamStorage *params;
  uint32_t timeout_ms; /**< Requested timeout in milliseconds */
} PluginCommand;

/**
 * @brief Response container populated by the plugin and returned to the host.
 *
 * The plugin appends output values to this container using:
 * - @ref plugin_response_push
 *
 * @note Ownership:
 * - The container is allocated and owned by the host
 * - Plugins must NOT allocate or free this structure
 * - All data is copied into the container via API calls
 *
 * @note Lifetime:
 * - The container is valid only for the duration of the API call
 * - Plugins must NOT retain pointers to data inside the container
 *
 * @section usage_example Typical Usage
 *
 * @code
 * Variable out = {0};
 * out.type = PARAM_TYPE_DOUBLE;
 * out.value.d_val = 3.14159;
 *
 * plugin_response_push(resp, &out);
 * @endcode
 */
typedef struct PluginResponse PluginResponse;

/**
 * @brief Configuration passed during plugin initialization.
 */
typedef struct {
  char instrument_name[PLUGIN_MAX_STRING_LEN];
  char address[PLUGIN_MAX_STRING_LEN]; // Contains resource string, serial port,
                                       // USB path, or custom address
  uint32_t baud_rate; // Serial baud_rate if using serial connection
  char custom[PLUGIN_MAX_STRING_LEN]; // Additional instrument specific fields
                                      // outside of address and baud_rate
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
 * @note
 *  You should save this instrument_name as a global variable in the plugin
 * since it won't be sent again
 *
 * @param config Configuration data (must not be modified)
 * @return 0 on success, non-zero on failure
 */
INSTRUMENT_PLUGIN_API uint8_t plugin_initialize(const PluginConfig *config);

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
 * - The plugin must populate the response using plugin_response_push()
 * - The response container is owned by the host and must not be freed
 */
INSTRUMENT_PLUGIN_API uint8_t plugin_execute_command(const PluginCommand *cmd,
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

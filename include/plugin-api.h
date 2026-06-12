#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "instrument-plugin.h"

/**
 * @file plugin-api.h
 * @brief Plugin-facing API for interacting with host-provided data.
 *
 * This API is used by plugin implementations to:
 * - Read input parameters supplied by the host
 * - Populate response values returned back to the host
 *
 * @note
 * - Plugins do NOT allocate or free any storage objects.
 * - All storage is owned and managed by the host.
 * - Returned pointers remain valid only for the duration of the call.
 */

/* ============================================================
 * ParamStorage (read-only to plugins)
 * ============================================================ */

/**
 * @brief Get the number of input parameters.
 *
 * @param ps Pointer to parameter storage (provided by host)
 * @return Number of valid parameters
 *
 * @note
 * - Safe to call with NULL (returns 0)
 */
INSTRUMENT_PLUGIN_API uint8_t param_storage_count(const ParamStorage *ps);

/**
 * @brief Retrieve a parameter by index.
 *
 * @param ps Pointer to parameter storage
 * @param index Index of parameter to retrieve
 * @return Pointer to Variable, or NULL if index is out of range
 *
 * @note
 * - Returned pointer is owned by the host
 * - Must NOT be stored or freed by the plugin
 * - Valid only for the duration of the API call
 */
INSTRUMENT_PLUGIN_API const Variable *param_storage_get(const ParamStorage *ps,
                                                        uint8_t index);

/* ============================================================
 * PluginResponse (write-only from plugins)
 * ============================================================ */

/**
 * @brief Append a value to the plugin response.
 *
 * @param resp Response container provided by host
 * @param var Pointer to value to append
 * @return 0 on success, non-zero on failure
 *
 * @note
 * - Performs a copy of @p var
 * - Expands storage automatically if needed
 * - The plugin must not retain pointers to internal storage
 */
INSTRUMENT_PLUGIN_API uint8_t plugin_response_push(PluginResponse *resp,
                                                   const Variable *var);

#ifdef __cplusplus
}
#endif

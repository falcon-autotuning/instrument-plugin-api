#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "instrument-plugin.h"

/**
 * @file plugin-host.h
 * @brief Host-side API for constructing and managing plugin parameters
 *        and responses.
 *
 * This API is intended for use by the instrument script server (ISS) or host
 * application. It provides functions to:
 * - Allocate and manage parameter storage
 * - Efficiently build commands
 * - Collect and read plugin responses
 *
 * @note
 * - All memory allocation is owned by the host.
 * - Plugins must NOT call these functions.
 * - Storage uses small-buffer optimization (SBO) internally.
 *
 * @section usage_constructors Choosing the right constructor
 *
 * Two creation methods are provided:
 *
 * - @ref param_storage_create / @ref plugin_response_create
 *   Use when the number of elements is small or unknown.
 *   These rely on inline storage and grow automatically.
 *
 * - @ref param_storage_create_with_capacity /
 *   @ref plugin_response_create_with_capacity
 *   Use when the expected number of elements is known ahead of time.
 *   This avoids reallocations and provides optimal performance.
 */

/* ============================================================
 * ParamStorage (host → plugin)
 * ============================================================ */

/**
 * @brief Create a new parameter storage container using inline capacity.
 *
 * This constructor uses the built-in inline buffer (SBO) and is optimal when:
 * - The number of parameters is small (≤ inline capacity), or
 * - The number of parameters is not known in advance.
 *
 * @return Pointer to a new ParamStorage, or NULL on allocation failure.
 *
 * @see param_storage_create_with_capacity
 */
INSTRUMENT_PLUGIN_API ParamStorage *param_storage_create(void);

/**
 * @brief Create a new parameter storage container with a specified capacity.
 *
 * This constructor pre-allocates enough space to hold @p capacity elements.
 *
 * Use this when:
 * - The number of parameters is known ahead of time
 * - You want to avoid dynamic resizing and copies
 *
 * @param capacity Number of elements to allocate space for
 * @return Pointer to a new ParamStorage, or NULL on failure
 *
 * @note
 * - If @p capacity is less than or equal to inline capacity, no heap allocation
 * occurs.
 */
INSTRUMENT_PLUGIN_API ParamStorage *
param_storage_create_with_capacity(uint8_t capacity);

/**
 * @brief Ensure the storage can hold at least @p capacity elements.
 *
 * @param ps Pointer to ParamStorage
 * @param capacity Minimum number of elements to support
 *
 * @note
 * - Does nothing if already large enough
 * - Preserves existing elements
 */
INSTRUMENT_PLUGIN_API void param_storage_reserve(ParamStorage *ps,
                                                 uint8_t capacity);

/**
 * @brief Reset the storage to empty without freeing memory.
 *
 * @param ps Pointer to ParamStorage
 *
 * @note
 * - Retains allocated memory for reuse
 * - Sets count to 0
 */
INSTRUMENT_PLUGIN_API void param_storage_reset(ParamStorage *ps);

/**
 * @brief Free the storage and all associated memory.
 *
 * @param ps Pointer to ParamStorage
 *
 * @note Safe to call with NULL
 */
INSTRUMENT_PLUGIN_API void param_storage_free(ParamStorage *ps);

/**
 * @brief Append a parameter to the storage.
 *
 * @param ps Pointer to ParamStorage
 * @param var Pointer to variable to append
 *
 * @note
 * - Copies the value
 * - Expands storage automatically if needed
 */
INSTRUMENT_PLUGIN_API uint8_t param_storage_push(ParamStorage *ps,
                                                 const Variable *var);

/* ============================================================
 * PluginResponse (plugin → host)
 * ============================================================ */

/**
 * @brief Create a new response container using inline capacity.
 *
 * Optimized for common cases where the response contains a small number
 * of values (typically 1).
 *
 * @return Pointer to PluginResponse or NULL on failure
 *
 * @see plugin_response_create_with_capacity
 */
INSTRUMENT_PLUGIN_API PluginResponse *plugin_response_create(void);

/**
 * @brief Create a new response container with a specified capacity.
 *
 * Pre-allocates storage to hold @p capacity elements.
 *
 * Use this when:
 * - The expected number of response values is known
 * - You want to avoid reallocations
 *
 * @param capacity Number of elements to allocate space for
 * @return Pointer to PluginResponse or NULL on failure
 */
INSTRUMENT_PLUGIN_API PluginResponse *
plugin_response_create_with_capacity(uint8_t capacity);

/**
 * @brief Ensure response container can hold at least @p capacity elements.
 *
 * @param resp Pointer to PluginResponse
 * @param capacity Desired minimum capacity
 *
 * @note
 * - Preserves existing elements
 * - Does nothing if already large enough
 */
INSTRUMENT_PLUGIN_API void plugin_response_reserve(PluginResponse *resp,
                                                   uint8_t capacity);

/**
 * @brief Reset the response to empty state.
 *
 * @param resp Pointer to PluginResponse
 *
 * @note
 * - Does not free memory
 * - Enables reuse
 */
INSTRUMENT_PLUGIN_API void plugin_response_reset(PluginResponse *resp);

/**
 * @brief Free the response container and associated memory.
 *
 * @param resp Pointer to PluginResponse
 *
 * @note Safe to call with NULL
 */
INSTRUMENT_PLUGIN_API void plugin_response_free(PluginResponse *resp);

/**
 * @brief Get the number of response values.
 *
 * @param resp Pointer to PluginResponse
 * @return Number of stored values
 */
INSTRUMENT_PLUGIN_API uint8_t plugin_response_count(const PluginResponse *resp);

/**
 * @brief Retrieve a response value by index.
 *
 * @param resp Pointer to PluginResponse
 * @param index Element index
 * @return Pointer to Variable or NULL if out-of-bounds
 *
 * @note
 * - Returned pointer is owned by the container
 * - Valid until reset/free
 */
INSTRUMENT_PLUGIN_API const Variable *
plugin_response_get(const PluginResponse *resp, uint8_t index);

#ifdef __cplusplus
}
#endif

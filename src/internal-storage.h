#pragma once

#include "instrument-plugin.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define STORAGE_INLINE_CAP(s, type) (sizeof((s)->inline_data) / sizeof(type))

#define DECLARE_INTERNAL_STORAGE(struct_name, type, cap)                       \
  struct struct_name {                                                         \
    type *heap;                                                                \
    uint8_t count;                                                             \
    uint8_t capacity;                                                          \
    type inline_data[(cap)];                                                   \
  };

#define DEFINE_STORAGE_CREATE_WITH_CAP(struct_name, prefix, type)              \
  struct_name *prefix##_create_with_capacity(uint8_t initial_cap) {            \
    struct_name *s = calloc(1, sizeof(struct_name));                           \
    if (!s)                                                                    \
      return NULL;                                                             \
                                                                               \
    uint8_t inline_cap = STORAGE_INLINE_CAP(s, type);                          \
                                                                               \
    if (initial_cap > inline_cap) {                                            \
      s->heap = malloc(sizeof(type) * initial_cap);                            \
      if (!s->heap) {                                                          \
        free(s);                                                               \
        return NULL;                                                           \
      }                                                                        \
      s->capacity = initial_cap;                                               \
    } else {                                                                   \
      s->heap = NULL;                                                          \
      s->capacity = inline_cap;                                                \
    }                                                                          \
                                                                               \
    return s;                                                                  \
  }

#define DEFINE_STORAGE_CREATE(struct_name, prefix, type)                       \
  struct_name *prefix##_create(void) {                                         \
    struct_name *s = calloc(1, sizeof(struct_name));                           \
    if (!s)                                                                    \
      return NULL;                                                             \
    s->capacity = STORAGE_INLINE_CAP(s, type);                                 \
    return s;                                                                  \
  }

#define DEFINE_STORAGE_RESERVE(struct_name, prefix, type)                      \
  void prefix##_reserve(struct_name *s, uint8_t new_cap) {                     \
    if (!s || new_cap <= s->capacity)                                          \
      return;                                                                  \
                                                                               \
    type *new_mem = malloc(sizeof(type) * new_cap);                            \
    if (!new_mem)                                                              \
      return;                                                                  \
                                                                               \
    const type *old_data = s->heap ? s->heap : s->inline_data;                 \
    memcpy(new_mem, old_data, sizeof(type) * s->count);                        \
                                                                               \
    if (s->heap)                                                               \
      free(s->heap);                                                           \
    s->heap = new_mem;                                                         \
    s->capacity = new_cap;                                                     \
  }

#define DEFINE_STORAGE_RESET(struct_name, prefix)                              \
  void prefix##_reset(struct_name *s) {                                        \
    if (!s)                                                                    \
      return;                                                                  \
    s->count = 0;                                                              \
  }

#define DEFINE_STORAGE_FREE(struct_name, prefix)                               \
  void prefix##_free(struct_name *s) {                                         \
    if (!s)                                                                    \
      return;                                                                  \
    if (s->heap)                                                               \
      free(s->heap);                                                           \
    free(s);                                                                   \
  }

#define DEFINE_STORAGE_COUNT(struct_name, prefix)                              \
  uint8_t prefix##_count(const struct_name *s) { return s ? s->count : 0; }

#define DEFINE_STORAGE_GET(struct_name, prefix, type)                          \
  const type *prefix##_get(const struct_name *s, uint8_t i) {                  \
    if (!s || i >= s->count)                                                   \
      return NULL;                                                             \
    const type *data = s->heap ? s->heap : s->inline_data;                     \
    return &data[i];                                                           \
  }

#define DEFINE_STORAGE_PUSH(struct_name, prefix, type)                         \
  uint8_t prefix##_push(struct_name *s, const type *v) {                       \
    if (!s || !v)                                                              \
      return 1;                                                                \
                                                                               \
    if (s->count == s->capacity) {                                             \
      uint8_t inline_cap = STORAGE_INLINE_CAP(s, type);                        \
      uint8_t new_cap = s->capacity ? (uint8_t)(s->capacity * 2) : inline_cap; \
                                                                               \
      type *new_mem = malloc(sizeof(type) * new_cap);                          \
      if (!new_mem)                                                            \
        return 1;                                                              \
                                                                               \
      const type *old_data = s->heap ? s->heap : s->inline_data;               \
      memcpy(new_mem, old_data, sizeof(type) * s->count);                      \
                                                                               \
      if (s->heap)                                                             \
        free(s->heap);                                                         \
      s->heap = new_mem;                                                       \
      s->capacity = new_cap;                                                   \
    }                                                                          \
                                                                               \
    type *data = s->heap ? s->heap : s->inline_data;                           \
    data[s->count++] = *v;                                                     \
    return 0;                                                                  \
  }

// 2 for ParamStorage since when setting there is a Channel + Value
DECLARE_INTERNAL_STORAGE(ParamStorage, Variable, 2)
// 1 for PluginResponse since when getting there is 1
DECLARE_INTERNAL_STORAGE(PluginResponse, Variable, 1)

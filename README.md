# Instrument Plugin API

C99 interface for writing instrument plugins for the Instrument Script Server.

This package provides a **stable ABI contract** that allows external developers to implement instrument drivers as dynamically loaded plugins.

---

## Overview

Plugins are shared libraries (`.so`, `.dll`) that implement this C API.  
The host (Instrument Script Server) loads these libraries at runtime and communicates with them using this interface.

Most plugin logic will live inside:

```c
plugin_execute_command(...)
````

This function should dispatch commands (e.g., via switch/if chains or lookup tables) and perform instrument I/O.

---

## Getting Started

Include headers

```c
#include <instrument-plugin-api/instrument_plugin.h>
#include <instrument-plugin-api/plugin-api.h>
```

> Plugins should **NOT include `plugin-host.h`**

Implement required functions

```c
PluginMetadata plugin_get_metadata(void);

uint8_t plugin_initialize(const PluginConfig *config);

uint8_t plugin_execute_command(const PluginCommand *cmd,
                               PluginResponse *resp);

void plugin_shutdown(void);
```

---

## Minimal Example

```c
#include <instrument-plugin-api/instrument_plugin.h>
#include <instrument-plugin-api/plugin-api.h>

PluginMetadata plugin_get_metadata(void) {
  PluginMetadata meta = {0};
  meta.api_version = INSTRUMENT_PLUGIN_API_VERSION;
  strcpy(meta.name, "Example Plugin");
  strcpy(meta.version, "1.0.0");
  strcpy(meta.protocol_type, "example");
  strcpy(meta.description, "Minimal example plugin");
  return meta;
}

uint8_t plugin_initialize(const PluginConfig *config) {
  (void)config;
  return 0;
}

uint8_t plugin_execute_command(const PluginCommand *cmd,
                               PluginResponse *resp) {

  uint8_t count = param_storage_count(cmd->params);

  for (uint8_t i = 0; i < count; i++) {
    const Variable *v = param_storage_get(cmd->params, i);
    // process parameters
  }

  Variable out = {0};
  out.type = PARAM_TYPE_INT64;
  out.value.i64_val = 42;

  plugin_response_push(resp, &out);

  return 0;
}

void plugin_shutdown(void) {}
```

---

## Data Model

### Command (host → plugin)

```c
PluginCommand
```

Contains:

* command identifier
* command/verb string
* parameter storage (`ParamStorage`)
* timeout

Accessing parameters

```c
uint8_t count = param_storage_count(cmd->params);

for (uint8_t i = 0; i < count; i++) {
    const Variable *v = param_storage_get(cmd->params, i);
}
```

---

### Response (plugin → host)

```c
PluginResponse
```

Plugins populate responses using:

```c
plugin_response_push(resp, &value);
```

Supports:

* multiple return values
* typed values (`Variable`)

---

## Host API (advanced usage)

The host constructs commands using `plugin-host.h`.

Example:

```c
ParamStorage *ps = param_storage_create_with_capacity(2);

Variable v = {0};
v.type = PARAM_TYPE_INT64;
v.value.i64_val = 10;

param_storage_push(ps, &v);
```

> Plugins should never use this API.

---

## Memory & Ownership Rules

* All storage is owned by the host
* Plugins must NOT allocate or free storage containers
* Plugins must NOT retain pointers to returned values

---

## Related Projects

* [Instrument Script Server](https://github.com/falcon-autotuning/instrument-script-server) (host application)
* [Instrument SDK](https://github.com/falcon-autotuning/instrument-sdk) (plugin template repository)

---

## License

See [LICENSE](LICENSE) for details.

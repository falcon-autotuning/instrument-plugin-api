# Instrument Plugin API

C99 interface for writing instrument plugins for the Instrument Script Server.

This package provides a **stable ABI contract** that allows external developers to implement instrument drivers as dynamically loaded plugins.

***

## Overview

Plugins are shared libraries (`.so`, `.dll`) that implement this C API.  
The server loads these libraries at runtime and communicates with them using this interface.
It is expected that the plugin_execute_command is where most plugin time will be spent.
For instruments consisting of lookup like tables for their commands,  this should implement a large if elif else tree to select the proper command to execute depending on the verb.

***

## Getting Started

### 1. Include the API

```c
#include <instrument-plugin-api/instrument_plugin.h>
```

***

### 2. Implement required functions

Every plugin **must implement**:

```c
PluginMetadata plugin_get_metadata(void);

int32_t plugin_initialize(const PluginConfig *config);

int32_t plugin_execute_command(const PluginCommand *cmd,
                               PluginResponse *resp);

void plugin_shutdown(void);
```

***

### 3. Build as a shared library

#### Linux

```bash
gcc -shared -fPIC plugin.c -o my_plugin.so
```

#### Windows

```bat
clang-cl /LD plugin.c /Fe:my_plugin.dll
```

***

## Minimal Example

```c
#include <instrument-plugin-api/instrument_plugin.h>
#include <string.h>

PluginMetadata plugin_get_metadata(void) {
  PluginMetadata meta = {0};
  meta.api_version = INSTRUMENT_PLUGIN_API_VERSION;
  strcpy(meta.name, "Example Plugin");
  strcpy(meta.version, "1.0.0");
  strcpy(meta.protocol_type, "example");
  strcpy(meta.description, "Minimal example plugin");
  return meta;
}

int32_t plugin_initialize(const PluginConfig *config) {
  (void)config;
  return 0;
}

int32_t plugin_execute_command(const PluginCommand *cmd,
                               PluginResponse *resp) {
  strncpy(resp->command_id, cmd->id, PLUGIN_MAX_STRING_LEN - 1);
  strncpy(resp->instrument_name, cmd->instrument_name,
          PLUGIN_MAX_STRING_LEN - 1);

  resp->success = true;
  strncpy(resp->text_response, "OK", PLUGIN_MAX_PAYLOAD - 1);
  return 0;
}

void plugin_shutdown(void) {}
```

***

## Data Model

### Command

The server sends commands via:

```c
PluginCommand
```

Includes:

* command ID
* verb (operation)
* parameters
* timeout
* response expectation

***

### Response

Plugins return results via:

```c
PluginResponse
```

Supports:

* success/failure
* structured return value
* text response
* binary data
* large data via buffer ID

***

## Memory & Ownership Rules (Important)

* All structs are **owned by the caller**
* Do **not store pointers** from `PluginCommand`
* Do **not free anything passed into your plugin**
* Fill `PluginResponse` in-place

### For pointer-based parameter types

* Memory is owned by the caller
* Valid only for the duration of the call
* Must not be freed or retained

***

## Versioning

The API version is defined as:

```c
#define INSTRUMENT_PLUGIN_API_VERSION 1
```

Plugins must:

```c
meta.api_version = INSTRUMENT_PLUGIN_API_VERSION;
```

The server will reject incompatible versions.

***

## ABI Stability Rules

To maintain compatibility:

* ❌ Do NOT reorder struct fields
* ❌ Do NOT remove fields
* ✅ Only add fields at the end
* ✅ Keep types stable

***

## Build Integration (CMake)

```cmake
find_package(instrument-plugin-api CONFIG REQUIRED)

add_library(my_plugin MODULE plugin.c)

target_link_libraries(my_plugin
  PRIVATE instrument-plugin-api::instrument-plugin-api
)
```

***

## What This Repository Is (and Is Not)

### ✅ This repo provides

* The plugin interface header
* Build integration (CMake/vcpkg)

### ❌ This repo does NOT provide

* Plugin loader
* Server runtime
* Data buffer system

Those belong to the **instrument script server**.

***

## Related Projects

* Instrument Script Server (host application)
* Plugin template repository (recommended starting point)

***

## License

MPLv2

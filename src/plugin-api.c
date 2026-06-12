#include "plugin-api.h"
#include "internal-storage.h"

DEFINE_STORAGE_COUNT(ParamStorage, param_storage)
DEFINE_STORAGE_GET(ParamStorage, param_storage, Variable)

DEFINE_STORAGE_PUSH(PluginResponse, plugin_response, Variable)

#include "plugin-host.h"
#include "internal-storage.h"

DEFINE_STORAGE_CREATE_WITH_CAP(ParamStorage, param_storage, Variable)
DEFINE_STORAGE_CREATE(ParamStorage, param_storage, Variable)
DEFINE_STORAGE_RESERVE(ParamStorage, param_storage, Variable)
DEFINE_STORAGE_RESET(ParamStorage, param_storage)
DEFINE_STORAGE_FREE(ParamStorage, param_storage)
DEFINE_STORAGE_PUSH(ParamStorage, param_storage, Variable)

DEFINE_STORAGE_CREATE(PluginResponse, plugin_response, Variable)
DEFINE_STORAGE_CREATE_WITH_CAP(PluginResponse, plugin_response, Variable)
DEFINE_STORAGE_RESERVE(PluginResponse, plugin_response, Variable)
DEFINE_STORAGE_RESET(PluginResponse, plugin_response)
DEFINE_STORAGE_FREE(PluginResponse, plugin_response)
DEFINE_STORAGE_PUSH(PluginResponse, plugin_response, Variable)
DEFINE_STORAGE_COUNT(PluginResponse, plugin_response)
DEFINE_STORAGE_GET(PluginResponse, plugin_response, Variable)

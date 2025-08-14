// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2025-03-15
// [Describe]	Plugin Administration Implementation
// [Copyright]  xiong.qiang
// [Brief]      Plugin loading and management implementation
// *************************************************************************
#include "../_inc/plugin_admin.hpp"
#include "itf/i_plugin.hpp"
#include "../3rd/x/x.hpp"
#include <map>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace nb {

void FreeLib(void* handle){
    #ifdef _WIN32
        auto releaseFunc = (FUNC_PLG_RELEASE)GetProcAddress((HMODULE)handle, "clearNB");
        if (!releaseFunc) 
            releaseFunc();
        FreeLibrary((HMODULE)handle);
    #else
        auto releaseFunc = (FUNC_PLG_RELEASE)dlsym(handle, "clearNB");
        if (!releaseFunc) 
            releaseFunc();
        dlclose(handle);
    #endif
}

PluginAdmin::PluginAdmin():I_PluginAdmin() {
    // Initialize any needed resources
}

PluginAdmin::~PluginAdmin() {
    unloadAll();
}

I_Plugin* PluginAdmin::load(x::cStr& path) {
#ifdef _WIN32
    // Windows implementation
    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) {
        return nullptr;
    }
    
    auto instanceFunc = (FUNC_PLG_INSTANCE)GetProcAddress(handle, "instanceNB");
    if (!instanceFunc) {
        FreeLibrary(handle);
        return nullptr;
    }
#else
    // Unix implementation
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        mError = dlerror();
        return nullptr;
    }
    
    auto instanceFunc = (FUNC_PLG_INSTANCE)dlsym(handle, "instanceNB");
    if (!instanceFunc) {
        dlclose(handle);
        return nullptr;
    }
#endif

    I_Plugin* plugin = (I_Plugin*)instanceFunc();
    if (!plugin && !plugin->valid()) {
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return nullptr;
    }

    // Store handle for later unloading
    mPluginHandles[plugin] = handle;
    return plugin;
}

x::Result PluginAdmin::unload(I_Plugin* p) {
    if (!p) 
        return x::Result(1, "Invalid argument");
    auto it = mPluginHandles.find(p);
    if (it == mPluginHandles.end())
        return x::Result(2, "Plugin not found");
    FreeLib(it->second);
    mPluginHandles.erase(it);
    return x::Result::OK();
}

x::Result PluginAdmin::unload(x::cStr& name) {
    for (auto it = mPluginHandles.begin(); it != mPluginHandles.end(); ++it) {
        if (it->first->name() == name) {
            FreeLib(it->second);
            mPluginHandles.erase(it);
            return x::Result::OK();
        }
    }
    return x::Result(1, _fmt("Plugin = [{}] not found", name));
}

I_Plugin* PluginAdmin::plugin(x::cStr& name) {
    for (auto& pair : mPluginHandles) {
        if (pair.first->name() == name) {
            return pair.first;
        }
    }
    return nullptr;
}

void PluginAdmin::unloadAll() {
    for (auto& pair : mPluginHandles) {
        pair.first->uninit();
        FreeLib(pair.second);
    }
    mPluginHandles.clear();
}

std::string PluginAdmin::pluginInfo(x::cStr& path)
{
#ifdef _WIN32
    // Windows implementation
    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) {
        return "";
    }
    
    auto infoFunc = (FUNC_PLG_INFO)GetProcAddress(handle, "infoNB");
    if (!infoFunc) {
        FreeLibrary(handle);
        return "";
    }
#else
    // Unix implementation
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        return "";
    }
    
    auto infoFunc = (FUNC_PLG_INFO)dlsym(handle, "infoNB");
    if (!infoFunc) {
        dlclose(handle);
        return "";
    }
#endif

    std::string info = infoFunc();

    // close dll
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
    return info;
}

} // namespace nb

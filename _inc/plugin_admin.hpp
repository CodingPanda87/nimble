// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-09-05
// [Describe]	Plugin Administration
// [Copyright]  xiong.qiang
// [Brief]      Plugin loading and management system
// *************************************************************************
#pragma once

#include "itf/i_plugin.hpp"

namespace nb {

typedef void *      (*FUNC_PLG_INSTANCE)();
typedef const char* (*FUNC_PLG_INFO)();  
typedef void        (*FUNC_PLG_RELEASE)();  

class PluginAdmin : public I_PluginAdmin{
public:
    PluginAdmin();
    ~PluginAdmin() override;

    void unloadAll();

    std::string error() const{
        return mError;
    }

    void pump();

    static std::string pluginInfo(x::cStr& path);

    // ---------- I_PluginAdmin ----------
    
    I_Plugin * load(x::cStr& path)   override;
    x::Result  unload(I_Plugin *p)   override;
    x::Result  unload(x::cStr& name) override;
    I_Plugin * plugin(x::cStr& name) override;

private:

    std::map<I_Plugin*, void*> mPluginHandles;  // Stores plugin instances and their handles
    std::string                mError;         
};

} // namespace nb

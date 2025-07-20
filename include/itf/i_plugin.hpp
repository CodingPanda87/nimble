// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-01-03
// [Describe] 	Plugin inteface of nimble framwork 
// [Copyright]	@2024 xiong.qiang All Rights Reserved.
// *************************************************************************
#pragma once

#include "itf.hpp"

namespace nb{

constexpr Version VER_PLUGIN       = {1, 0, 0};
constexpr Version VER_PLUGIN_MIN   = {1, 0, 0};

class I_Ctx;

class I_Plugin : public ITF{
public:
    virtual ~I_Plugin() = default;

    virtual x::Result   init(I_Ctx *i)              = 0;
    virtual void        pump()                      = 0;
    virtual void        uninit()                    = 0;
    virtual x::cStr&    info()                      = 0;
    virtual x::cStr&    name()                      = 0;
    virtual void*       itfPtr(x::cStr& name) 
                                     const noexcept = 0;

    template<typename T>
    T * itf(x::cStr& name) const noexcept{
        return static_cast<T*>(itfPtr(name));
    }

protected:
    I_Plugin() : ITF("I_Plugin", VER_PLUGIN, VER_PLUGIN_MIN){}
};

class I_PluginAdmin:public ITF{
public:
    virtual ~I_PluginAdmin() = default;

    virtual I_Plugin * load(x::cStr& path)    = 0;
    virtual x::Result  unload(I_Plugin *p)    = 0;
    virtual x::Result  unload(x::cStr& name)  = 0;
    virtual I_Plugin * plugin(x::cStr& name)  = 0;

protected:
    I_PluginAdmin() : ITF("I_PluginAdmin", VER_PLUGIN, VER_PLUGIN_MIN){}   
};

} // namespace nb
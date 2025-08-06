// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2025-03-01
// [Describe]	Platform Implementation
// [Copyright]  xiong.qiang
// [Brief]      Core platform initialization and management
// *************************************************************************
#include "platform.hpp"
#include "core/sys.hpp"
#include "plugin_admin.hpp"
#include "nlohmann/json.hpp"
#include "log.hpp"
#include "config.hpp"
#include "event.hpp"
#include "nb.hpp"
#include "mem_pool.hpp"

NB_GLOBAL()

namespace nb {

void*               g_onlyFlag = nullptr; 
PluginAdmin         g_pluginAdmin;
Log                 g_logObj;
std::thread         g_mainThread;

Event               g_evt;
ThreadPoolAdmin*    g_threadPoolAdmin = ThreadPoolAdmin::instance();
MemPool             g_memPool;

void main_worker(Platform *p){
    while(p->isRunning()){
        p->pump();
        x::sleep(3);
    }
}

void config_log(const nlohmann::json &j){
    if(j.contains("filter")){
        auto const& flt = j["filter"];
        for(auto i = 0;i < flt.size();++i){
            auto s = flt.at(i).dump(4);
            if(!g_logObj.addFilter(s))
                std::cerr<<"Log add filter = "<<s<<" , failed !"<<std::endl;
        }
    }
}

Platform*  Platform::instance()
{
    static Platform s;
    return &s;
}

x::Result Platform::init(x::cStr &cfgPath, const bool& isUI)
{
    if(sys::has_only(_fmt("nb:{}",sys::proc_id())))
        return x::Result(1,"Platform already inited !");
    g_log = &g_logObj;
    LOG_INFO("hello"," ^_^ NB ^_^");
    std::ifstream f(PATH_CFG_PLAT);
    auto j = nlohmann::json::parse(f);
    if(j.contains("plugin")){
        auto plugins = j["plugin"];
        std::string s = "\n ------------- load plugin --------------\n";
        for(auto& p : plugins){
            auto path = p.get<std::string>();
            if(path.find("/") == std::string::npos)
                path = sys::proc_dir() + "/" + path;
            auto plg = g_pluginAdmin.load(path);
            if(plg != nullptr){
                s += _fmt("[OK] \t {}, \t{}\n",path,plg->info());
                plg->init(this);
            }
            else
                s += _fmt("[FAIL] \t {} \tError : {}\n",path,g_pluginAdmin.error());

        }
        LOG_INFO("load_plugin",s);
    }
    if(j.contains("log"))
        config_log(j["log"]);
    if(isUI){
        g_mainThread = std::thread(main_worker,this);
        g_mainThread.detach();
    }
    g_memory = Memory::instance();
    running_ = true;
    return x::Result::OK();
}

void Platform::pump() // only for console
{
    g_logObj.pump();
}

void Platform::stop()
{
    if(running_) running_ = false;
    g_pluginAdmin.unloadAll();
    if(g_mainThread.joinable())
        g_mainThread.join();
    LOG_INFO("goodbye"," ^_^ NB ^_^");
    g_logObj.flush();
    g_logObj.enable(false);
}

// ----------------------- I_Ctx -----------------------
I_Evt*      Platform::evt()                   const noexcept
{
    return &g_evt;
}

I_Log*      Platform::log()                   const noexcept
{
    return &g_logObj;
}

I_MemPool*  Platform::memPool()               const noexcept
{
    return &g_memPool;
}

ThreadPoolAdmin* Platform::threadPoolAdmin()  const noexcept
{
    return g_threadPoolAdmin;
}

I_PluginAdmin* Platform::pluginAdmin()  const noexcept
{
    return &g_pluginAdmin;
}

void        Platform::exit(x::cStr &info)     noexcept 
{
    running_ = false;
    LOG_INFO("exit",info);
}

x::Result   Platform::regItf(x::cStr& name, ITF *itf) noexcept 
{
    std::unique_lock<std::shared_mutex> lk(mtx_);
    if(itfs_.find(name) != itfs_.end())
        return x::Result(1,_fmt("Itf = {} already registed !",name));
    itfs_[name] = itf;
    return x::Result::OK();
}

ITF*        Platform::getItf(x::cStr& name)   const noexcept
{
    std::shared_lock<std::shared_mutex> lk(mtx_);
    auto it = itfs_.find(name);
    if(it != itfs_.end())
        return it->second;
    return nullptr;
}

void        Platform::unregItf(x::cStr& name)  noexcept
{
    std::unique_lock<std::shared_mutex> lk(mtx_);
    auto it = itfs_.find(name);
    if(it != itfs_.end())
        itfs_.erase(it);
}

} // namespace nb

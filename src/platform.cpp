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

Event               g_evtObj;
ThreadPoolAdmin*    g_threadPoolAdmin = ThreadPoolAdmin::instance();
MemPool             g_memPool;

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

void say_hello()
{
    std::vector<std::string> nbArt = {
        "*     *  ***** ",
        "**    *  *    *",
        "* *   *  *    *",
        "*  *  *  ***** ",
        "*   * *  *    *",
        "*    **  *    *",
        "*     *  ***** "
    };
    std::string nbArtStr = "\n\t\t\t\t\t\t";
    for(auto& s : nbArt){
        nbArtStr += s;
        nbArtStr += "\n\t\t\t\t\t\t";
    }
    LOG_INFO("hello",_fmt("\n{}",nbArtStr));
}

void  Platform::initInner(const std::string& cfgPath)
{
    const x::str cfgFilePath =  cfgPath == "" ? PATH_CFG_PLAT : cfgPath;
    std::ifstream f(cfgFilePath);
    if(!f.is_open()){
        isInited_ = 1;
        LOG_ERROR("Plantform init",_fmt("open cfg file = {} failed !",cfgFilePath));
        return;
    }
    try{
        auto j = nlohmann::json::parse(f);
        // ---------- load plugins ----------
        LOG_INFO("load_plugin","will load plugin ...");
        if(j.contains("plugin")){
            const auto& plugins = j["plugin"];
            std::string s = "\n ------------- load plugin --------------\n";
            for(auto& p : plugins){
                auto path = p.get<std::string>();
                if(path.find("/") == std::string::npos)
                    path = sys::proc_dir() + "/" + path;
                auto plg = g_pluginAdmin.load(path);
                if(plg != nullptr){
                    s += _fmt("[OK] \t {}, \t{}\n",x::file_name(path),plg->info());
                    try{
                        plg->init(this);
                    }catch(const std::exception& e){
                        s += _fmt("[FAIL] \t {} \t Init Error : {}\n",x::file_name(path),e.what());
                    }
                }
                else
                    s += _fmt("[FAIL] \t {} \tError : {}\n",x::file_name(path),g_pluginAdmin.error());

            }
            LOG_INFO("load_plugin",s);
        }
        // ---------- config log ----------
        if(j.contains("log"))
            config_log(j["log"]);
        isInited_ = 0;
        return;
    }catch(const std::exception& e){
        LOG_ERROR("Plantform init",_fmt("parse cfg file = {} failed ! json error = \n {}",cfgFilePath,e.what()));
    }
    isInited_ = 2;
}

void Platform::main_worker(Platform *p,std::string cfgPath){
    p->initInner(cfgPath);
    if(p->isInited_ > 0)
        return;
    while(p->isRunning()){
        p->pump();
        x::sleep(3);
    }
}

x::Result Platform::init(x::cStr &cfgPath, const bool& isUI)
{
    if(sys::has_only(_fmt("nb:{}",sys::proc_id())))
        return x::Result(1,"Platform already inited !");
    g_log = &g_logObj;
    say_hello();
    running_ = true;
    if(isUI){
        g_mainThread = std::thread(main_worker,this,cfgPath);
        g_mainThread.detach();
    }else{
        initInner(cfgPath);
        if(!isInited_)
            return x::Result(1,"Platform init failed !");
    }
    g_memory = Memory::instance();
    return x::Result::OK();
}

void Platform::pump() // only for console
{
    g_logObj.pump();
    try{
        g_pluginAdmin.pump();
    }
    catch(const std::exception& e){
        log()->error("pump",_s("Plugin Pump Error : ") + e.what(),_code_info());
    }
}

void Platform::stop()
{
    if(running_) running_ = false;
    g_evtObj.exit();
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
    return &g_evtObj;
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

std::string Platform::pluginInfo(x::cStr& path)
{
    return PluginAdmin::pluginInfo(path);
}

} // namespace nb

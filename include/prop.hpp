// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2025-08-17
// [Describe]	property
// [Copyright]  xiong.qiang
// [Brief]      Provide property capabilities and thread-safe
// *************************************************************************
#pragma once

#include <functional>
#include <shared_mutex>

#include "x/x.hpp"

namespace nb {

// <moduleName,key,value>
typedef std::function<void(x::cStr&, x::cStr&, const std::any&)> PropChangeCB;

class Property;

// 属性监听者，用于管理资源同步释放(位于属性管理器的指针)
// 属性监听者释放时，自动从属性管理器中异常
class PropListener{
public:
    virtual ~PropListener() {
        unregPropListener();
    };

    virtual void onPropChanged(x::cStr&, x::cStr&, const std::any&) = 0;

    // 注册属性监听器时自动调用，无需手动调用
    void addProper(Property* prop){
        std::lock_guard<std::shared_mutex> lck(mtx_);
        if(std::find(props_.begin(), props_.end(), prop) == props_.end())
            props_.push_back(prop);
    }

    // 由属性管理器调用，无需手动调用
    void rmProper(Property* prop){
        std::lock_guard<std::shared_mutex> lck(mtx_);
        auto it = std::find(props_.begin(), props_.end(), prop);
        if(it != props_.end())
            props_.erase(it);
    }

    // 析构自动释放，无需手动调用
    void unregPropListener();

protected:
    PropListener() = default;

    mutable std::shared_mutex   mtx_;
    std::vector<Property*>      props_;
};

// 属性管理器，销毁时，自动释放所有属性监听者
class Property {
public:
    struct CallbackLinker{
        PropListener* ptr = nullptr;
        PropChangeCB  cb;

        bool operator == (PropListener* p) const noexcept {
            return ptr == p;
        }

        bool operator != (PropListener* p) const noexcept {
            return ptr != p;
        }
    };

    Property(x::cStr& moduleName):moduleName_(moduleName){}

    virtual ~Property() {
        clearListener();
    }

    const x::cStr& moduleName() const noexcept {
        return moduleName_;
    }

    x::Result addProp(x::cStr& key, const std::any& dft){
        if(values_.find(key) != values_.end())
            return x::Result(1,"key already exists");
        std::lock_guard<std::shared_mutex> lck(mtx_);
        values_[key] = dft;
        return x::Result::OK();
    }

    // 如果监听者是lambda和静态函数，C函数，listener为空，只有监听类，才需要传入listener对象
    x::Result addPropListener(x::cStr& key,PropListener* listener){
        if(listener == nullptr) return x::Result(1,"listener is null");
        listener->addProper(this);
        return addPropListener_(key, {listener,std::bind(&PropListener::onPropChanged,listener,
                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    }

    x::Result addPropListener(x::cStr& key,const PropChangeCB &cb){
        return addPropListener_(key, {nullptr,cb});
    }

    template<typename T>
    std::optional<T> getProp(x::cStr& key) const{
        std::shared_lock<std::shared_mutex> lck(mtx_);
        auto it = values_.find(key);
        if(it == values_.end())
            return std::nullopt;
        return std::make_optional(std::any_cast<T>(it->second));
    }

    x::Result setProp(x::cStr& key, const std::any& value){
        {
            std::lock_guard<std::shared_mutex> lck(mtx_);
            if(values_.find(key) == values_.end())
                return x::Result(1,"no prop = " + key);
            values_[key] = value;
        }
        auto listeners = findListener(key);
        for(auto& cb: listeners){
            if(cb.cb)
                cb.cb(moduleName_, key, value);
        }
        listeners = findListener("*");
        for(auto& cb: listeners){
            if(cb.cb)
                cb.cb(moduleName_, key, value);
        }
        return x::Result::OK();
    }

    void clearListener(){
        std::lock_guard<std::shared_mutex> lck(mtx_);
        listenerCB_.clear();
    }

    void rmPropListener(PropListener * p){
        std::lock_guard<std::shared_mutex> lck(mtx_);
        for(auto it = listenerCB_.begin(); it != listenerCB_.end(); ++it){
            auto& cbs = it->second;
            for(auto cbIt = cbs.begin(); cbIt != cbs.end(); ++cbIt){
                // not allow repeate listener when add, so break
                if(*cbIt == p){
                    cbs.erase(cbIt);
                    break;
                }
            }
        }
    }

    size_t sizeListener() const{
        std::shared_lock<std::shared_mutex> lck(mtx_);
        size_t cnt = 0;
        for(auto& it: listenerCB_){
            cnt += it.second.size();
        }
        return cnt;
    }

protected:
    std::vector<CallbackLinker> findListener(x::cStr& key){
        std::lock_guard<std::shared_mutex> lck(mtx_);
        if(listenerCB_.find(key) == listenerCB_.end())
            return {};
        return listenerCB_[key];
    }

    x::Result addPropListener_(x::cStr& key,const CallbackLinker &cb){
        std::lock_guard<std::shared_mutex> lck(mtx_);
        if(listenerCB_.find(key) == listenerCB_.end()){
            listenerCB_[key] = std::vector<CallbackLinker>{cb};
            return x::Result::OK();
        }
        // 为监听器对象时，才检查是否已存在
        if(cb.ptr != nullptr && std::find(listenerCB_[key].begin(), listenerCB_[key].end(),cb.ptr) != listenerCB_[key].end())
            return x::Result(1,"listener already exists");
        listenerCB_[key].push_back(cb);
        return x::Result::OK();
    }

    x::str                                                  moduleName_;
    std::unordered_map<x::str, std::any>                    values_;
    std::unordered_map<x::str, std::vector<CallbackLinker>> listenerCB_;
    mutable std::shared_mutex                               mtx_;
};


void PropListener::unregPropListener(){
    for(auto prop: props_)
        prop->rmPropListener(this);
    std::lock_guard<std::shared_mutex> lck(mtx_);
    props_.clear();
}

} // namepsace nb
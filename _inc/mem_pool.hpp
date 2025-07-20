// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-08-10
// [Describe]	Memory Pool
// [Copyright]  xiong.qiang
// [Brief]      Memory buffer pooling and management
// *************************************************************************
#include "itf/i_mempool.hpp"
#include <shared_mutex>

namespace nb{

class MemPool : public I_MemPool{
public:
    MemPool():I_MemPool() {}
    ~MemPool() override   {}

    void check() {
        auto now = x::timestamp_ms();
        std::unique_lock<std::shared_mutex> lk(mtx_);
        for(auto it = pool_.begin(); it != pool_.end();){
            if((now - it->second) > freeBufTime_) {
                it = pool_.erase(it);
            }else{
                ++it;
            }
        }
    }

    void setFreeBufTime(const int &ms){
        freeBufTime_ = ms;
    }

    void setMatchFactor(const float &factor){
        matchFactor_ = factor < 0.1f ? factor : 0.1f;
    }

    // --------------- I_MemPool ----------------
    BufferPtr   get(size_t size)             noexcept override{
        if(size == 0) return nullptr;
        std::unique_lock<std::shared_mutex> lk(mtx_);
        for(auto it = pool_.begin(); it != pool_.end(); ++it){
            if(isMatch(size, it->first->size())){
                auto p = it->first;
                pool_.erase(it);
                poolUsed_[p] = x::timestamp_ms();
                return p;
            }
        }
        return newBuf(size);
    }
    
    void        giveback(const BufferPtr& buf) noexcept override{
        if(!buf || buf->size() == 0) return;
        std::unique_lock<std::shared_mutex> lk(mtx_);
        auto it = poolUsed_.find(buf);
        if(it != poolUsed_.end()) {
            poolUsed_.erase(it);
            pool_[buf] = x::timestamp_ms();
        }
    }

    size_t      size()                 const noexcept override{
        size_t sz = 0;
        std::shared_lock<std::shared_mutex> lk(mtx_);
        for(const auto &p : pool_)
            sz += p.first->size();
        for(const auto &p : poolUsed_)
            sz += p.first->size();
        return sz;
    }

protected:
    int                                   freeBufTime_ = 10000;
    mutable std::shared_mutex             mtx_;
    float                                 matchFactor_ = 0.05f;
    // <buffer,time>
    std::unordered_map<BufferPtr, size_t> pool_;
    std::unordered_map<BufferPtr, size_t> poolUsed_;

    BufferPtr newBuf(size_t size){
        auto b = std::shared_ptr<Buffer>(
            new Buffer(static_cast<x::u32>(size),_code_info()));
        poolUsed_[b] = x::timestamp_ms();
        return b;
    }

    bool isMatch(const size_t & sz, const size_t &bufsz){
        return bufsz >= sz && bufsz <= sz * (1 + matchFactor_);
    }
};

} // namespace nb

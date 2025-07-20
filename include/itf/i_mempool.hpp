// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-10-12
// [Describe]	Memory Pool Interface
// [Copyright]  xiong.qiang
// [Brief]      Memory buffer management interface
// *************************************************************************
#pragma once

#include <cstring>
#include "x/x.hpp"
#include "itf.hpp"
#include "core/memory.hpp"

namespace nb{

constexpr Version VER_MEM       = {1, 0, 0};
constexpr Version VER_MEM_MIN   = {1, 0, 0};

struct Buffer{
    Buffer() = default;

    Buffer(x::cU32& size,x::cStr& info) : size_(size) {
        data_ = New<char>(size,info);
    }

    Buffer(const Buffer&) = delete;
    Buffer(Buffer&& other)  : 
    data_(std::move(other.data_)), size_(other.size_) {}
    
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&& other) {
        data_ = std::move(other.data_);
        size_ = other.size_;
        return *this;
    }

    operator bool() const noexcept { return data_.get() != nullptr; }
    char * get()    const noexcept { return data_.get(); }
    size_t size()   const noexcept { return size_;      }

    bool   input(const char * src, size_t size) noexcept {
        if (size > this->size() || !src || !data_)
            return false;
        memcpy(data_.get(), src, size);
        return true;
    }

protected:
    std::unique_ptr<char,Delete<char>> data_;
    x::u32 size_ = 0;
};

typedef std::shared_ptr<Buffer> BufferPtr;

class I_MemPool : public ITF{
public:
    virtual ~I_MemPool() = default;

    virtual BufferPtr   get(size_t size)             noexcept = 0;
    virtual void        giveback(const BufferPtr& b) noexcept = 0;
    virtual size_t      size()                 const noexcept = 0;

protected:
    I_MemPool(): ITF("I_MemPool",VER_MEM, VER_MEM_MIN) {}
};


}// namespace nb

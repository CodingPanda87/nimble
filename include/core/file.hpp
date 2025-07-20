#pragma once

#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace nb{

class File{
public:
    File() {}
    ~File() {
        if(isOpen())
            close();
    }

    bool open(const std::string& path,bool isTruncate = false){
        std::unique_lock lock(mutex_);
        if (file_.is_open()) {
            flush();
            file_.close();
        }
        auto mode = std::ios::in | std::ios::out | std::ios::binary;
        if (isTruncate) mode |= std::ios::trunc;
        file_.open(path, mode);
        return file_.is_open();
    }

    void writeCacheMode(const size_t& size = 1024*1024){
        cacheSize_ = size;
        cache_.reserve(size);
    }

    void        close(bool isFlush = true){
        std::unique_lock lock(mutex_);
        if (file_.is_open()) {
            if(isFlush) flush();
            file_.close();
        }
    }

    void        seek(const size_t& pos){
        std::shared_lock lock(mutex_);
        if (!isOpen()) throw std::runtime_error("File not open");
        file_.seekg(pos);
    }

    void        seekEnd(){
        std::shared_lock lock(mutex_);
        if (!isOpen()) throw std::runtime_error("File not open");
        file_.seekg(0, std::ios::end);
    }

    bool        isOpen() const{
        return file_.is_open();
    }

    std::string readAll(){
        std::shared_lock lock(mutex_);
        if (!isOpen()) throw std::runtime_error("File not open");
        
        file_.seekg(0, std::ios::end);
        size_t size = file_.tellg();
        file_.seekg(0, std::ios::beg);
        
        std::string content(size, '\0');
        file_.read(&content[0], size);
        return content;
    }

    size_t      read(char *buf,const size_t& len){
        std::shared_lock lock(mutex_);
        if (!isOpen()) throw std::runtime_error("File not open");
        file_.read(buf, len);
        return file_.gcount();
    }

    bool        write(const std::string& data){
        return write(data.data(), data.size());
    }

    bool        write(const char *buf,const size_t& len){
        std::unique_lock lock(mutex_);
        if (!isOpen()) return false;
        if (cacheSize_ > 0) {
            if (cache_.size() + len > cacheSize_)
                flush();
            cache_.insert(cache_.end(), buf, buf + len);
        } else 
            file_.write(buf, len);
        return true;
    }

    void        flush(){
        if (!cache_.empty()) {
            file_.write(cache_.data(), cache_.size());
            cache_.clear();
            file_.flush();
        }
    }

protected:
    std::shared_mutex mutex_;
    std::vector<char> cache_;
    size_t            cacheSize_;
    std::fstream      file_;
};

}
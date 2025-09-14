#pragma once

#include "thread_pool.hpp"
#include "x/x.hpp"
#include <future>
#include <atomic>

namespace nb{

struct Task{

    Task(const std::string& key):
    id_(x::timestamp()),key_(key){ }

    Task(const Task& other) = delete;
    Task& operator=(const Task& other) = delete;

    const size_t&       id()             const noexcept {
        return id_;
    }

    const std::string&  key()            const noexcept {
        return key_;
    }

    bool                isValid()        const noexcept {
        return id_ != 0;
    }

    bool operator == (const Task& other) const noexcept {
        return id_ == other.id_;
    }

    bool operator != (const Task& other) const noexcept {
        return id_ != other.id_;
    }

    void start(){
        start_tm = x::timestamp_us();
    }

    void done(){
        end_tm  = x::timestamp_us();
        isdone_ = true;
    }

    size_t take_time(){
        if(!isdone_) return 0;
        return end_tm - start_tm;
    }

    bool is_done() const noexcept {
        return isdone_.load();
    }

protected:
    size_t              id_      = 0;
    std::string         key_;
    std::atomic_bool    isdone_  = false;
    size_t              start_tm = 0;  // us
    size_t              end_tm   = 0;  // us
};

class Concurrent{
public:
    Concurrent() {}

    Concurrent(const Concurrent& other)             = delete;
    Concurrent &operator=(const Concurrent& other)  = delete;

    void add_task(const std::string& key,std::function<x::Struct> func){
        auto task = std::make_shared<Task>(key);
        tasks_.push_back(task);
        pool_->enqueue([=](){
            task->start();
            func();
            task->done();
        });
    }

    void add_task(std::shared_ptr<Task> task){
        tasks_.push_back(task);
    }

    // -1 means wait forever
    bool wait_all(const int& timeout = -1){
        int t = timeout;
        while(timeout < 0 || (t-- > 0)){
            if(sleep(3)) // force exit
                return false;
            if(all_done())
                return true;
        }
        return false;
    }

    void stop(){
        working_ = false;
    }

    const std::string print_info() const {
        std::string s = _fmt("********** concurrent [{}] **********",name_);
        for(auto& task: tasks_){
            s += _fmt("task[{}] {}ms\n",task->key(),task->take_time());
        }
        s += "************************************";
        return s;
    }

protected:
    bool sleep(int ms10){
        while(working_ && (ms10-- > 0)){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return !working_;
    }

    bool all_done() const{
        for(auto& task: tasks_){
            if(!task->is_done()){
                return false;
            }
        }
        return true;
    }

private:
    std::string                        name_;
    std::vector<std::shared_ptr<Task>> tasks_; 
    bool                               working_ = true;
};

}// naemspace nb

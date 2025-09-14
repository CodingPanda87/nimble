#pragma once

#include "thread_pool.hpp"
#include "x/x.hpp"
#include <future>
#include <atomic>
#include <condition_variable>
#include <mutex>

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

    void done(const int& code = 1, const std::string& error = ""){
        end_tm  = x::timestamp_us();
        isdone_ = code;
        error_  = error;
    }

    size_t take_time(){
        if(!isdone_) return 0;
        return end_tm - start_tm;
    }

    bool is_done() const noexcept {
        return isdone_ > 0 || isdone_ < 0;
    }

    bool isError() const noexcept {
        return isdone_ < 0;
    }

    const std::string&  error()        const noexcept {
        return error_;
    }

protected:
    size_t              id_      = 0;
    std::string         key_;
    std::string         error_;
    std::atomic_int     isdone_  = 0;
    size_t              start_tm = 0;  // us
    size_t              end_tm   = 0;  // us
};

class Concurrent{
public:
    Concurrent(const std::string& name = "") : name_(name) {}

    Concurrent(const Concurrent& other)             = delete;
    Concurrent &operator=(const Concurrent& other)  = delete;

    template<typename F, typename... Args>
    bool add_task(const std::string& key, F&& func, ThreadPool* pl, Args&&... args){
        if(!pl) return false;
        
        auto task = std::make_shared<Task>(key);
        {
            std::lock_guard<std::mutex> lock(tasks_mutex_);
            tasks_.push_back(task);
        }
        
        pl->commitNoRet([this, task, func, args...]() mutable {
            try {
                task->start();
                func(std::forward<Args>(args)...);
                task->done();
            } catch (const std::exception& e) {
                task->done(-1, e.what());
            } catch (...) {
                task->done(-1, "Unknown error occurred");
            }
            
            // Notify waiting threads that a task completed
            std::lock_guard<std::mutex> lock(tasks_mutex_);
            completed_tasks_++;
            task_cv_.notify_all();
        });
        return true;
    }

    // -1 means wait forever
    bool wait_all(const int& timeout_ms = -1){
        std::unique_lock<std::mutex> lock(tasks_mutex_);
        
        if (timeout_ms < 0) {
            // Wait forever
            task_cv_.wait(lock, [this] { 
                return !working_ || all_done_internal(); 
            });
        } else {
            // Wait with timeout
            auto status = task_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
                [this] { return !working_ || all_done_internal(); });
            if (!status) {
                return false; // Timeout
            }
        }
        
        return !working_ ? false : all_done_internal();
    }

    bool isError() const{
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        for(auto& task: tasks_){
            if(task->isError()){
                return true;
            }
        }
        return false;
    }

    void stop(){
        {
            std::lock_guard<std::mutex> lock(tasks_mutex_);
            working_ = false;
        }
        task_cv_.notify_all();
    }

    const std::string print_info() const {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        std::string s = _fmt("********** concurrent [{}] **********\n", name_);
        for(auto& task: tasks_){
            s += _fmt("task[{}] {}ms {}\n", 
                     task->key(), 
                     task->take_time() / 1000, // convert us to ms
                     task->isError() ? _fmt("[ERROR: {}]", task->error()) : "");
        }
        s += "************************************\n";
        return s;
    }

    size_t task_count() const {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        return tasks_.size();
    }

    size_t completed_count() const {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        return completed_tasks_;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        tasks_.clear();
        completed_tasks_ = 0;
        working_ = true;
    }

protected:
    bool all_done_internal() const {
        for(auto& task: tasks_){
            if(!task->is_done()){
                return false;
            }
        }
        return true;
    }

private:
    std::string                        name_;
    mutable std::mutex                 tasks_mutex_;
    std::vector<std::shared_ptr<Task>> tasks_; 
    std::atomic<size_t>                completed_tasks_{0};
    std::condition_variable            task_cv_;
    std::atomic<bool>                  working_{true};
};

}// naemspace nb

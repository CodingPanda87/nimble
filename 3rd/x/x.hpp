//*******************************************************
//                      ___   ___ 
//                      \  \ /  / 
//                       \  V  /  
//                        >   <   
//                       /  .  \  
//                      /__/ \__\ 
//
//*******************************************************                              

/**
 * @file x.hpp
 * @author xiong qiang 
 * @brief x-lib , write cpp like python
 * @version 0.9
 * @date 2025-06-09
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#if __cplusplus < 202002L 
#error "x-lib requires C++20 or later"
#endif

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <format>
#include <ranges>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <random>
#include <ctime>
#include <numeric>
#include <iomanip>
#include <any>
#include <functional>
#include <thread>
#include <map>
#include <unordered_map>
#include <condition_variable>
#include <mutex>
#if __cplusplus >= 202302L
#include <print>
#endif

// ----------------------- macro define -----------------------
#define _coto       const auto
#define _vec        std::vector
#define _nul        nullptr
#define _s(a)       std::string(a)   
#define _fmt        std::format
#define _unmap      std::unordered_map

#define _excode_info(i)     _fmt("{}/{}/{}", \
                            std::filesystem::path(__FILE__).filename().string(), \
                            __LINE__, (i  == std::string("") ? __func__ : i))
#define _code_info()        _excode_info("")
#define _build_time()       (__DATE__ " " __TIME__)

// concate define string or "str"
#define CONCAT_IMPL(s1, s2) s1 s2
#define _cat(s1, s2) CONCAT_IMPL(s1, s2)


// adapte lower version of MSC
#if defined(_MSC_VER) && _MSC_VER < 1936
namespace std {
    template <typename... Args>
    struct format_string {
        std::string_view str;
        constexpr format_string(const char* s) : str(s) {}
        constexpr format_string(std::string_view s) : str(s) {}
    };
}
#endif

#if __cplusplus < 202302L
namespace std {
template<typename... Args>
inline void print(std::format_string<Args...> formt, Args&&... args) {
    std::cout << std::format(formt, std::forward<Args>(args)...);
}
template<typename... Args>
inline void println(std::format_string<Args...> formt, Args&&... args) {
    std::cout << std::format(formt, std::forward<Args>(args)...) << std::endl;
}
} // namespace std
#endif

#define _prt     std::print
#define _prtln   std::println

namespace x {

// ----------------------- basic types -----------------------
typedef char                 i8;
typedef short               i16;
typedef int                 i32;
typedef long long           i64;

typedef unsigned char        u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef float               f32;
typedef double              f64;

typedef std::string         str;
typedef std::string_view    sView;

// ----------------------- const types -----------------------
typedef const i8            cI8;
typedef const i16          cI16;
typedef const i32          cI32;
typedef const i64          cI64;
typedef const u8            cU8;
typedef const u16          cU16;
typedef const u32          cU32;
typedef const u64          cU64;
 
typedef const f32          cF32;
typedef const f64          cF64;
 
typedef const str          cStr;
typedef const sView      cSView;

// ----------------------- mini func -----------------------
inline std::string vfmt(cStr& ft, auto&&... args){
    return std::vformat(ft, std::make_format_args(args...));
}

inline void vprt(cStr& ft, auto&&... args) {
    std::cout << std::vformat(ft, std::make_format_args(args...));
}

inline void vprtln(cStr& ft, auto&&... args) {
    std::cout << std::vformat(ft, std::make_format_args(args...)) << std::endl;
}

inline void veprt(cStr& ft, auto&&... args) {
    std::cerr << std::vformat(ft, std::make_format_args(args...));
}

inline void veprtln(cStr& ft, auto&&... args) {
    std::cerr << std::vformat(ft, std::make_format_args(args...)) << std::endl;
}

// ----------------------- string operations -----------------------
inline _vec<str> split(cStr &s, cSView& delimiter) noexcept {
    if(s.empty() || delimiter.empty()) return {};
    _vec<str> result;
    size_t start = 0;
    size_t end = s.find(delimiter);
    while (end != str::npos) {
        result.emplace_back(s.substr(start, end - start));
        start = end + delimiter.length();
        end = s.find(delimiter, start);
    }
    if (start == 0) return {};
    result.emplace_back(s.substr(start));
    return result;
}

template<typename Range>
inline str join(const Range& parts, cSView& delimiter) noexcept {
    if (parts.empty()) return "";
    str result;
    auto it = parts.begin();
    result += *it;
    ++it;
    for (; it != parts.end(); ++it) {
        result += delimiter;
        result += *it;
    }
    return result;
}

inline str trim(cStr &s) noexcept {
    auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
    auto end = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
    return {start, end};
}

inline str replace(str s, cSView& old_sub, cSView& new_sub) noexcept {
    size_t pos = 0;
    while ((pos = s.find(old_sub, pos)) != str::npos) {
        s.replace(pos, old_sub.length(), new_sub);
        pos += new_sub.length();
    }
    return s;
}

inline str reverse(str s) noexcept {
    std::reverse(s.begin(), s.end());
    return s;
}

inline str upper(str s) noexcept {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

inline str lower(str s) noexcept {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

inline bool startWith(cStr& s, cSView& prefix) noexcept {
    return s.substr(0, prefix.size()) == prefix;
}

inline bool endWith(cStr& s, cSView& suffix) noexcept {
    return s.size() >= suffix.size() && 
    s.substr(s.size() - suffix.size()) == suffix;
}

inline bool contain(cSView& sv, cSView& sub) noexcept {
    return sv.find(sub) != sView::npos;
}

inline bool is_digit(cStr& ss) noexcept {
    if (ss.empty()) return false;
    auto s = ss;
    bool is_negative = false;
    if (s[0] == '-') {
        is_negative = true;
        s.erase(0, 1); // remove negative sign
    }
    bool has_decimal_point = false;
    try {
        for (char c : s) {
            if (c == '.') {
                if (has_decimal_point) return false;
                has_decimal_point = true;
            } else if (c <= -1 || !std::isdigit(c)) {
                return false; 
            }
        }
    } catch (...) {
        return false;
    }
    return true;
}

inline bool same_nocase(cStr& s1, cStr& s2) noexcept {
    if (s1.size() != s2.size()) return false;
    return std::equal(s1.begin(), s1.end(), s2.begin(), 
                      [](char a, char b) { return ::tolower(a) == ::tolower(b); });
}

// ----------------------- filesystem operations -----------------------
inline str file_name(cStr &path){
    return std::filesystem::path(path).filename().string();
}

inline str file_extension(cStr& filename) noexcept{
    size_t last_dot = filename.find_last_of('.');
    if (last_dot == str::npos) {
        return "";
    }
    return filename.substr(last_dot + 1);
}

inline bool exists(cStr& path) noexcept {
    return std::filesystem::exists(path);
}

inline bool is_file(cStr& path) noexcept {
    return std::filesystem::is_regular_file(path);
}

inline bool is_dir(cStr& path) noexcept {
    return std::filesystem::is_directory(path);
}

inline str read_file(cStr& path) {
    std::ifstream file(path);
    if (!file.is_open()) 
        return "";
    return str((std::istreambuf_iterator<char>(file)), 
                std::istreambuf_iterator<char>());
}

inline void write_file(cStr& path, cStr& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error(_fmt("Failed to open file: {}", path));
    }
    file << content;
}

inline str join_path(const _vec<str>& parts) {
    std::filesystem::path result;
    for (const auto& part : parts) {
        result /= part;
    }
    return result.string();
}

inline _vec<str> split_path(cStr& path) {
    _vec<str> result;
    for (const auto& part : std::filesystem::path(path)) {
        result.push_back(part.string());
    }
    return result;
}

inline str norm_path(cStr& path) {
    return std::filesystem::path(path).lexically_normal().string();
}

inline u64 file_size(cStr& path) noexcept {
    if (!std::filesystem::exists(path)) return 0;
    return std::filesystem::file_size(path);
}

inline _vec<str> list_dir(cStr& path) {
    if (!std::filesystem::exists(path)) return {};
    _vec<str> result;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        result.push_back(entry.path().string());
    }
    return result;
}

inline void create_dir(cStr& path) {
    std::filesystem::create_directory(path);
}

inline void remove_file(cStr& path) {
    std::filesystem::remove(path);
}

inline void remove_dir(cStr& path) {
    std::filesystem::remove_all(path);
}

// ----------------------- utility functions -----------------------

inline u64 id_thread() noexcept{
    std::hash<std::thread::id> hasher;
    return hasher(std::this_thread::get_id());
}

inline void sleep(cU64& milliseconds) noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

inline str current_time(cStr& format = "%Y-%m-%d %H:%M:%S") noexcept {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(&tm, format.c_str());
    return ss.str();
}

inline u64 timestamp() noexcept {
    return std::chrono::steady_clock::now().
            time_since_epoch().count();
}

inline u64 timestamp_ms() noexcept {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

inline u64 timestamp_us() noexcept {
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

inline i32 rand_int(i32 min, i32 max) noexcept {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<i32> dist(min, max);
    return dist(gen);
}

inline f64 rand_float(f64 min = 0.0, f64 max = 1.0) noexcept {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<f64> dist(min, max);
    return dist(gen);
}

template<typename T>
inline T clamp(T value, T min, T max) noexcept {
    return (value < min) ? min : (value > max) ? max : value;
}

// ----------------------- Timer -----------------------

class Timer {
public:
    Timer() = default;
    ~Timer() { stop(); }

    void start(cU64& interval_ms,
               const std::function<void()>& task, 
               const bool& repeat = false) {
        stop();
        running_ = true;
        thread_ = std::thread([this, interval_ms, task, repeat]() {
            while (running_) {
                std::unique_lock<std::mutex> lock(mutex_);
                if (cv_.wait_for(lock, std::chrono::milliseconds(interval_ms),
                                [this] { return !running_; })) {
                    break;
                }
                if (running_) 
                    task();
                if (!repeat) 
                    break;
            }
            running_ = false;
        });
    }

    void stop() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            running_ = false;
            cv_.notify_all();
        }
        if (thread_.joinable()) 
            thread_.join();
    }

    bool is_running() const noexcept { return running_;}

private:
    std::thread             thread_;
    std::mutex              mutex_;
    std::condition_variable cv_;
    bool                    running_ = false;
};

// ----------------------- Result -----------------------

class Result {
public:
    Result()  = default;
    Result(cU32& code, cStr& msg = "") : code_(code), msg_(msg) {}

    bool    ok()        const noexcept { return code_ == 0;  }
    cU32&   code()      const noexcept { return code_;       }
    cStr&   message()   const noexcept { return msg_;        }

    operator bool()     const noexcept { return ok();        }

    void setResult(const std::any& d1, 
                   const std::any& d2 = _nul, 
                   const std::any& d3 = _nul) noexcept {
        dat1_ = d1;
        dat2_ = d2;
        dat3_ = d3;
        code_ = 0;
    }

    void setError(cU32 code, cStr& msg = "") noexcept {
        code_ = code;
        msg_  = msg; 
    }   

    template<typename T>
    T dat1() const {
        return std::any_cast<T>(dat1_);
    }

    template<typename T>
    T dat2() const {
        return std::any_cast<T>(dat2_);
    }

    template<typename T>
    T dat3() const {
        return std::any_cast<T>(dat3_);
    }

    static Result SetData(const std::any& d1, 
                          const std::any& d2 = _nul, 
                          const std::any& d3 = _nul) {
        Result res;
        res.setResult(d1, d2, d3);
        return res;
    }

    static Result OK() { return Result(0, "");}

private:
    u32      code_ = 0xFFFFFFFF;
    str       msg_;
    std::any dat1_;
    std::any dat2_;
    std::any dat3_;
};

// ----------------------- Struct -----------------------
class Struct {
public:
    Struct()  = default;
    ~Struct() = default;

    Struct(const Struct& other) 
        : data_(other.data_) {}

    Struct(const std::initializer_list<std::pair<str,std::any>>&init) {
        for (const auto& [key, value] : init) 
            data_[key] = value;
    }
        
    Struct& operator=(const Struct& other) {
        if (this != &other) {
            data_ = other.data_;
        }
        return *this;
    }
    
    Struct(Struct&& other) noexcept 
        : data_(std::move(other.data_)){}
          
    Struct& operator=(Struct&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
        }
        return *this;
    }

    static Struct One(const std::any& value){
        Struct s;
        s.add(".", value);
        return s;
    } 
    
    template<typename T>
    void add(cStr& key, const T& value) {
        data_[key] = value;
    }

    template<typename T>
    T get(cStr& key) const {
        try {
            return std::any_cast<T>(data_.at(key));
        } catch (const std::bad_any_cast&) {
            throw std::runtime_error(_fmt("Type mismatch for key: {}", key));
        }
    }

    template<typename T>
    T getOnly1() const {
        try {
            return std::any_cast<T>(data_.at("."));
        } catch (const std::bad_any_cast&) {
            throw std::runtime_error(_fmt("Type mismatch for key: {}", "."));
        }
    }

    bool has(cStr& key) const noexcept {
        return data_.find(key) != data_.end();
    }

    void remove(cStr& key) noexcept {
        data_.erase(key);
    }

    bool empty() const noexcept {
        return data_.empty();
    }

    void clear() noexcept {
        data_.clear();
    }

    size_t size() const noexcept {
        return data_.size();
    }

private:
    std::unordered_map<str, std::any>   data_;
};


// ----------------------- Time -----------------------
class Time {
public:
    Time() = default;
    
    Time(cI32& year,     cI32& month,      cI32& day, 
         cI32& hour = 0, cI32& minute = 0, cI32& second = 0, 
         cI32& microseconds = 0) {
        auto dp = std::chrono::year_month_day{
            std::chrono::year{year},
            std::chrono::month{static_cast<unsigned>(month)},
            std::chrono::day{static_cast<unsigned>(day)}
        };
        auto time = std::chrono::hh_mm_ss{
            std::chrono::hours{hour} + 
            std::chrono::minutes{minute} + 
            std::chrono::seconds{second} +
            std::chrono::microseconds{microseconds}
        };
        tp_ = std::chrono::sys_days{dp} + time.to_duration();
    }

    // Create from string "YYYY-MM-DD HH:MM:SS.fffffff"
    explicit Time(cStr& time_str) {
        std::istringstream ss(time_str);
        std::chrono::sys_time<std::chrono::microseconds> tp;
        ss >> std::chrono::parse("%Y-%m-%d %H:%M:%S", tp);
        if (ss.fail()) 
            throw std::runtime_error("Invalid time format");
        if (ss.peek() == '.') {
            ss.ignore();
            u64 us;
            ss >> us;
            if (!ss.fail()) 
                tp += std::chrono::microseconds(us);
        }
        tp_ = tp;
    }

    static Time now() noexcept {
        return Time(std::chrono::system_clock::now());
    }

    // Serialize to string "YYYY-MM-DD HH:MM:SS.ffffff"
    str to_string() const noexcept {
        auto local_time = std::chrono::zoned_time{ std::chrono::current_zone(), tp_};
        auto local_tp = local_time.get_local_time();
        auto dp = floor<std::chrono::days>(local_tp);
        std::chrono::year_month_day ymd{dp};
        std::chrono::hh_mm_ss hms{local_tp - dp};
        return _fmt("{} {}",ymd, hms);
    }

    bool operator==(const Time& other) const noexcept { return tp_ == other.tp_; }
    bool operator!=(const Time& other) const noexcept { return tp_ != other.tp_; }
    bool operator<(const Time& other)  const noexcept { return tp_ < other.tp_; }
    bool operator<=(const Time& other) const noexcept { return tp_ <= other.tp_; }
    bool operator>(const Time& other)  const noexcept { return tp_ > other.tp_; }
    bool operator>=(const Time& other) const noexcept { return tp_ >= other.tp_; }

    Time operator+(const std::chrono::microseconds& duration) const noexcept {
        return Time(tp_ + duration);
    }
    
    Time operator-(const std::chrono::microseconds& duration) const noexcept {
        return Time(tp_ - duration);
    }
    
    std::chrono::microseconds operator-(const Time& other) const noexcept {
        return std::chrono::duration_cast<std::chrono::microseconds>(tp_ - other.tp_);
    }

    i32 year() const noexcept {
        auto ymd = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(tp_)};
        return static_cast<int>(ymd.year());
    }
    
    i32 month() const noexcept {
        auto ymd = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(tp_)};
        return static_cast<unsigned>(ymd.month());
    }
    
    i32 day() const noexcept {
        auto ymd = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(tp_)};
        return static_cast<unsigned>(ymd.day());
    }
    
    i32 hour() const noexcept {
        auto time = std::chrono::hh_mm_ss{tp_ - std::chrono::floor<std::chrono::days>(tp_)};
        return time.hours().count();
    }
    
    i32 minute() const noexcept {
        auto time = std::chrono::hh_mm_ss{tp_ - std::chrono::floor<std::chrono::days>(tp_)};
        return time.minutes().count();
    }
    
    u64 second() const noexcept {
        auto time = std::chrono::hh_mm_ss{tp_ - std::chrono::floor<std::chrono::days>(tp_)};
        return time.seconds().count();
    }
    
    u64 microseconds() const noexcept {
        auto time = std::chrono::hh_mm_ss{tp_ - std::chrono::floor<std::chrono::days>(tp_)};
        return time.subseconds().count()/10;
    }

private:
    explicit Time(std::chrono::system_clock::time_point tp) : tp_(tp) {}
    
    std::chrono::system_clock::time_point tp_;
};

} // namespace x

// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2025-11-01
// [Describe]	json_helper
// [Copyright]  xiong.qiang
// [Brief]      Extended for nlohmann json
// *************************************************************************
#pragma once

#include "json.hpp"

namespace nlohmann {

template<typename T>
inline T json_value_safe(const json& j, const std::string& key, T default_val) {
    static_assert(!std::is_same<T, const char*>::value, 
        "please use std::string instead of const char*,json_value_safe() in json_helper.hpp");
    try { 
        return j.at(key).get<T>(); 
    } catch (...) {}
    return default_val;
}

} // namespace nlohmann

#define NLOHMANN_DEFINE_TYPE_INTRUSIVE_SAFE(Type, ...)                       \
    friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO_SAFE, __VA_ARGS__)) \
    }                                                                        \
    friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_SAFE, __VA_ARGS__)) \
    }

#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_SAFE(Type, ...)                  \
    inline void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO_SAFE, __VA_ARGS__)) \
    }                                                                        \
    inline void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_SAFE, __VA_ARGS__)) \
    }

#define NLOHMANN_JSON_TO_SAFE(field) nlohmann_json_j[#field] = nlohmann_json_t.field;
#define NLOHMANN_JSON_FROM_SAFE(field) nlohmann_json_t.field = nlohmann::json_value_safe(nlohmann_json_j, #field, nlohmann_json_t.field);

// ---------- JSON_COMPATIBLE_MODE ----------
#ifndef JSON_FORCE_MATCH
#define JSON_DEFINE_TYPE_INTRUSIVE NLOHMANN_DEFINE_TYPE_INTRUSIVE_SAFE
#define JSON_DEFINE_TYPE_NON_INTRUSIVE NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_SAFE
#else
#define JSON_DEFINE_TYPE_INTRUSIVE NLOHMANN_DEFINE_TYPE_INTRUSIVE
#define JSON_DEFINE_TYPE_NON_INTRUSIVE NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE
#endif
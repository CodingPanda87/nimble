#pragma once

#include "x/x.hpp"

namespace nb{

struct Version{
protected:
    int major = 0;
    int minor = 0;
    int patch = 0;

public:

    constexpr Version() = default;

    constexpr Version(const int &major, const int & minor, const int & patch) :
            major(major), minor(minor), patch(patch) {}

    constexpr bool valid() const {
        return !(major == 0 && minor == 0 && patch == 0);
    }

    constexpr bool operator==(const Version& other) const {
        return major == other.major && minor == other.minor && patch == other.patch;
    }

    constexpr bool operator!=(const Version& other) const {
        return !(*this == other);
    }

    constexpr bool operator<(const Version& other) const {
        return  major < other.major || 
                (major == other.major && minor < other.minor) ||
                (major == other.major && minor == other.minor && patch < other.patch);
    }

    constexpr bool operator<=(const Version& other) const {
        return (*this < other) || (*this == other);
    }

    constexpr bool operator>(const Version& other) const {
        return !(*this <= other);
    }

    constexpr bool operator>=(const Version& other) const {
        return !(*this < other);
    }

    std::string toString() const {
        return  std::to_string(major) + "." + 
                std::to_string(minor) + "." + 
                std::to_string(patch);
    }

    static Version fromString(const std::string &version) {
        auto parts = x::split(version, ".");
        if (parts.size() != 3) return {};
        return Version(std::stoi(parts[0]),std::stoi(parts[1]), std::stoi(parts[2]));
    }
};

}
#pragma once

#include "itf/itf.hpp"

inline constexpr auto VER_DEMO_ITF      = nb::Version{1,0,0};
inline constexpr auto VER_DEMO_ITF_MIN  = nb::Version{1,0,0};

class IntefaceDemo: public nb::ITF {
public:
    virtual ~IntefaceDemo() = default;

    void feed(const std::string &data);

protected:
    IntefaceDemo() : nb::ITF("IntefaceDemo",VER_DEMO_ITF,VER_DEMO_ITF_MIN) {}
};

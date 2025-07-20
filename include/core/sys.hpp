#pragma once

#include "x/x.hpp"

namespace nb::sys {

x::u64 proc_id();
x::str proc_path();
x::str proc_name();
x::str proc_dir();

x::str get_env(x::cStr& name);
bool   set_env(x::cStr& name, x::cStr& value);

x::str run_cmd(x::cStr& cmd);
void*  new_only(x::cStr& name);
bool   rm_only(void * obj,x::cStr& name = "");
bool   has_only(x::cStr& name);
}

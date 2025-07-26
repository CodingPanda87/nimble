#include <iostream>
#include "nb.hpp"

int main() {
    auto& platform =  *nb::Platform::instance();
    if(!platform.init("cfg.json")){
        std::cout << "platform init failed" << std::endl;
        return 1;
    }
    while(getchar() != 'q'){
        std::cout << "Hello, World!" << std::endl;
        x::sleep(100);
    }
    platform.stop();
    std::cerr << x::Time::now().to_string() << std::endl;
    return 0;
}
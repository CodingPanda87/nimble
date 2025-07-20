#include <iostream>
#include "nb.hpp"

int main() {
    nb::Platform platform;
    if(!platform.init("cfg.json")){
        std::cout << "platform init failed" << std::endl;
        return 1;
    }
    while(getchar() != 'q'){
        std::cout << "Hello, World!" << std::endl;
        x::sleep(100);
    }
    platform.stop();
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
#include "clew.h"
#include "deviceinfo_helper.h"
#include <iostream>
#include <cassert>

int main() {
    bool clpresent = 0 == clewInit();

    if( !clpresent ) {
        std::cout << "opencl library not found." << std::endl;
        return -1;
    }

    bool to_show_gpu_config = true;

    if (to_show_gpu_config)
        show_gpu_config();

    return 0;
}


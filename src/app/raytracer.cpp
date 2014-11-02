#include "../3rd/clew/include/clew.h"
#include "../3rd/OpenCLHelper/deviceinfo_helper.h"
#include <iostream>
#include <cassert>

void show_gpu_config()
{
    cl_int error = 0;   // Used to handle error codes
    cl_platform_id platform_id;
    cl_context context;
    cl_command_queue queue;
    cl_device_id device;

    // Platform
    cl_uint num_platforms;
    error = clGetPlatformIDs(1, &platform_id, &num_platforms);
    std::cout << "num platforms: " << num_platforms << std::endl;
    assert (num_platforms == 1);
    assert (error == CL_SUCCESS);

    // Device
    cl_uint num_devices;
    error = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device, &num_devices);
    if (error != CL_SUCCESS) {
        std::cout << "Error getting device ids: " << error << std::endl;
        exit(error);
    }
    std::cout << "num devices: " << num_devices << std::endl;
    cl_device_id *device_ids = new cl_device_id[num_devices];
    error = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, num_devices, device_ids, &num_devices);
    if (error != CL_SUCCESS) {
        std::cout << "Error getting device ids: " << error << std::endl;
        exit(error);
    }
    for( int i = 0; i < num_devices; i++ ) {
        device = device_ids[i];
        std::cout << "device " << i << " id " << device << std::endl;
        printDeviceInfoMB( "global memory size", device, CL_DEVICE_GLOBAL_MEM_SIZE );
        printDeviceInfoKB( "local memory size", device, CL_DEVICE_LOCAL_MEM_SIZE );
        printDeviceInfoKB( "global cache size", device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE );
        printDeviceInfo( "global cacheline size", device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE );
        printDeviceInfo( "max compute units", device, CL_DEVICE_MAX_COMPUTE_UNITS );
        printDeviceInfo( "max workgroup size", device, CL_DEVICE_MAX_WORK_GROUP_SIZE );
        printDeviceInfo( "max workitem dimensions", device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS );
        printDeviceInfoArray( "max workitem sizes", device, CL_DEVICE_MAX_WORK_ITEM_SIZES, 3 );
        printDeviceInfoString( "device name", device, CL_DEVICE_NAME );
        printDeviceInfo( "frequency MHz", device, CL_DEVICE_MAX_CLOCK_FREQUENCY );
        std::cout << std::endl;
    }
}


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


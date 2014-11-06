#include <OpenCL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

const int kWidth = 1366;
const int kHeight = 768;
const bool kFullscreen = false;

size_t global_work_size = kWidth * kHeight;

float viewMatrix[16];

float sphere1Pos[3] = {0,0,10};
float sphere2Pos[3] = {0,0,-10};
float sphereVelocity = 1;
float sphereTransforms[2][16];

cl_command_queue queue;
cl_kernel kernel;
cl_mem buffer, viewTransform, worldTransforms;

SDL_Window* window;

void InitOpenCL()
{
    // 1. Get a platform.
    cl_platform_id platform;
    
    clGetPlatformIDs( 1, &platform, NULL );
    // 2. Find a gpu device.
    cl_device_id device;
    
    clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU,
                   1,
                   &device,
                   NULL);
    // 3. Create a context and command queue on that device.
    cl_context context = clCreateContext( NULL,
                                         1,
                                         &device,
                                         NULL, NULL, NULL);
    queue = clCreateCommandQueue( context,
                                 device,
                                 0, NULL );
    // 4. Perform runtime source compilation, and obtain kernel entry point.
    std::ifstream file("scene.cl");
    std::string source;
    if (file){
    while(!file.eof()){
        char line[256];
        file.getline(line,255);
        source += std::string(line) + "\n";
    }
    }
    if (source.length()==0)
    {
        std::string err = "fail to load shader";
    }
    
    cl_ulong maxSize;
    clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE , sizeof(cl_ulong), &maxSize, 0);
    
    const char* str = source.c_str();
    cl_program program = clCreateProgramWithSource( context,
                                                   1,
                                                   &str,
                                                   NULL, NULL );
    cl_int result = clBuildProgram( program, 1, &device, NULL, NULL, NULL );
    if ( result ){
        char* build_log;
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        build_log = new char[log_size+1];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
        build_log[log_size] = '\0';
        if( log_size > 2 ) {
            std::cout << "build log: " << build_log << std::endl;
        }
        delete[] build_log;
        std::cout << "Error during compilation! (" << result << ")" << std::endl;
    }
    kernel = clCreateKernel( program, "tracekernel", NULL );
    // 5. Create a data buffer.
    buffer        = clCreateBuffer( context,
                                   CL_MEM_WRITE_ONLY,
                                   kWidth * kHeight *sizeof(cl_float4),
                                   NULL, 0 );
    viewTransform = clCreateBuffer( context,
                                   CL_MEM_READ_WRITE,
                                   16 *sizeof(cl_float),
                                   NULL, 0 );
    
    worldTransforms = clCreateBuffer( context,
                                     CL_MEM_READ_WRITE,
                                     16 *sizeof(cl_float)*2,
                                     NULL, 0 );
    
    clSetKernelArg(kernel, 0, sizeof(buffer), (void*) &buffer);
    clSetKernelArg(kernel, 1, sizeof(cl_uint), (void*) &kWidth);
    clSetKernelArg(kernel, 2, sizeof(cl_uint), (void*) &kWidth);
    clSetKernelArg(kernel, 3, sizeof(viewTransform), (void*) &viewTransform);
    clSetKernelArg(kernel, 4, sizeof(worldTransforms), (void*) &worldTransforms);
}

void Render(float delta)
{
    clEnqueueNDRangeKernel(   queue,
                           kernel,
                           1,
                           NULL,
                           &global_work_size,
                           NULL, 0, NULL, NULL);
    
    // 7. Look at the results via synchronous buffer map.
    cl_float4 *ptr = (cl_float4 *) clEnqueueMapBuffer( queue,
                                                      buffer,
                                                      CL_TRUE,
                                                      CL_MAP_READ,
                                                      0,
                                                      kWidth * kHeight * sizeof(cl_float4),
                                                      0, NULL, NULL, NULL );
    
    cl_float *viewTransformPtr = (cl_float *) clEnqueueMapBuffer( queue,
                                                                 viewTransform,
                                                                 CL_TRUE,
                                                                 CL_MAP_WRITE,
                                                                 0,
                                                                 16 * sizeof(cl_float),
                                                                 0, NULL, NULL, NULL );
    
    cl_float *worldTransformsPtr = (cl_float *) clEnqueueMapBuffer( queue,
                                                                   worldTransforms,
                                                                   CL_TRUE,
                                                                   CL_MAP_WRITE,
                                                                   0,
                                                                   16 * sizeof(cl_float)*2,
                                                                   0, NULL, NULL, NULL );
    
    
    memcpy(viewTransformPtr, viewMatrix, sizeof(float)*16);
    memcpy(worldTransformsPtr, sphereTransforms[0], sizeof(float)*16);
    memcpy(worldTransformsPtr+16, sphereTransforms[1], sizeof(float)*16);
    
    
    clEnqueueUnmapMemObject(queue, viewTransform, viewTransformPtr, 0, 0, 0);
    clEnqueueUnmapMemObject(queue, worldTransforms, worldTransformsPtr, 0, 0, 0);
    
    unsigned char* pixels = new unsigned char[kWidth*kHeight*4];
    for(int i=0; i <  kWidth * kHeight; i++){
        pixels[i*4] = ptr[i].s[0]*255;
        pixels[i*4+1] = ptr[i].s[1]*255;
        pixels[i*4+2] = ptr[i].s[2]*255;
        pixels[i*4+3] = 1;
    }
    
    glBindTexture(GL_TEXTURE_2D, 1);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexImage2D(GL_TEXTURE_2D, 0, 4, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    delete [] pixels;
    
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1,1,1,-1,1,100);
    glMatrixMode(GL_MODELVIEW);
	   
    glLoadIdentity();
    glBegin(GL_QUADS);
    glTexCoord2f(0,1);
    glVertex3f(-1,-1,-1);
    glTexCoord2f(0,0);
    glVertex3f(-1,1,-1);
    glTexCoord2f(1,0);
    glVertex3f(1,1,-1);
    glTexCoord2f(1,1);
    glVertex3f(1,-1,-1);
    glEnd();
    
    clFinish( queue );
    SDL_GL_SwapWindow(window);
}

void Update(float delta)
{
    
    float translate[3] = {0,0.0,0};
    
    int count;
    const Uint8* keys = SDL_GetKeyboardState(&count);
    if ( keys[SDL_SCANCODE_DOWN] ){
        translate[2] = -0.01*delta;
    }
    if ( keys[SDL_SCANCODE_UP] ){
        translate[2] = 0.01*delta;
    }
    if ( keys[SDL_SCANCODE_LEFT] ){
        translate[0] =- 0.01*delta;
    }
    if ( keys[SDL_SCANCODE_RIGHT] ){
        translate[0] = 0.01*delta;
    }
    
    int x,y;
    SDL_GetMouseState(&x,&y);
    int relX = (kWidth/2.0f - x)*delta;
    int relY = (kHeight/2.0f - y)*delta;
    
    glMatrixMode(GL_MODELVIEW);
    
    glLoadIdentity();
    glMultMatrixf(viewMatrix);
    glTranslatef(translate[0],translate[1],translate[2]); 
    
    if ( relX != 0){
        glRotatef(-relX/200.0f, 0, 1, 0);
    }
    if ( relY != 0){
        glRotatef(-relY/200.0f, 1, 0, 0);
    }
    
    glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);
    
    // Sphere Transforms
    glLoadIdentity();
    glTranslatef(0, 0, sphere1Pos[2]);
    glGetFloatv(GL_MODELVIEW_MATRIX, sphereTransforms[0]);
    
    glLoadIdentity();
    glTranslatef(0, 0, sphere2Pos[2]);
    glGetFloatv(GL_MODELVIEW_MATRIX, sphereTransforms[1]);
    
    sphere1Pos[2] += sphereVelocity*delta/30.0f;
    sphere2Pos[2] += sphereVelocity*(-1)*delta/30.0f;
    
    if ( sphere1Pos[2] > 50 ){
        sphereVelocity = -1;
    }
    else if ( sphere1Pos[2] < -50 ){
        sphereVelocity = 1;
    }
}

int main(int argc, char* argv[])
{
    InitOpenCL();
    
    memset(viewMatrix, 0, sizeof(float)*16);
    viewMatrix[0] = viewMatrix[5] = viewMatrix[10] = viewMatrix[15] = 1;
    
    
    SDL_Init(SDL_INIT_EVERYTHING);

        Uint32 flags = SDL_WINDOW_OPENGL;
        if ( kFullscreen ){
            flags |= SDL_WINDOW_FULLSCREEN;

            SDL_ShowCursor(0);
        }

        window = SDL_CreateWindow("RayTracer",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                kWidth, kHeight,
                flags);
    
    SDL_GL_CreateContext(window);
    
    SDL_WarpMouseInWindow(window, kWidth/2.0f, kHeight/2.0f);
    
    glEnable(GL_TEXTURE_2D);
    
        bool loop = true;
        int lastTicks = SDL_GetTicks();
        while(loop){
            int delta = SDL_GetTicks() - lastTicks;
            lastTicks = SDL_GetTicks();
            SDL_Event e;
            while(SDL_PollEvent(&e)){
                if ( e.type == SDL_QUIT){
                    loop = false;
                }
                else if ( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
                    loop = false;
                }
            }
        
            delta *= 0.1;
        Update(delta);
        Render(delta);
        
            std::stringstream ss;
            ss << 1000.0f / delta ;
            SDL_SetWindowTitle(window, ss.str().c_str());
        }
    
    SDL_DestroyWindow(window);
    return 0;
}

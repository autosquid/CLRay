#include "clew.h"
#include "show_gpu_info.hpp"
#include <iostream>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"
#include "OpenCLHelper.h"
#include "CLKernel.h"
#include "CLArrayFloat.h"


const int kWidth = 1366;
const int kHeight = 768;
const bool kFullscreen = true;

size_t global_work_size = kWidth * kHeight;
size_t local_work_size = 1;

float viewMatrix[16];

float sphere1Pos[3] = {0,0,10};
float sphere2Pos[3] = {0,0,-10};
float sphereVelocity = 1;
float sphereTransforms[2][16];

CLArrayFloat* buffer, *viewTransform, *worldTransforms; //on device
CLKernel* kernel;

SDL_Window* window;

void InitOpenCL()
{
    if( !OpenCLHelper::isOpenCLAvailable() ) {
        std::cout << "opencl library not found" << std::endl;
        exit(-1);
    }
    std::cout << "found opencl library" << std::endl;

    OpenCLHelper cl(0);
    auto kernel_src_file = std::string("/Users/uuplusu/Github/CLRay/shaders/scene.cl");
    kernel = cl.buildKernel(kernel_src_file, "tracekernel");

    buffer = cl.arrayFloat(kWidth * kHeight *sizeof(cl_float4));

    kernel->output(buffer);
    kernel->input(kWidth);
    kernel->input(kHeight);

    viewTransform = cl.arrayFloat(16);
    viewTransform->createOnDevice();

    worldTransforms = cl.arrayFloat(32);

    kernel->inout(viewTransform);
    kernel->input(worldTransforms);
}

void Render(int delta) {

    kernel->run(1, &global_work_size, &local_work_size);

    memcpy(viewTransform, viewMatrix, sizeof(float)*16);
    memcpy(worldTransforms, sphereTransforms[0], sizeof(float)*16);
    memcpy(worldTransforms+16, sphereTransforms[1], sizeof(float)*16);

    auto pixels  = new unsigned char[kWidth*kHeight*4];
    for(int i=0; i <  kWidth * kHeight*4; i++){
        if (i%4==3){
            pixels[i] = 255;
        }else{
            pixels[i] = (*buffer)[i]*255;
        }
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

    SDL_GL_SwapWindow(window);
}

void Update(int delta)
{
    int count;
    auto keys = SDL_GetKeyboardState(&count);

    float translate[3] = {0,0,0};
    if ( keys[SDLK_DOWN] ){
        translate[2] = -0.01*delta;
    }
    if ( keys[SDLK_UP] ){
        translate[2] = 0.01*delta;
    }
    if ( keys[SDLK_LEFT] ){
        translate[0] =- 0.01*delta;
    }
    if ( keys[SDLK_RIGHT] ){
        translate[0] = 0.01*delta;
    }

    int x,y;
    SDL_GetMouseState(&x,&y);
    int relX = (kWidth/2.0f - x)*delta;
    int relY = (kHeight/2.0f - y)*delta;
    SDL_WarpMouseInWindow(window, kWidth/2.0f, kHeight/2.0f);

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

void CLRayTracer()
{
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

        glEnable(GL_TEXTURE_2D);

        bool loop = true;
        int lastTicks = SDL_GetTicks();
        while(loop){
            int delta = SDL_GetTicks() - lastTicks;
            lastTicks = SDL_GetTicks();
            SDL_Event e;
            while(SDL_PollEvent(&e)){
                if ( e.type == SDL_QUIT ){
                    loop = false;
                }
                else if ( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
                    loop = false;
                }
            }

            Update(delta);
            Render(delta);

            std::stringstream ss;
            ss << 1000.0f / delta ;
            SDL_SetWindowTitle(window, ss.str().c_str());
        }

        SDL_DestroyWindow(window);
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
        gpuconfig::show_gpu_config();

    CLRayTracer();
    return 0;
}


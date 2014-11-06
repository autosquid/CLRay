add_executable(main raytracer.cpp)
target_link_libraries(main RayCoreLib ${SDL_LIBRARY} ${OPENGL_LIBRARY} ${OPENCL_LIBRARIES})

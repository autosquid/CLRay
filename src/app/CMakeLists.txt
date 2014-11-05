add_executable(main raytracer.cpp)
target_link_libraries(main RayCoreLib ${SDL2_LIBRARY} ${OPENGL_LIBRARIES})

file(GLOB cpp_s "*.cpp")
add_executable(cpuray ${cpp_s})
target_link_libraries(cpuray RayCoreLib ${SDL2_LIBRARY} ${OPENGL_LIBRARIES})

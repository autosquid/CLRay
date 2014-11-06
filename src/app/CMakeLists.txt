add_executable(main raytracer.cpp)
target_link_libraries(main RayCoreLib ${SDL2_LIBRARY} ${OPENGL_LIBRARY} ${OPENCL_LIBRARIES})
add_custom_command(TARGET main PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:main>)

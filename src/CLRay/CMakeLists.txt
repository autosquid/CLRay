file(GLOB_RECURSE h_s "*.hpp")
file(GLOB_RECURSE cpp_s "*.cpp")

add_library(CLRayLib ${cpp_s})
target_link_libraries(CLRayLib clew OpenHelper)

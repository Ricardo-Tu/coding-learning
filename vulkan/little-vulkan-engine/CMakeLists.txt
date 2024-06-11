cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(lve
        LANGUAGES CXX
        DESCRIPTION "little vulkan engine")

include(cmake/CMakeLists.txt)
find_program(GLSLC_PROGRAM glslc REQUIRED)

message(STATUS "run glslc to compile shaders ...")
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_SOURCE_DIR}/vert.spv)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_SOURCE_DIR}/frag.spv)
message(STATUS "compile shader OK")

aux_source_directory(src SRC)

add_library(lve STATIC ${SRC})
target_include_directories(lve PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(lve PUBLIC Vulkan::Vulkan)
target_link_libraries(lve PUBLIC tinyobjloader::tinyobjloader)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
target_compile_features(lve PUBLIC cxx_std_17)

# 添加可执行文件
add_executable(app_lve ./src/main.cpp)
aux_source_directory(./ SANDBOX_SRC)
target_link_libraries(app_lve PUBLIC lve SDL2)
CopyShader(app_lve)
CopyTexture(app_lve)
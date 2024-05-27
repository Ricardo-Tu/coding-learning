cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(colortriangle VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找 OpenGL 包
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

# 查找 GLFW 包
# find_package(glfw3 3.3 REQUIRED)

# 查找 GLEW 包
# find_package(GLEW REQUIRED)

# 包含头文件目录
# include_directories(${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})

# 添加可执行文件
file(GLOB SOURCES "./src/*.cpp")
add_executable(colortriangle ${SOURCES})

# 链接库
target_link_libraries(colortriangle ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})

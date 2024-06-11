if (NOT TARGET SDL2)
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        add_library(SDL2 ALIAS SDL2::SDL2)
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2 sdl2 REQUIRED IMPORTED_TARGET)
        add_library(SDL2 ALIAS PkgConfig::SDL2)
    endif()
endif()

find_package(Vulkan REQUIRED)
find_package(tinyobjloader REQUIRED)

macro(CopyShader target_name)
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/vert.spv $<TARGET_FILE_DIR:${target_name}>)
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/frag.spv $<TARGET_FILE_DIR:${target_name}>)
endmacro(CopyShader)

macro(CopyTexture target_name)
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${target_name}>/resources)
endmacro(CopyTexture)
                
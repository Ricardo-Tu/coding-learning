if (NOT TARGET SDL2)
    if(WIN32)
        set(SDL2_ROOT "" CACHE PATH "SDL2 root directory")
        set(SDL2_INCLUDE_DIR "${SDL2_ROOT}/lib/x64")
        set(SDL2_LIB_DIR "{SDL2_ROOT} /lib/x64")
        add_library(SDL2::SDL2 SHARED IMPORTED GLOBAL)
        set_target_properties(
            SDL2::SDL2_BIN_DIR
            PROPERITES
                IMPORTED_LICATION "{SDL2_LIB_DIR} /SDL2.dll"
                IMPORTED_IMPLIB "{SDL2_LIB_DIR} /SDL2.lib"
                INTERFACE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIR}
        )
        add_library(SDL2::SDL2main SHARED IMPORTED GLOBAL)
        set_target_properties(
            SDL2::SDL2main
            PROPERTIES 
                IMPORTED_LICATION "${SDL2_LIB_DIR}/SDL2.dll"
                IMPORTED_IMPLIB "${SDL2_LIB_DIR}/SDL2main.lib"
                INTERFACE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIR}
        )
        add_library(SDL2 INTERFACE IMPORTED GLOBAL)
        target_link_libraries(SDL2 INTERFACE SDL2::SDL2 SDL2::SDL2main)
    else()
        find_package(SDL2 QUIET)
        if(SDL2_FOUND)
            add_library(SDL2 ALIAS SDL2::SDL2)
        else()
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(SDL2 sdl2 REQUIRED IMPORTED_TARGET)
            add_library(SDL2 ALIAS PkgConfig::SDL2)
        endif()
    endif()
endif()
cmake_minimum_required(VERSION 3.19)

project(else_subset LANGUAGES C CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/picked_externals.cmake)

set(ELSE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE STRING "Path to the ELSE source directory")

# TODO: PD_FLOATSIZE

# Add Linux-specific configurations
if(UNIX AND NOT APPLE)
    # Set proper flags for forward compatibility
    add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2 -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2 -O2")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_MACOSX_RPATH ON)

# Add static linking on Windows
if(WIN32)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

if(UNIX AND NOT APPLE)
    set(CMAKE_BUILD_RPATH "$ORIGIN")
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()
if(APPLE)
    set(CMAKE_BUILD_RPATH "@loader_path")
    set(CMAKE_INSTALL_RPATH "@loader_path")
endif()

# TODO: should it be this low?
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)



# instead of including ${ELSE_SOURCE_DIR}/Source/Shared/shared.cmake

file(GLOB SHARED_SOURCES
    ${ELSE_SOURCE_DIR}/Source/Shared/*.c
    ${ELSE_SOURCE_DIR}/Source/Shared/libsamplerate/*.c
)

add_library(else_shared STATIC ${SHARED_SOURCES})
target_compile_definitions(else_shared PRIVATE $<$<BOOL:${PD_MULTI}>:PDINSTANCE=1 PDTHREADS=1>)
set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_STATIC_LIBRARIES else_shared)

if(APPLE)
    target_link_options(else_shared PRIVATE -undefined dynamic_lookup)
elseif(WIN32)
	target_link_libraries(else_shared PRIVATE ws2_32)
endif()

include_directories(${PD_SOURCES_PATH} ${ELSE_SOURCE_DIR}/Source/Shared)



set(SETUP_DECLS "")
set(SETUP_CALLS "")
set(LIBS_LIST)

foreach(EXTERNAL_NAME ${PICKED_ELSE_EXTERNALS})
    string(FIND ${EXTERNAL_NAME} "~" NAME_HAS_TILDE)
    if(NAME_HAS_TILDE GREATER -1)
        set(SOURCE_SUBDIR "Audio")
    else()
        set(SOURCE_SUBDIR "Control")
    endif()

    string(REPLACE "~" "_tilde" TARGET_NAME "${EXTERNAL_NAME}")

    add_library(${TARGET_NAME} STATIC ${ELSE_SOURCE_DIR}/Source/${SOURCE_SUBDIR}/${EXTERNAL_NAME}.c)
    target_compile_definitions(${TARGET_NAME} PRIVATE $<$<BOOL:${PD_MULTI}>:PDINSTANCE=1 PDTHREADS=1>)
    list(APPEND LIBS_LIST ${TARGET_NAME})

    string(FIND ${EXTERNAL_NAME} "." NAME_HAS_DOT)
    if(NAME_HAS_DOT EQUAL -1)
        string(REPLACE "~" "_tilde" EXPORT_FN_NAME "${EXTERNAL_NAME}_setup")
    else()
        string(REPLACE "." "0x2e" TEMP_NAME "${EXTERNAL_NAME}")
        string(REPLACE "~" "_tilde" EXPORT_FN_NAME "setup_${TEMP_NAME}")
    endif()

    string(APPEND SETUP_DECLS "void ${EXPORT_FN_NAME}();\n")
    string(APPEND SETUP_CALLS "    ${EXPORT_FN_NAME}();\n")
endforeach(EXTERNAL_NAME)

# manually add [lb] and [s2f~] from Extra/Aliases
# because they are used in a lot of abstractions
add_library(lb STATIC ${ELSE_SOURCE_DIR}/Source/Extra/Aliases/lb.c)
target_compile_definitions(lb PRIVATE $<$<BOOL:${PD_MULTI}>:PDINSTANCE=1 PDTHREADS=1>)
list(APPEND LIBS_LIST lb)
string(APPEND SETUP_DECLS "void lb_setup();\n")
string(APPEND SETUP_CALLS "    lb_setup();\n")

add_library(s2f_tilde STATIC ${ELSE_SOURCE_DIR}/Source/Extra/Aliases/s2f~.c)
target_compile_definitions(s2f_tilde PRIVATE $<$<BOOL:${PD_MULTI}>:PDINSTANCE=1 PDTHREADS=1>)
list(APPEND LIBS_LIST s2f_tilde)
string(APPEND SETUP_DECLS "void s2f_tilde_setup();\n")
string(APPEND SETUP_CALLS "    s2f_tilde_setup();\n")



configure_file(else_subset.c.in ${CMAKE_CURRENT_BINARY_DIR}/else_subset.c)
add_library(else_subset STATIC ${CMAKE_CURRENT_BINARY_DIR}/else_subset.c)
if(MSVC)
    # TODO: test this on an actual windows machine
    set_property(TARGET else_subset APPEND_STRING PROPERTY LINK_FLAGS "/export:else_subset_setup")
endif()
foreach(LIB_NAME ${LIBS_LIST})
    target_link_libraries(else_subset ${LIB_NAME})
endforeach()
target_link_libraries(else_subset else_shared)

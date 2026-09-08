cmake_minimum_required(VERSION 3.17)
project(astral_engine VERSION 20.10.0 LANGUAGES CXX C)
include(FetchContent)

# Variables
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

# CCache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    message(STATUS "Using ccache for faster compilation.")
endif()

# GLFW 
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(glfw)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)

# GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8 
    SOURCE_SUBDIR  cmake
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad_gl_core_45 STATIC LOADER API gl:core=4.5)
# add_library(glad STATIC 
#     ${VENDOR_DIR}/glad/src/gl.c
# )
# target_include_directories(glad PUBLIC 
#     ${VENDOR_DIR}/glad/include
# )

# GLM
FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG         1.0.1	
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(glm)

# ImGui
FetchContent_Declare(
    imgui	
	GIT_REPOSITORY	https://github.com/ocornut/imgui
	GIT_TAG         v1.92.2b	
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(imgui)
add_library(imgui_lib
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC glfw)

# AssImp
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp
    GIT_TAG v6.0.5
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(assimp)
option(ASSIMP_BUILD_TESTS off)
option(ASSIMP_INSTALL off)

# Project
add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC 
    ${INCLUDE_DIR} 
)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${SRC_DIR}/*.cpp"
)
file(COPY ${RESOURCES_DIR}
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
target_sources(${PROJECT_NAME} PRIVATE
    ${SRC_FILES}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad_gl_core_45
    glm
    imgui_lib
    assimp::assimp
    ${CMAKE_DL_LIBS} # Needed for glad - https://stackoverflow.com/a/56842079/2394163
    $<$<PLATFORM_ID:Windows>:winmm>
)
add_definitions(-DGLFW_INCLUDE_NONE)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    COMPILE_OPTIONS -Werror
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

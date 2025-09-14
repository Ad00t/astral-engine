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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

# GLFW - Installed via: sudo dnf install -y glfw-devel
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)
find_package(glfw3 REQUIRED)

# GLAD
add_library(glad STATIC 
    ${VENDOR_DIR}/glad/src/gl.c
)
target_include_directories(glad PUBLIC 
    ${VENDOR_DIR}/glad/include
)

# GLM
FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG         1.0.1	
)
FetchContent_MakeAvailable(glm)

# ImGui
FetchContent_Declare(
    imgui	
	GIT_REPOSITORY	https://github.com/ocornut/imgui
	GIT_TAG         v1.92.2b	
)
FetchContent_MakeAvailable(imgui)
add_library(imgui_lib
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    # add backends you want:
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

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
    glad
    glm
    imgui_lib
    ${CMAKE_DL_LIBS} # Needed for glad - https://stackoverflow.com/a/56842079/2394163
)
add_definitions(-DGLFW_INCLUDE_NONE)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    COMPILE_OPTIONS -Werror
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

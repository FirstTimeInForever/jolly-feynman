cmake_minimum_required(VERSION 3.12)
project(object-viewer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

find_package(glfw CONFIG)
find_package(glew CONFIG)
find_package(fmt CONFIG)
find_package(glm CONFIG)
find_package(assimp CONFIG)
find_package(stb CONFIG)
find_package(imgui CONFIG)

add_executable(object-viewer
    src/main.cpp
    src/mesh.hpp
    src/model.hpp
    src/shader_program.cpp
    src/shader_program.hpp
    src/camera.hpp
    src/skybox.hpp
    src/stb_image_wrapper.hpp
    shaders/scene_fragment.glsl
    shaders/scene_vertex.glsl
    shaders/skybox_vertex.glsl
    shaders/skybox_fragment.glsl
    bindings/imgui_impl_glfw.cpp
    bindings/imgui_impl_glfw.h
    bindings/imgui_impl_opengl3.cpp
    bindings/imgui_impl_opengl3.h
)

add_custom_command(
    TARGET object-viewer
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/shaders ${PROJECT_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/assets
)

target_link_libraries(object-viewer GLEW::glew_s glfw::glfw fmt::fmt glm::glm assimp::assimp stb::stb imgui::imgui)

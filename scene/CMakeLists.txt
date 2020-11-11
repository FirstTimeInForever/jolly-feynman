cmake_minimum_required(VERSION 3.12)
project(scene)

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

add_executable(scene
    src/main.cpp
    src/mesh.hpp
    src/model.hpp
    src/shader_program.cpp
    src/shader_program.hpp
    src/camera.hpp
    src/skybox.hpp
    src/heightmap.hpp
    src/light.hpp
    src/simple_cube.hpp
    src/shadows.hpp
    src/ui.hpp
    src/utility.hpp
    src/water.hpp
    src/stb_image_wrapper.hpp
    shaders/water_vertex.glsl
    shaders/water_fragment.glsl
    shaders/terrain_vertex.glsl
    shaders/terrain_fragment.glsl
    shaders/common_object_fragment.glsl
    shaders/common_object_vertex.glsl
    shaders/simple_object_fragment.glsl
    shaders/simple_object_vertex.glsl
    shaders/depth_fragment.glsl
    shaders/depth_vertex.glsl
    bindings/imgui_impl_glfw.cpp
    bindings/imgui_impl_glfw.h
    bindings/imgui_impl_opengl3.cpp
    bindings/imgui_impl_opengl3.h
)

add_custom_command(
    TARGET scene
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/shaders ${PROJECT_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/assets
)

target_link_libraries(scene GLEW::glew_s glfw::glfw fmt::fmt glm::glm assimp::assimp stb::stb imgui::imgui)

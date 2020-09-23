#include <iostream>
#include <vector>
#include <chrono>

#include <fmt/format.h>

#include <GL/glew.h>

#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/constants.hpp>

#include "opengl_shader.h"

#include <tuple>
#include <array>

static void glfw_error_callback(int error, const char *description) {
    std::cerr << fmt::format("Glfw Error {}: {}\n", error, description);
}


GLfloat vertices[] = {
    -1.0, -1.0,
    1.0, -1.0,
    1.0, 1.0,
    -1.0, 1.0
};

GLuint vertex_indices[] = {
    0, 1, 2,
    2, 3, 0
};

// Raw 2x2 texture
float texture[] = {
    0/255.0f, 0/255.0f, 0/255.0f,
    255/255.0f, 70/255.0f, 45/255.0f,
    255/255.0f, 200/255.0f, 100/255.0f,
    0/255.0f, 23/255.0f, 12/255.0f
};

auto create_buffers() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);

    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, tex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 4, 0, GL_RGB, GL_FLOAT, texture);
    glBindTexture(GL_TEXTURE_1D, 0);

    return std::make_tuple(vbo, vao, ebo, tex);
}

void bind_shader_attributes(shader_t& shader_program) {
    GLint position_attribute = glGetAttribLocation(shader_program.program_id_, "ipos");
    glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(position_attribute);
    GLint ipos = glGetAttribLocation(shader_program.program_id_, "pos");
    glVertexAttribPointer(ipos, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(ipos);
}

auto get_window_size(GLFWwindow* window) {
    int width = 0;
    int height = 0;
    glfwGetWindowSize(window, &width, &height);
    return std::make_tuple((float)width, (float)height);
}

auto handle_mouse_drag(GLFWwindow* window) {
    auto delta = ImGui::GetMouseDragDelta(0, 0);
    ImGui::ResetMouseDragDelta();
    auto [width, height] = get_window_size(window);
    float x = delta.x / width * 2;
    float y = delta.y / height * 2;
    return std::make_tuple(-x, y);
}

auto handle_mouse_wheel(GLFWwindow* window, float cx, float cy, float zoom) {
    float wheel = ImGui::GetIO().MouseWheel;
    if (std::abs(wheel) < 0.1) {
        return std::make_tuple(0, 0, 1);
    }
    auto mouse = ImGui::GetMousePos();
    auto [width, height] = get_window_size(window);
    auto mx = mouse.x * 2;
    auto my = mouse.y * 2;
    float x = (mx / width) - 1;
    float y = (my / height) - 1;
    if (wheel > 0) {
        wheel = 1 / 1.2;
    }
    else {
        wheel = 1.2;
    }
    std::cout << wheel << std::endl;
    const auto nz = (1 - wheel) * zoom;
    float tx = (x + cx) * nz;
    float ty = (-y + cy) * nz;
    return std::make_tuple(tx, ty, wheel);
}

int main(int, char **) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui - Conan", NULL, NULL);
    if (window == NULL) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!\n";
        return 1;
    }

    auto [vbo, vao, ebo, tex] = create_buffers();
    shader_t shader_program("vertex.glsl", "fragment.glsl");
    bind_shader_attributes(shader_program);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    float zoom = 1.0f;
    float centerx = 0.0f;
    float centery = 0.0f;
    float max_radius = 8.0f;
    int max_iterations = 25;
    float shiftx = 0.0f;
    float shifty = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        glViewport(0, 0, display_w, display_h);

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            auto [x, y, z] = handle_mouse_wheel(window, centerx, centery, zoom);
            shiftx += x;
            shifty += y;
            zoom *= z;
        }

        if (!ImGui::IsAnyWindowFocused()) {
        auto [x, y] = handle_mouse_drag(window);
            centerx += x;
            centery += y;
        }

        ImGui::Begin("Fractal props");
        ImGui::SetWindowSize(ImVec2(300, 200));
        ImGui::SliderFloat("Zoom", &zoom, 0.01, 100);
        ImGui::SliderFloat("Max Radius", &max_radius, 0, 20);
        ImGui::SliderInt("Max Iterations", &max_iterations, 1, 100);
        if (ImGui::Button("Reset Center")) {
            centerx = 0.0;
            centery = 0.0;
            shiftx = 0.0;
            shifty = 0.0;
        }
        ImGui::End();


        shader_program.set_uniform("zoom", zoom);
        shader_program.set_uniform("max_iterations", max_iterations);
        shader_program.set_uniform("center", centerx, centery);
        shader_program.set_uniform("shift", shiftx, shifty);
        shader_program.set_uniform("max_radius", max_radius);
        shader_program.set_uniform("tex", 0);
        shader_program.use();
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, tex);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 4, 0, GL_RGB, GL_FLOAT, texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

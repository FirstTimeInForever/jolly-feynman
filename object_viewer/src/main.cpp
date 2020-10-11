#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fmt/format.h>
#include <optional>
#include <array>

#include <imgui.h>
#include "../bindings/imgui_impl_glfw.h"
#include "../bindings/imgui_impl_opengl3.h"

#include "shader_program.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "skybox.hpp"


const unsigned int initial_width = 1080;
const unsigned int initial_height = 920;
float lastx = (float)initial_width / 2.0;
float lasty = (float)initial_height / 2.0;
bool first_mouse = true;
camera scene_camera(glm::vec3(0.0f, 0.0f, 10.0f));

void init_ui(GLFWwindow* window) {
    const char* glsl_version = "#version 330";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
}

void dispose_ui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

std::optional<GLFWwindow*> init() {
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << fmt::format("glfw error {}: {}\n", error, description);
    });
    if (!glfwInit()) {
        return std::nullopt;
    }
    // GL 3.3 + GLSL 330
    //const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    auto window = glfwCreateWindow(initial_width, initial_height, "OBJ Viewer", nullptr, nullptr);
    if (!window) {
        return std::nullopt;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!\n";
        return std::nullopt;
    }
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        if (first_mouse) {
            lastx = (float)xpos;
            lasty = (float)ypos;
            first_mouse = false;
        }
        float xoffset = xpos - lastx;
        float yoffset = lasty - ypos;
        lastx = (float)xpos;
        lasty = (float)ypos;
        if (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS) {
            return;
        }
        scene_camera.mouse_move(xoffset, yoffset);
    });


    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        scene_camera.mouse_scroll((float)yoffset);
    });
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    init_ui(window);
    return std::make_optional(window);
}

auto get_window_size(const GLFWwindow* window) {
    int width;
    int height;
    glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);
    return std::make_tuple(width, height);
}

void process_input(GLFWwindow* window, float delta_time) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        scene_camera.keyboard(camera::direction::forward, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        scene_camera.keyboard(camera::direction::backward, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        scene_camera.keyboard(camera::direction::left, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        scene_camera.keyboard(camera::direction::right, delta_time);
    }
}

int main(int argc, char** argv) {
    auto init_result = init();
    if (!init_result.has_value()) {
        return 1;
    }
    auto window = init_result.value();
    glEnable(GL_DEPTH_TEST);

    auto model_shader = shader_program(
        "../shaders/scene_vertex.glsl",
        "../shaders/scene_fragment.glsl"
    );
    std::array<model, 3> models = {
        model("assets/models/lemur", "lemur.obj"),
        model("assets/models/cat", "12221_Cat_v1_l3.obj"),
        model("assets/models/astronaut", "Astronaut.obj")
    };
    std::array<std::tuple<GLuint, GLuint, GLuint, shader_program>, 4> skyboxes = {
        skybox::create_skybox("assets/skyboxes/water", "jpg"),
        skybox::create_skybox("assets/skyboxes/debug", "jpg"),
        skybox::create_skybox("assets/skyboxes/forest1", "jpg"),
        skybox::create_skybox("assets/skyboxes/forest2", "jpg"),
    };

    auto delta_time = 0.0f;
    auto last_frame = 0.0f;
    auto refraction_ratio = 1.5f;
    auto texture_balance = 0.5f;
    auto scale = 0.03f;

    auto rotation = glm::vec2(0.0f);
    int model_index = 0;
    int skybox_index = 0;
    bool rotation_enabled = true;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        float current_time = glfwGetTime();
        delta_time = current_time - last_frame;
        last_frame = current_time;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            model_shader = shader_program("../shaders/scene_vertex.glsl", "../shaders/scene_fragment.glsl");
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            break;
        }
        process_input(window, delta_time);

        auto [display_width, display_height] = get_window_size(window);
        glViewport(0, 0, display_width, display_height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetWindowSize(ImVec2(200, 100));
        ImGui::Begin("Props");
        ImGui::SliderFloat("Refraction Ratio", &refraction_ratio, 1.00001, 2.0f);
        ImGui::SliderFloat("Texture/Effects Balance", &texture_balance, 0, 1);
        ImGui::SliderFloat("Scale", &scale, 0.01, 2);
        ImGui::Combo("Model", &model_index, "lemur\0cat\0astronaut\0\0");
        ImGui::Combo("Skybox", &skybox_index, "water\0debug\0forest1\0forest2\0\0");
        ImGui::Checkbox("Rotation", &rotation_enabled);
        ImGui::End();
        if (!ImGui::IsAnyWindowFocused()) {
            auto delta = ImGui::GetMouseDragDelta(0, 0);
            ImGui::ResetMouseDragDelta();
            rotation += glm::vec2(delta.x / display_width * 2, delta.y / display_height * 2) * 30.0f;
        }

        auto actual_model = models[model_index];
        auto& [skybox_vao, skybox_vbo, skybox_texture, skybox_shader] = skyboxes[skybox_index];

        model_shader.use();
        auto model = glm::mat4(1.0f);
        model = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        auto center_shift = (actual_model.min_values + actual_model.max_values) / 2.0f;
        model = glm::translate(model, -center_shift);
        auto view = scene_camera.get_view_matrix();
        view = glm::rotate(view, glm::radians(rotation.y), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(rotation.x), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(
            glm::radians(scene_camera.zoom),
            (float)display_width / (float)display_height,
            0.1f,
            100.0f
        );
        auto transform = glm::mat4(1.0f);
        if (rotation_enabled) {
            transform = glm::rotate(
                transform,
                glm::radians(current_time * 25),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        }

        model_shader.set_uniform("model", model * transform);
        model_shader.set_uniform("view", view);
        model_shader.set_uniform("projection", projection);
        model_shader.set_uniform("camera_position", scene_camera.position);
        model_shader.set_uniform("skybox_texture", 0);
        model_shader.set_uniform("refraction_ratio", refraction_ratio);
        model_shader.set_uniform("texture_balance", texture_balance);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        actual_model.draw(model_shader);

        skybox::draw(
            skybox_vao,
            skybox_texture,
            skybox_shader,
            glm::mat4(glm::mat3(view)),
            projection
        );

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    dispose_ui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

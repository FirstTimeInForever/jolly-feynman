#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fmt/format.h>
#include <optional>
#include <array>

#include "shader_program.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "skybox.hpp"
#include "ui.hpp"
#include "heightmap.hpp"
#include "water.hpp"
#include "utility.hpp"
#include "light.hpp"
#include "shadows.hpp"
#include "simple_cube.hpp"


const unsigned int initial_width = 1080;
const unsigned int initial_height = 920;
float lastx = (float)initial_width / 2.0;
float lasty = (float)initial_height / 2.0;
bool first_mouse = true;
camera scene_camera(glm::vec3(0.0f, 0.3f, 1.0f));

float current_time = 0;
float delta_time = 0.0f;

auto boat_position = glm::vec3(0.0f, 0.0f, 0.0f);
size_t boat_position_index = 0;
float boat_time = 0.0f;
float boat_self_rotation = glm::radians(-180.0f);

glm::mat4 light_space_matrix;
GLuint shadow_map = 0;
GLuint lighthouse_projection_texture = 0;

const inline float near_plane = 0.01f;
const inline float far_plane = 1000.0f;

const auto lighthouse_light_position = glm::vec3(-17.920118, 13.820092, -18.100204);
auto lighthouse_light_space_matrix = glm::mat4(1.0f);

std::optional<GLFWwindow*> init(int argc, char** argv) {
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
    if (argc < 2) {
        fmt::print("Setting GLFW_OPENGL_FORWARD_COMPAT=GL_TRUE\n");
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }
    auto window = glfwCreateWindow(initial_width, initial_height, "Scene", nullptr, nullptr);
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
    //init_ui(window);
    return std::make_optional(window);
}

void process_input(GLFWwindow* window, float delta_time) {
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        scene_camera.speed *= 1.2;
        fmt::print("Camera speed set to: {}\n", scene_camera.speed);
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
        scene_camera.speed /= 1.2;
        fmt::print("Camera speed set to: {}\n", scene_camera.speed);
    }
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
    //const float inc = 0.02f;
    //if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    //    lighthouse_light_position += glm::vec3(inc, 0, 0);
    //}
    //if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    //    lighthouse_light_position -= glm::vec3(inc, 0, 0);
    //}
    //if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    //    lighthouse_light_position -= glm::vec3(0, 0, inc);
    //}
    //if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    //    lighthouse_light_position += glm::vec3(0, 0, inc);
    //}
    //if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    //    lighthouse_light_position += glm::vec3(0, inc, 0);
    //}
    //if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    //    lighthouse_light_position -= glm::vec3(0, inc, 0);
    //}
    //fmt::print("glm::vec3({}, {}, {})\n", lighthouse_light_position.x, lighthouse_light_position.y, lighthouse_light_position.z);
}

void set_light_uniforms(shader_program& shader) {
    global_light::set_uniforms(shader, "global_light");
    shader.set_uniform("shadow_map", 6);
}

const auto default_clipping_plane = glm::vec4(0, -1, 0, 10000);

void set_mvpc_uniforms(shader_program& shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection, glm::vec4 clipping_plane = default_clipping_plane) {
    shader.set_uniform("model", model);
    shader.set_uniform("view", view);
    shader.set_uniform("projection", projection);
    shader.set_uniform("clipping_plane", clipping_plane);
    shader.set_uniform("time", current_time);
    shader.set_uniform("lightSpaceMatrix", light_space_matrix);
    shader.set_uniform("camera_position", scene_camera.position);
    shader.set_uniform("lighthouse_light_space_matrix", lighthouse_light_space_matrix);
    shader.set_uniform("lighthouse_light_position", lighthouse_light_position);
    shader.set_uniform("lighthouse_light_target_point", boat_position);
    shader.set_uniform("lighthouse_projection_texture", 7);
}

inline void apply_center_shift(glm::mat4& model_matrix, mesh& target) {
    auto center_shift = (target.min_values + target.max_values) / 2.0f;
    model_matrix = glm::translate(model_matrix, -center_shift);
}

inline void apply_center_shift(glm::mat4& model_matrix, model& target) {
    auto center_shift = (target.min_values + target.max_values) / 2.0f;
    model_matrix = glm::translate(model_matrix, -center_shift);
}

void render_terrain(glm::mat4 view, glm::mat4 projection, glm::vec4 clipping_plane, shader_program& shader, mesh& terrain_mesh) {
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, shadow_map);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, lighthouse_projection_texture);
    shader.use();
    auto model = glm::mat4(1.0f);
    apply_center_shift(model, terrain_mesh);
    set_mvpc_uniforms(shader, model, view, projection, clipping_plane);
    set_light_uniforms(shader);
    shader.set_uniform("camera_position", scene_camera.position);
    shader.set_uniform("camera_front", scene_camera.front);
    terrain_mesh.draw(shader);
}

void render_trees(glm::mat4 view, glm::mat4 projection, glm::vec4 clipping_plane, shader_program& shader, model& tree_model) {
    shader.use();
    auto model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.01f));
    apply_center_shift(model, tree_model);
    std::vector<glm::vec3> translate_vectors = {
        glm::vec3(-5.9100404, 4.840016, -6.600056),
        glm::vec3(-6.1000447, 2.8399978, -1.9399986),
        glm::vec3(-1.1, 2.7499986, -23.34993),
        glm::vec3(5.850004, 2.7499986, -27.849861),
        glm::vec3(-12.65003, 2.7499986, -17.50002),
        glm::vec3(-19.049995, 7.2000093, -21.449959)
    };
    for (auto translate_vector: translate_vectors) {
        auto cmodel = glm::translate(model, translate_vector);
        set_mvpc_uniforms(shader, cmodel, view, projection, clipping_plane);
        set_light_uniforms(shader);
        tree_model.draw(shader);
    }
}

void render_water(glm::mat4 view, glm::mat4 projection, water_framebuffers_controller& controller, shader_program& shader, mesh& water_mesh) {
    controller.bind_textures();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.use();
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-0.5f, 0.0f, -0.5f));
    model = glm::scale(model, glm::vec3(1.0f));
    set_mvpc_uniforms(shader, model, view, projection);
    set_light_uniforms(shader);
    shader.set_uniform("camera_position", scene_camera.position);
    shader.set_uniform("texture_diffuse_reflection", 0);
    shader.set_uniform("texture_diffuse_refraction", 1);
    shader.set_uniform("texture_dudv", 2);
    shader.set_uniform("texture_normal", 3);
    shader.set_uniform("texture_refraction_depth", 4);
    set_light_uniforms(shader);
    water_mesh.draw(shader, true);

    glDisable(GL_BLEND);
}

void render_boat(glm::mat4 view, glm::mat4 projection, glm::vec4 clipping_plane, shader_program& shader, model& boat_model) {
    shader.use();
    auto model = glm::mat4(1.0f);
    std::vector<glm::vec3> path_points = {
        glm::vec3(0.0, 0.0, 0.0),
        glm::vec3(0.0, 0.0, -814.0),
        glm::vec3(300.0, 0.0, -814.0),
        glm::vec3(300.0, 0.0, 0.0)
    };
    model = glm::scale(model, glm::vec3(0.0002));
    if (glm::distance(path_points[boat_position_index], boat_position) < 0.01f) {
        boat_position_index = (boat_position_index + 1) % path_points.size();
        boat_self_rotation += glm::radians(90.0f);
    }
    auto current_target_point = path_points[boat_position_index];
    boat_time += delta_time;
    if (boat_time > 0.01f) {
        auto direction = glm::normalize(boat_position - current_target_point);
        boat_position -= direction * 1.0f;
        boat_time = 0;
    }
    model = glm::translate(model, boat_position);
    model = glm::rotate(model, -boat_self_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
    set_mvpc_uniforms(shader, model, view, projection, clipping_plane);
    set_light_uniforms(shader);
    boat_model.draw(shader);
}

void render_lighthouse(glm::mat4 view, glm::mat4 projection, glm::vec4 clipping_plane, shader_program& shader, model& lighthouse_model) {
    shader.use();
    auto model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.01f));
    model = glm::translate(model, glm::vec3(-17.950012, 4.6499996, -18.05001));
    auto transform = glm::mat4(1.0f);
    set_mvpc_uniforms(shader, model * transform, view, projection, clipping_plane);
    set_light_uniforms(shader);
    lighthouse_model.draw(shader);
}

int main(int argc, char** argv) {
    auto init_result = init(argc, argv);
    if (!init_result.has_value()) {
        return 1;
    }
    auto window = init_result.value();
    glEnable(GL_DEPTH_TEST);

    auto terrain_shader = load_shader("../shaders/terrain");
    auto terrain_mesh = create_terrain("assets/heightmap/heightmap.png");
    auto water_shader = load_shader("../shaders/water");
    auto water_mesh = create_water();
    auto common_object_shader = load_shader("../shaders/common_object");
    auto simple_object_shader = load_shader("../shaders/simple_object");
    auto tree_model = model("assets/models/tree", "lowpoyltree.obj");
    auto boat_model = model("assets/models/boat", "boat.obj");
    auto lighthouse_model = model("assets/models/lighthouse", "lighthouse.obj");

    auto [simple_cube_vao, simple_cube_vbo] = simple_cube::create();

    auto depth_shader = load_shader("../shaders/depth");

    water_framebuffers_controller water_framebuffers(window);
    shadow_framebuffer_controller shadow_framebuffer;
    shadow_map = shadow_framebuffer.depth_map;
    lighthouse_projection_texture = details::load_texture("assets/batman.png");

    auto last_frame = 0.0f;

    auto reflection_clipping_plane = glm::vec4(0, 1, 0, 0);
    auto refraction_clipping_plane = glm::vec4(0, -1, 0, 0);

    auto rotation = glm::vec2(0.0f);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        current_time = glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            current_time = last_frame;
        }
        delta_time = current_time - last_frame;
        last_frame = current_time;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            terrain_shader = load_shader("../shaders/terrain");
            common_object_shader = load_shader("../shaders/common_object");
            water_shader = load_shader("../shaders/water");
        }
        process_input(window, delta_time);

        auto [display_width, display_height] = utility::get_window_size(window);
        glViewport(0, 0, display_width, display_height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CLIP_DISTANCE0);
        glEnable(GL_DEPTH_TEST);

        auto view = scene_camera.get_view_matrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(scene_camera.zoom),
            (float)display_width / (float)display_height,
            near_plane,
            far_plane
        );

        const auto render_stuff = [&](glm::vec4 clipping_plane, glm::mat4 view = scene_camera.get_view_matrix()) {
            render_terrain(view, projection, clipping_plane, terrain_shader, terrain_mesh);
            render_trees(view, projection, clipping_plane, common_object_shader, tree_model);
            render_boat(view, projection, clipping_plane, common_object_shader, boat_model);
            render_lighthouse(view, projection, clipping_plane, common_object_shader, lighthouse_model);
        };

        const auto render_stuff_with_shader = [&](shader_program& shader, glm::vec4 clipping_plane, glm::mat4 view = scene_camera.get_view_matrix()) {
            render_terrain(view, projection, clipping_plane, shader, terrain_mesh);
            render_trees(view, projection, clipping_plane, shader, tree_model);
            render_boat(view, projection, clipping_plane, shader, boat_model);
            render_lighthouse(view, projection, clipping_plane, shader, lighthouse_model);
        };

        {
            const float vl = 10;
            glm::mat4 lightProjection = glm::ortho(-vl, vl, -vl, vl, 0.1f, far_plane);
            glm::mat4 lightView = glm::lookAt(lighthouse_light_position, boat_position * 0.0002f, glm::vec3(0.0, 1.0, 0.0));
            lighthouse_light_space_matrix = lightProjection * lightView;
        }

        {
            water_framebuffers.bind_reflection_frame_buffer();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            auto distance = 2 * scene_camera.position.y;
            scene_camera.position.y -= distance;
            scene_camera.pitch = -scene_camera.pitch;
            scene_camera.update();
            auto corrected_view = scene_camera.get_view_matrix();
            render_stuff(reflection_clipping_plane, corrected_view);
            scene_camera.position.y += distance;
            scene_camera.pitch = -scene_camera.pitch;
            scene_camera.update();
            water_framebuffers_controller::unbind_current_framebuffer(display_width, display_height);
        }
        water_framebuffers.bind_refraction_frame_buffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_stuff(refraction_clipping_plane);
        water_framebuffers_controller::unbind_current_framebuffer(display_width, display_height);

        const auto configure_matrices = [&]() {
            const float vl = 1;
            glm::mat4 lightProjection = glm::ortho(-vl, vl, -vl, vl, near_plane, far_plane);
            glm::mat4 lightView = glm::lookAt(global_light::position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            light_space_matrix = lightProjection * lightView;
        };

        glDisable(GL_CLIP_DISTANCE0);

        shadow_framebuffer.bind_framebuffer();
        glViewport(0, 0, shadow_framebuffer.width, shadow_framebuffer.height);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        configure_matrices();
        render_stuff_with_shader(depth_shader, default_clipping_plane);
        glCullFace(GL_BACK);
        shadow_framebuffer.unbind_framebuffer(display_width, display_height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        configure_matrices();

        render_stuff(default_clipping_plane);

        simple_cube::render(
            simple_cube_vao,
            simple_object_shader,
            glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.015f)), global_light::position),
            view,
            projection
        );

        //simple_cube::render(
        //    simple_cube_vao,
        //    simple_object_shader,
        //    glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)), lighthouse_light_position),
        //    view,
        //    projection
        //);

        render_water(view, projection, water_framebuffers, water_shader, water_mesh);

        glfwSwapBuffers(window);
    }
    water_framebuffers.dispose();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

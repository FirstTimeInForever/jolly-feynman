#ifndef LIGHT_HPP
#define LIGHT_HPP

#pragma once

#include <string>
#include <glm/glm.hpp>
#include "shader_program.hpp"

//struct ambient_light_source {
//    ambient_light_source(const glm::vec3& color, float strength) : color(color), strength(strength) {}
//
//    glm::vec3 color;
//    float strength;
//
//    void set_uniforms(shader_program& shader, const std::string& name) const {
//        shader.set_uniform(name + ".color", color);
//        shader.set_uniform(name + ".strength", strength);
//    }
//};
//
//struct directional_light_source {
//    directional_light_source(const glm::vec3& direction, const glm::vec3& color, float strength)
//        : direction(direction), color(color), strength(strength) {}
//
//    glm::vec3 direction;
//    glm::vec3 color;
//    float strength;
//
//    void set_uniforms(shader_program& shader, const std::string& name) const {
//        shader.set_uniform(name + ".direction", direction);
//        shader.set_uniform(name + ".color", color);
//        shader.set_uniform(name + ".strength", strength);
//    }
//};
//
//struct point_light_source {
//    point_light_source(const glm::vec3& position, const glm::vec3& color, float strength,
//                       float constant, float linear, float quadratic)
//        : position(position), color(color), strength(strength), constant(constant),
//          linear(linear), quadratic(quadratic) {}
//
//    glm::vec3 position;
//    glm::vec3 color;
//    float strength;
//    float constant;
//    float linear;
//    float quadratic;
//
//    void set_uniforms(shader_program& shader, const std::string& name) const {
//        shader.set_uniform(name + ".position", position);
//        shader.set_uniform(name + ".color", color);
//        shader.set_uniform(name + ".strength", strength);
//        shader.set_uniform(name + ".constant", constant);
//        shader.set_uniform(name + ".linear", linear);
//        shader.set_uniform(name + ".quadratic", quadratic);
//    }
//};
//
//
//
////inline const auto global_ambient_light_source = ambient_light_source(
////    glm::vec3(1.0f, 1.0f, 1.0f),
////    0.4f
////);
////
////inline const auto global_directional_light_source = directional_light_source(
////    glm::vec3(1.0f, 0.0f, 1.0f),
////    glm::vec3(1.0f, 1.0f, 1.0f),
////    1.0f
////);
//
////inline const auto sunlight_source = point_light_source(
////    glm::vec3(1.0f, 1.0f, 1.0f),
////    glm::vec3(1.0f, 1.0f, 0.1f),
////    1.0f,
////    1.0f,
////    0.09f,
////    0.032f
////);

namespace global_light {
    //inline auto position = glm::vec3(0.0f, 2.0f, 0.0f);
    inline auto position = glm::vec3(26.64988, 23.800043, 6.5000067);
    const inline auto color = glm::vec3(1.0f, 1.0f, 1.0f);
    const inline auto ambient_strength = 0.4f;
    const inline auto directional_strength = 1.0f;

    void set_uniforms(shader_program& shader, const std::string& name) {
        shader.set_uniform(name + "_position", position);
        shader.set_uniform(name + "_color", color);
        shader.set_uniform(name + "_ambient_strength", ambient_strength);
        shader.set_uniform(name + "_directional_strength", directional_strength);
    }
}


#endif

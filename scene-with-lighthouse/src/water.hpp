#ifndef WATER_HPP
#define WATER_HPP

#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "mesh.hpp"
#include "utility.hpp"

mesh create_water() {
    return mesh({
        vertex(glm::vec3(0, 0, 0), glm::vec3(0), glm::vec2(0, 0)),
        vertex(glm::vec3(0, 0, 1), glm::vec3(0), glm::vec2(0, 1)),
        vertex(glm::vec3(1, 0, 1), glm::vec3(0), glm::vec2(1, 1)),
        vertex(glm::vec3(1, 0, 0), glm::vec3(0), glm::vec2(1, 0))
    }, {0, 1, 2, 0, 3, 2});
}

class water_framebuffers_controller {
public:

    static const int reflection_width = 1080 * 4;
    static const int reflection_height = 920 * 4;

    static const int refraction_width = 1080 * 4;
    static const int refraction_height = 920 * 4;

    GLuint reflection_framebuffer = 0;
    GLuint reflection_texture = 0;
    GLuint reflection_depth_buffer = 0;
    GLuint refraction_framebuffer = 0;
    GLuint refraction_texture = 0;
    GLuint refraction_depth_texture = 0;

    GLuint dudv_texture = 0;
    GLuint normal_map_texture = 0;

    explicit water_framebuffers_controller(const GLFWwindow* window) {
        auto [width, height] = utility::get_window_size(window);
        reflection_framebuffer = create_framebuffer();
        reflection_texture = create_texture_attachment(reflection_width, reflection_height);
        reflection_depth_buffer = create_depth_buffer_attachment(reflection_width, reflection_height);
        unbind_current_framebuffer(width, height);
        refraction_framebuffer = create_framebuffer();
        refraction_texture = create_texture_attachment(refraction_width, refraction_height);
        refraction_depth_texture = create_depth_texture_attachment(refraction_width, refraction_height);
        unbind_current_framebuffer(width, height);
        dudv_texture = details::load_texture("assets/water/dudv.png");
        normal_map_texture = details::load_texture("assets/water/normal_map.png");
    }

    //void init_reflection_framebuffer(int width, int height) {
    //    reflection_framebuffer = create_framebuffer();
    //    reflection_texture = create_texture_attachment(reflection_width, reflection_height);
    //    reflection_depth_buffer = create_depth_buffer_attachment(reflection_width, reflection_height);
    //    unbind_current_framebuffer(width, height);
    //}
    //
    //void init_refraction_framebuffer(int width, int height) {
    //    refraction_framebuffer = create_framebuffer();
    //    refraction_texture = create_texture_attachment(refraction_width, refraction_height);
    //    refraction_depth_texture = create_depth_texture_attachment(refraction_width, refraction_height);
    //    unbind_current_framebuffer(width, height);
    //}

    void bind_reflection_frame_buffer() {
        bind_framebuffer(reflection_framebuffer, reflection_width, reflection_height);
    }

    void bind_refraction_frame_buffer() {
        bind_framebuffer(refraction_framebuffer, refraction_width, refraction_height);
    }

    static void unbind_current_framebuffer(int width, int height) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }

    static GLuint create_framebuffer() {
        GLuint framebuffer = 0;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        return framebuffer;
    }

    static void bind_framebuffer(GLuint framebuffer, int width, int height) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, width, height);
    }

    static GLuint create_texture_attachment(int width, int height) {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        return texture;
    }

    static GLuint create_depth_texture_attachment(int width, int height) {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
        return texture;
    }

    static GLuint create_depth_buffer_attachment(int width, int height) {
        GLuint depth_buffer = 0;
        glGenRenderbuffers(1, &depth_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
        return depth_buffer;
    }

    void bind_textures() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, reflection_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, refraction_texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, dudv_texture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, normal_map_texture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, refraction_depth_texture);
        glActiveTexture(GL_TEXTURE0);
    }

    void dispose() {
        glDeleteFramebuffers(1, &reflection_framebuffer);
        glDeleteTextures(1, &reflection_texture);
        glDeleteRenderbuffers(1, &reflection_depth_buffer);
        glDeleteFramebuffers(1, &refraction_framebuffer);
        glDeleteTextures(1, &refraction_texture);
        glDeleteTextures(1, &refraction_depth_texture);
    }
};

#endif
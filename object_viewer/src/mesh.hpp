#ifndef MESH_HPP
#define MESH_HPP

#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

struct vertex {
    vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texcoords):
        position(position), normal(normal), texcoords(texcoords) {}

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;
};

struct texture {
    texture(GLuint id, std::string type): id(id), type(std::move(type)) {}

    GLuint id;
    std::string type;
};

class mesh {
public:

    mesh(std::vector<vertex> vertices, std::vector<GLuint> indices, std::vector<texture> textures):
        vertices(vertices), indices(indices), textures(textures) {
        setup_mesh();
    }

    void draw(shader_program& shader) {
        int diffuse_textures = 1;
        int specular_textures = 1;
        int normal_textures = 1;
        for (size_t index = 0; index < textures.size(); index += 1) {
            glActiveTexture(GL_TEXTURE1 + index);
            std::string number;
            auto name = textures[index].type;
            if(name == "texture_diffuse") {
                number = std::to_string(diffuse_textures++);
            } else if (name == "texture_normal") {
                number = std::to_string(normal_textures++);
            }
            shader.set_uniform(name + number, (int)(index + 1));
            glBindTexture(GL_TEXTURE_2D, textures[index].id);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    std::vector<vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<texture> textures;

private:

    void setup_mesh() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, texcoords));

        glBindVertexArray(0);
    }

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

#endif

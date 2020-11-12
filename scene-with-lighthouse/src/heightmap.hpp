#ifndef HEIGHTMAP_HPP
#define HEIGHTMAP_HPP

#pragma once

#include <vector>
#include <tuple>
#include <string>
#include <glm/glm.hpp>
#include <fmt/core.h>
#include <cmath>

#include "stb_image_wrapper.hpp"
#include "mesh.hpp"
#include "model.hpp"

namespace {
    inline auto load_raw_image(const std::string& path) {
        fmt::print("Loading heightmap from: {}", path);
        int width = 0;
        int height = 0;
        int components = 0;
        auto data = stbi_load(path.c_str(), &width, &height, &components, 0);
        if (!data) {
            fmt::print("Failed to load heightmap at path: {}\n", path);
            stbi_image_free(data);
            throw std::runtime_error(fmt::format("Failed to load heightmap at path: {}", path));
        }
        return std::make_tuple((std::byte*)data, width, height, components);
    }
}

class heightmap_loader {
public:

    explicit heightmap_loader(const std::string& path) {
        fmt::print("Loading heightmap from {}", path);
        data = (std::byte*)stbi_load(path.c_str(), &width, &height, &components, 0);
        if (!data) {
            fmt::print("Failed to load heightmap at path: {}\n", path);
            stbi_image_free(data);
            throw std::runtime_error(fmt::format("Failed to load heightmap at path: {}", path));
        }
    }

    std::byte* data = nullptr;
    int width = 0;
    int height = 0;
    int components = 0;
    float miny = 0;
    float maxy = 0.5;

    float startx = -0.5f;
    float startz = -0.5f;


    const float inct = 100.0f;

    inline float calc_height(int x, int z) const {
        auto red = (int)data[x * components + 0 + z * components * width];
        auto green = (int)data[x * components + 1 + z * components * width];
        auto blue = (int)data[x * components + 2 + z * components * width];
        int rgb = ((0xff & red) << 16) | ((0xff & green) << 8) | (0xff & blue);
        return miny + std::abs(maxy - miny) * ((float)rgb / (float)(255 * 255 * 255));
    }

    void calc_normals(std::vector<vertex>& vertices) const {
        for (int row = 0; row < height; row += 1) {
            for (int column = 0; column < width; column += 1) {
                auto normal = glm::vec3(0.0f);
                if (row > 0 && row < height -1 && column > 0 && column < width -1) {
                    auto v0 = vertices[row * width + column].position;
                    auto v1 = vertices[row * width + (column - 1)].position - v0;
                    auto v2 = vertices[(row + 1) * width + column].position - v0;
                    auto v3 = vertices[row * width + (column + 1)].position - v0;
                    auto v4 = vertices[(row - 1) * width + column].position - v0;
                    normal = glm::normalize(
                        glm::normalize(glm::cross(v1, v2)) +
                        glm::normalize(glm::cross(v2, v3)) +
                        glm::normalize(glm::cross(v3, v4)) +
                        glm::normalize(glm::cross(v4, v1))
                    );
                } else {
                    normal = glm::vec3(0, 1, 0);
                }
                vertices[row * width + column].normal = glm::normalize(normal);
            }
        }
    }

    auto create_mesh() {
        const float incx = std::abs(startx * 2.0f) / width;
        const float incz = std::abs(startz * 2.0f) / height;
        std::vector<vertex> vertices;
        std::vector<GLuint> indices;
        for (int row = 0; row < height; row++) {
            for (int column = 0; column < width; column++) {
                //fmt::print("{}\n", calc_height(column, row));
                vertices.emplace_back(
                    glm::vec3(startx + column * incx, calc_height(column, row), startz + row * incz),
                    glm::vec3(0),
                    glm::vec2(inct * (float)column / (float)width, inct * (float)row / (float)width)
                );
                if (column < width - 1 && row < height - 1) {
                    int leftTop = row * width + column;
                    int leftBottom = (row + 1) * width + column;
                    int rightBottom = (row + 1) * width + column + 1;
                    int rightTop = row * width + column + 1;

                    indices.push_back(leftTop);
                    indices.push_back(leftBottom);
                    indices.push_back(rightTop);

                    indices.push_back(rightTop);
                    indices.push_back(leftBottom);
                    indices.push_back(rightBottom);
                }
            }
        }
        return std::make_tuple(vertices, indices);
    }
};

inline mesh create_terrain(const std::string& path) {
    heightmap_loader loader("assets/heightmap/heightmap.png");
    auto [vertices, indices] = loader.create_mesh();
    loader.calc_normals(vertices);
    return mesh(vertices, indices, {
        texture(details::load_texture("assets/heightmap/p1.jpg"), "texture_diffuse"),
        texture(details::load_texture("assets/heightmap/p2.jpg"), "texture_diffuse"),
        texture(details::load_texture("assets/heightmap/rock.jpg"), "texture_diffuse"),
        texture(details::load_texture("assets/heightmap/detail.jpg"), "texture_diffuse")
    });
}

#endif
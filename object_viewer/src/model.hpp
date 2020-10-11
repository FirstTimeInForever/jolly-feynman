#ifndef MODEL_HPP
#define MODEL_HPP

#pragma once

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "mesh.hpp"
#include "shader_program.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fmt/format.h>
#include "stb_image_wrapper.hpp"
#include <map>
#include <limits>

namespace details {
    GLuint load_texture(const std::string& path, bool gamma = true) {
        std::cout << path << std::endl;
        GLuint textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        auto data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (!data) {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
            return 0;
        }
        GLenum format;
        if (nrComponents == 1) {
            format = GL_RED;
        }
        else if (nrComponents == 3) {
            format = GL_RGB;
        }
        else if (nrComponents == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);

        return textureID;
    }
}

class model {
public:
    explicit model(const std::string& path, const std::string& filename): path(path) {
        Assimp::Importer importer;
        auto scene = importer.ReadFile(fmt::format("{}/{}", path, filename), aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error(importer.GetErrorString());
        }
        process_node(scene->mRootNode, scene);
    }

    void draw(shader_program& shader) {
        for (auto& mesh: meshes) {
            mesh.draw(shader);
        }
    }

    glm::vec3 min_values = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max_values = glm::vec3(std::numeric_limits<float>::min());

private:
    std::string path;
    std::vector<mesh> meshes;
    std::map<std::string, texture> texture_cache;

    void process_node(aiNode* node, const aiScene* scene) {
        for (size_t mesh_index = 0; mesh_index < node->mNumMeshes; mesh_index += 1) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[mesh_index]];
            meshes.push_back(process_mesh(mesh, scene));
        }
        for (size_t index = 0; index < node->mNumChildren; index += 1) {
            process_node(node->mChildren[index], scene);
        }
    }

    glm::vec2 get_texture_corrdinats_for_vertex(const aiMesh* mesh, size_t vertex_index) {
        if (mesh->mTextureCoords[0]) {
            return glm::vec2(mesh->mTextureCoords[0][vertex_index].x, mesh->mTextureCoords[0][vertex_index].y);
        }
        return glm::vec2(0, 0);
    }

    mesh process_mesh(const aiMesh* raw_mesh, const aiScene* raw_scene) {
        std::vector<vertex> vertices;
        for (size_t vertex_index = 0; vertex_index < raw_mesh->mNumVertices; vertex_index += 1) {
            auto vertex = glm::vec3(
                raw_mesh->mVertices[vertex_index].x,
                raw_mesh->mVertices[vertex_index].y,
                raw_mesh->mVertices[vertex_index].z
            );
            min_values = glm::min(min_values, vertex);
            max_values = glm::max(max_values, vertex);
            vertices.emplace_back(
                vertex,
                glm::vec3(raw_mesh->mNormals[vertex_index].x, raw_mesh->mNormals[vertex_index].y,
                          raw_mesh->mNormals[vertex_index].z),
                get_texture_corrdinats_for_vertex(raw_mesh, vertex_index)
            );
        }
        std::vector<GLuint> indices;
        for (size_t face_index = 0; face_index < raw_mesh->mNumFaces; face_index += 1) {
            auto face = raw_mesh->mFaces[face_index];
            for (size_t index = 0; index < face.mNumIndices; index += 1) {
                indices.push_back(face.mIndices[index]);
            }
        }
        std::vector<texture> textures;
        if (raw_mesh->mMaterialIndex >= 0) {
            auto material = raw_scene->mMaterials[raw_mesh->mMaterialIndex];
            auto diffuse = load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            auto normals = load_material_textures(material, aiTextureType_HEIGHT, "texture_normal");
            textures.insert(textures.end(), diffuse.begin(), diffuse.end());
            textures.insert(textures.end(), normals.begin(), normals.end());
        }
        return mesh(vertices, indices, textures);
    }

    std::vector<texture> load_material_textures(const aiMaterial* mat, aiTextureType type, std::string type_name) {
        std::vector<texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            auto texture_path = fmt::format("{}/{}", path, str.C_Str());
            auto cache_entry = texture_cache.find(texture_path);
            if (cache_entry != texture_cache.end()) {
                textures.emplace_back(cache_entry->second);
            } else {
                auto actual_texture = texture(details::load_texture(texture_path), type_name);
                textures.emplace_back(actual_texture);
                texture_cache.insert_or_assign(texture_path, actual_texture);
            }
        }
        return textures;
    }
};

#endif

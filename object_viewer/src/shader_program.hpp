#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

class shader_program {
public:
    shader_program(std::string  vertex_code_fname, std::string  fragment_code_fname);

    ~shader_program();

    void use();

    void reload();

    template<typename T>
    void set_uniform(const std::string& name, T val);

    template<typename T>
    void set_uniform(const std::string& name, T val1, T val2);

    template<typename T>
    void set_uniform(const std::string& name, T val1, T val2, T val3);

    template<typename T>
    void set_uniform(const std::string& name, T val1, T val2, T val3, T val4);

    template<>
    void set_uniform<int>(const std::string& name, int val) {
        glUniform1i(glGetUniformLocation(program_id, name.c_str()), val);
    }

    template<>
    void set_uniform<bool>(const std::string& name, bool val) {
        glUniform1i(glGetUniformLocation(program_id, name.c_str()), val);
    }

    template<>
    void set_uniform<float>(const std::string& name, float val) {
        glUniform1f(glGetUniformLocation(program_id, name.c_str()), val);
    }

    template<>
    void set_uniform<float>(const std::string& name, float val1, float val2) {
        glUniform2f(glGetUniformLocation(program_id, name.c_str()), val1, val2);
    }

    template<>
    void set_uniform<float>(const std::string& name, float val1, float val2, float val3) {
        glUniform3f(glGetUniformLocation(program_id, name.c_str()), val1, val2, val3);
    }

    template<>
    void set_uniform<float>(const std::string& name, float val1, float val2, float val3, float val4) {
        glUniform4f(glGetUniformLocation(program_id, name.c_str()), val1, val2, val3, val4);
    }

    template<>
    void set_uniform<float*>(const std::string& name, float* val) {
        glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, val);
    }

    void set_uniform(const std::string& name, glm::vec3 value) {
        glUniform3fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]);
    }

    void set_uniform(const std::string& name, glm::vec4 value) {
        glUniform4fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]);
    }

    void set_uniform(const std::string &name, const glm::mat2& mat) {
        glUniformMatrix2fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void set_uniform(const std::string &name, const glm::mat3& mat) {
        glUniformMatrix3fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void set_uniform(const std::string &name, const glm::mat4& mat) {
        glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:

    void check_compile_error();

    void check_linking_error();

    void compile(const std::string& vertex_code, const std::string& fragment_code);

    void link();

    GLuint vertex_id;
    GLuint fragment_id;
    GLuint program_id;
    std::string vertex_shader_path;
    std::string fragment_shader_path;
};

#endif

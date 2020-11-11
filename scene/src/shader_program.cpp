#include "shader_program.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>

namespace {
    std::string read_shader_code(const std::string& filename) {
        try {
            std::ifstream file(filename);
            file.exceptions(std::ifstream::badbit);
            return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        }
        catch (std::exception const& exc) {
            std::cerr << "Error reading shader file: " << exc.what() << std::endl;
        }
        std::terminate();
    }
}

shader_program::shader_program(std::string  vertex_shader_path, std::string  fragment_shader_path):
    vertex_shader_path(std::move(vertex_shader_path)), fragment_shader_path(std::move(fragment_shader_path)) {
    reload();
}

void shader_program::reload() {
    const auto vertex_code = read_shader_code(vertex_shader_path);
    const auto fragment_code = read_shader_code(fragment_shader_path);
    compile(vertex_code, fragment_code);
    link();
}

shader_program::~shader_program() {
}

void shader_program::compile(const std::string& vertex_code, const std::string& fragment_code)
{
    const char* vcode = vertex_code.c_str();
    vertex_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_id, 1, &vcode, NULL);
    glCompileShader(vertex_id);

    const char* fcode = fragment_code.c_str();
    fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_id, 1, &fcode, NULL);
    glCompileShader(fragment_id);
    check_compile_error();
}

void shader_program::link() {
    program_id = glCreateProgram();
    glAttachShader(program_id, vertex_id);
    glAttachShader(program_id, fragment_id);
    glLinkProgram(program_id);
    check_linking_error();
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);
}

void shader_program::use() {
    glUseProgram(program_id);
}

void shader_program::check_compile_error() {
    int success;
    char infoLog[1024];
    glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_id, 1024, NULL, infoLog);
        std::cerr << "Error compiling Vertex shader_t:\n" << infoLog << std::endl;
    }
    glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_id, 1024, NULL, infoLog);
        std::cerr << "Error compiling Fragment shader_t:\n" << infoLog << std::endl;
    }
}

void shader_program::check_linking_error() {
    int success;
    char infoLog[1024];
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program_id, 1024, NULL, infoLog);
        std::cerr << "Error Linking shader_t Program:\n" << infoLog << std::endl;
    }
}

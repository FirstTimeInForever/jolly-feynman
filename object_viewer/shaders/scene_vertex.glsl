#version 330 core
layout (location = 0) in vec3 ipos;
layout (location = 1) in vec3 inormal;
layout (location = 2) in vec2 itexcoord;

out vec2 texcoord;
out vec3 normal;
out vec3 position;
out vec3 uview;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    texcoord = itexcoord;
    normal = mat3(transpose(inverse(model))) * inormal;
    position = vec3(model * vec4(ipos, 1.0));
    gl_Position = projection * view * model * vec4(ipos, 1.0);
}

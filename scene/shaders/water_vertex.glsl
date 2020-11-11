#version 330 core
layout (location = 0) in vec3 ipos;
layout (location = 1) in vec3 inormal;
layout (location = 2) in vec2 itexcoord;

out vec2 texcoord;
out vec3 vertex_position;
out vec4 clipping_space;
out vec3 camera_world_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_position;

void main() {
    texcoord = ipos.xz;
    vertex_position = vec3(model * vec4(ipos, 1.0));
    camera_world_position = camera_position - vertex_position.xyz;
    clipping_space = projection * view * model * vec4(ipos, 1.0);
    gl_Position = clipping_space;
}

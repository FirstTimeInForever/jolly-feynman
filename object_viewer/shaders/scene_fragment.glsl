#version 330 core
out vec4 FragColor;

in vec2 texcoord;
in vec3 normal;
in vec3 position;
in vec3 uview;

uniform vec3 camera_position;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform samplerCube skybox_texture;
uniform float refraction_ratio;
uniform float texture_balance;

vec4 create_effects(float refraction_ratio) {
    vec3 nr = normalize(position - camera_position);
    vec4 reflection = vec4(texture(skybox_texture, reflect(nr, normalize(normal))).rgb, 1.0);
    vec4 refraction = vec4(texture(skybox_texture, refract(nr, normalize(normal), 1.0 / refraction_ratio)).rgb, 1.0);
    float a = 1 + dot(nr, normalize(normal));
    float frensel = pow(1 - a, 5) * (1.0 - 1.0 / refraction_ratio) + 1.0 / refraction_ratio;
    return mix(refraction, reflection, frensel);
}

void main() {
    vec4 actual_texture = texture(texture_diffuse1, texcoord);
    FragColor = mix(actual_texture, create_effects(refraction_ratio), texture_balance);
}
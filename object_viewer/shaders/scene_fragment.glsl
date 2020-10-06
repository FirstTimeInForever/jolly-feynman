#version 330 core
out vec4 FragColor;

in vec2 texcoord;
in vec3 normal;
in vec3 position;

uniform vec3 camera_position;
uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_normal1;
uniform samplerCube skybox_texture;
uniform float refraction_ratio;
uniform float effects_balance;
uniform float texture_balance;

vec4 create_effects(float refraction_ratio, float balance) {
    vec3 nr = normalize(position - camera_position);
    vec4 reflection = vec4(texture(skybox_texture, reflect(nr, normalize(normal))).rgb, 1);
    vec4 refraction = vec4(texture(skybox_texture, refract(nr, normalize(-normal), refraction_ratio)).rgb, 1);
    return mix(reflection, refraction, balance);
}

void main() {
    vec4 actual_texture = texture(texture_diffuse1, texcoord);
    FragColor = mix(actual_texture, create_effects(refraction_ratio, effects_balance), texture_balance);
}
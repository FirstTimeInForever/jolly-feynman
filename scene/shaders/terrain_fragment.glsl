#version 330 core
out vec4 FragColor;

in vec2 texcoord;
in vec3 normal;
in vec3 vertex_position;
in vec4 FragPosLightSpace;
in vec3 camera_world_position;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_diffuse4;
uniform sampler2D texture_diffuse5;

uniform float time;

const float negative_terrain_treshold = 0;
const float positive_terrain_treshold = 0.03;

vec3 calculate_terrain_texture() {
    vec3 height_based_texture = texture(texture_diffuse1, texcoord).rgb;
    if (vertex_position.y < negative_terrain_treshold) {
        float diff = abs(vertex_position.y - negative_terrain_treshold) * 20;
        height_based_texture = mix(height_based_texture, texture(texture_diffuse2, texcoord).rgb, diff);
    } else if (vertex_position.y > positive_terrain_treshold) {
        float diff = abs(vertex_position.y - positive_terrain_treshold) * 20;
        height_based_texture = mix(height_based_texture, texture(texture_diffuse3, texcoord).rgb, diff);
    }
    float depth = clamp(gl_FragCoord.z / gl_FragCoord.w, 0, 0.5);
    vec3 detailed_texture = mix(texture(texture_diffuse4, texcoord * 4).rgb, vec3(1), depth);
    return height_based_texture * detailed_texture;
}

uniform vec3 global_light_position;
uniform vec3 global_light_color;
uniform float global_light_ambient_strength;
uniform float global_light_directional_strength;

//vec3 calculate_light() {
//    vec3 ambient = ambient_light_strength * light_color;
//
//    float inc_angle = 10.0f;
//    float radius = 1000.0f;
//    float x = cos(inc_angle) * radius;
//    float z = sin(inc_angle) * radius;
//    vec3 _lighthouse_light_target_point = vec3(z * cos(time) - x * sin(time), 0.0, x * cos(time) + z * sin(time));
//    vec3 _lighthouse_light_position = lighthouse_light_position * 0.05;
//    vec3 light_direction = normalize(_lighthouse_light_position - _lighthouse_light_target_point / 800);
//    vec3 dn = normalize(position - _lighthouse_light_position);
//    float angle = dot(dn, -light_direction);
//    if (angle < 0.996) {
//        return vec3(0.2f);
//    }
//    vec3 diffuse = angle * light_color;
////    return ambient + diffuse;
//    return ambient;
//}


uniform sampler2D shadow_map;
uniform vec3 camera_position;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadow_map, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 normal = normalize(normal);
    vec3 lightDir = normalize(global_light_position - vertex_position);
    float bias = 0.00001;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / (textureSize(shadow_map, 0));
    for(int x = -1; x <= 1; x += 1) {
        for(int y = -1; y <= 1; y += 1) {
            float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 10.0;
    if(projCoords.z > 1.0) {
        shadow = 0.0;
    }
    return shadow;
}

vec4 calc_global_light() {
    vec3 object_color = calculate_terrain_texture();
    vec3 _normal = normalize(normal);
    // ambient
    vec3 ambient = global_light_ambient_strength * object_color;
    // diffuse
    vec3 light_direction = normalize(global_light_position - vertex_position);
    float diff = max(dot(light_direction, _normal), 0.0);
    vec3 diffuse = diff * global_light_color;
    // specular
    vec3 view_direction = normalize(camera_position - vertex_position);
    float spec = 0.0;
    vec3 halfway_direction = normalize(light_direction + view_direction);
    spec = pow(max(dot(_normal, halfway_direction), 0.0), 86.0);
    vec3 specular = spec * global_light_color;
    // calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 result = (ambient + (1.0 - shadow) * (diffuse)) * object_color;
//    vec3 result = (ambient + diffuse) * object_color;
    return vec4(result, 1.0);
}


void main() {
//    vec3 light = calculate_light(normal).xyz;
    vec3 light = calc_global_light().xyz;

    FragColor = vec4(light, 1.0);
}
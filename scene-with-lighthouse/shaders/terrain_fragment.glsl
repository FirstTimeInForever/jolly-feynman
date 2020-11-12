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

uniform vec3 lighthouse_light_position;
uniform vec3 lighthouse_light_target_point;
uniform mat4 lighthouse_light_space_matrix;
uniform sampler2D lighthouse_projection_texture;

vec4 calc_lighthouse_light() {
    vec4 fcords = lighthouse_light_space_matrix * vec4(vertex_position, 1.0);
    vec3 projCoords = fcords.xyz / fcords.w;
    if(vertex_position.y < 0) {
        return vec4(0);
    }
    projCoords = projCoords * 0.5 + 0.5;
    vec2 tc = ((2 * projCoords - 1) / (2 * 0.004)).xy + 0.5;
    if (tc.x > 1 || tc.x < 0 || tc.y < 0 || tc.y > 1) {
        return vec4(0);
    }
    return texture(lighthouse_projection_texture, tc);
}

void main() {
//    vec3 light = calculate_light(normal).xyz;
    vec3 light = calc_global_light().xyz;

    FragColor = vec4(light, 1.0) + calc_lighthouse_light();
}
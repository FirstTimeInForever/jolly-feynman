#version 330 core
out vec4 FragColor;

in vec2 texcoord;
in vec3 vertex_position;
in vec4 clipping_space;
in vec3 camera_world_position;
uniform float time;

uniform sampler2D texture_diffuse_reflection;
uniform sampler2D texture_diffuse_refraction;
uniform sampler2D texture_dudv;
uniform sampler2D texture_normal;
uniform sampler2D texture_refraction_depth;

const float distortion_tiling = 5.0;
const float distorting_strength = 0.008;
const float water_reflection_value = 1.3;

const float near = 0.001f;
const float far = 1000.0f;

uniform vec3 camera_position;

uniform vec3 global_light_position;
uniform vec3 global_light_color;
uniform float global_light_ambient_strength;
uniform float global_light_directional_strength;

vec4 calc_global_light(vec3 normal, vec3 object_color) {
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
//    float shadow = ShadowCalculation(FragPosLightSpace);
    //    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * object_color;
    vec3 result = (ambient + (diffuse + specular)) * object_color;
    return vec4(result, 1.0);
}

float calculate_reflective_factor(vec3 normal) {
    vec3 view_vector = normalize(camera_world_position);
    float reflective_factor = dot(view_vector, normal);
    reflective_factor = pow(reflective_factor, water_reflection_value);
    return reflective_factor;
}

float calculate_water_depth(vec2 reflection_tc, vec2 refraction_tc) {
    float depth = texture(texture_refraction_depth, refraction_tc).r;
    float floor_distance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
    depth = gl_FragCoord.z;
    float water_distance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
    float water_depth = 1 - (floor_distance - water_distance);
    return water_depth;
}

uniform vec3 lighthouse_light_position;
uniform vec3 lighthouse_light_target_point;
uniform mat4 lighthouse_light_space_matrix;
uniform sampler2D lighthouse_projection_texture;

vec4 calc_lighthouse_light() {
    vec4 fcords = lighthouse_light_space_matrix * vec4(vertex_position, 1.0);
    vec3 projCoords = fcords.xyz / fcords.w;
    projCoords = projCoords * 0.5 + 0.5;
    vec2 tc = ((2 * projCoords - 1) / (2 * 0.004)).xy + 0.5;
    if (tc.x > 1 || tc.x < 0 || tc.y < 0 || tc.y > 1) {
        return vec4(0);
    }
    return texture(lighthouse_projection_texture, tc);
}

void main() {
    float corrected_time = time * 0.008;
    vec2 distortion_first_coords = (texcoord + vec2(sin(corrected_time), cos(corrected_time))) * distortion_tiling;
    vec2 distortion_second_coords = (vec2(-texcoord.x, texcoord.y) + vec2(sin(corrected_time), cos(corrected_time))) * distortion_tiling;
    vec2 first_distortion = texture(texture_dudv, distortion_first_coords).xy * distorting_strength;
    vec2 second_distortion = texture(texture_dudv, distortion_second_coords).xy * distorting_strength;
    vec2 distortion = first_distortion + second_distortion;

    vec2 ndc = (clipping_space.xy / clipping_space.w) / 2.0 + 0.5;
    vec2 reflection_tc = vec2(ndc.x, -ndc.y) + distortion;
    reflection_tc.x = clamp(reflection_tc.x, 0.001, 0.999);
    reflection_tc.y = clamp(reflection_tc.y, -0.999, -0.001);
    vec2 refraction_tc = ndc.xy + distortion;
    refraction_tc = clamp(refraction_tc, 0.001, 0.999);
    vec4 reflection_color = texture(texture_diffuse_reflection, reflection_tc);
    vec4 refraction_color = texture(texture_diffuse_refraction, refraction_tc);

    vec4 normal_color = texture(texture_normal, distortion_first_coords);
    vec3 normal = normalize(vec3(normal_color.r * 2.0 - 1.0, normal_color.b, normal_color.g * 2.0 - 1.0));

    float depth_alpha = clamp(calculate_water_depth(reflection_tc, refraction_tc), 0, 1);

    vec4 base_color = vec4(0, 0.2, 0.5, 1.0) + calc_lighthouse_light();
    vec4 reflection_refraction_color = mix(reflection_color, refraction_color, calculate_reflective_factor(vec3(0.0, 1.0, 0.0)));
    vec4 object_color = mix(reflection_refraction_color, base_color, 0.1);
    FragColor = calc_global_light(normal, object_color.rgb) * 1.4;
    FragColor.a = depth_alpha;
}
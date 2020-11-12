#version 330 core
out vec4 FragColor;

in vec2 texcoord;
in vec3 normal;
in vec3 vertex_position;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadow_map;
uniform vec3 camera_position;

uniform vec3 global_light_position;
uniform vec3 global_light_color;
uniform float global_light_ambient_strength;
uniform float global_light_directional_strength;

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
    vec3 object_color = texture(texture_diffuse1, texcoord).rgb;
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
//    FragColor = vec4(calculate_light(normal).xyz * texture(texture_diffuse1, texcoord).rgb, 1);
    vec3 light = calc_global_light().rgb;
//    vec3 light = calc_light().xyz;
    FragColor = vec4(light, 1);
}
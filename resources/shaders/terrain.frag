/* vim: set filetype=glsl : */
#version 460 core

in vec2 fragTexCoord;

out vec4 finalColor;

uniform sampler2D texture0;
uniform float water_level;

const vec3 WATER_COLOR = vec3(0.1, 0.45, 0.6);
const vec3 SAND_COLOR = vec3(0.8, 0.7, 0.5);
const vec3 GRASS_COLOR = vec3(0.3, 0.6, 0.05);
const vec3 ROCK_COLOR = vec3(0.45, 0.4, 0.4);
const vec3 SNOW_COLOR = vec3(0.9, 0.9, 1.0);

void main() {
    float height = texture(texture0, fragTexCoord).r;
    vec3 color = vec3(1.0, 0.0, 0.0);

    if (height <= water_level) color = WATER_COLOR;
    else if (height <= 0.65) color = SAND_COLOR;
    else if (height <= 0.80) color = GRASS_COLOR;
    else if (height <= 0.95) color = ROCK_COLOR;
    else color = SNOW_COLOR;

    finalColor = vec4(color, 1.0);
}

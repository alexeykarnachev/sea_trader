/* vim: set filetype=glsl : */
#version 460 core

struct Camera {
    vec2 position;
    float view_width;
    float aspect;
};

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform Camera camera;

out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragPosition;
out vec2 fragScreenPosition;

void main() {
    float view_height = camera.view_width / camera.aspect;
    float x = (vertexPosition.x - camera.position.x) / (0.5 * camera.view_width);
    float y = (vertexPosition.y - camera.position.y) / (-0.5 * view_height);
    vec4 position = vec4(x, y, 0.0, 1.0);

    fragScreenPosition = (position.xy + 1.0) / 2.0;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragPosition = vertexPosition;
    gl_Position = position;
}


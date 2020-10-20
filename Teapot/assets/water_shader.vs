#version 330 core

layout (location = 0)
in vec3 in_position;
layout (location = 1)
in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 aPosition;
out vec2 aTexCoords;

void main()
{
    aTexCoords = texcoords;
    vec4 pos = vec4(in_position, 1.0);
    aPosition = in_position;
    gl_Position = projection * view * model * pos;
}
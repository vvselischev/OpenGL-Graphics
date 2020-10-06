#version 330 core

layout (location = 0)
in vec3 in_position;

uniform mat4 MVP;

out vec3 TexCoords;

void main()
{
    TexCoords = in_position;
    vec4 pos = vec4(in_position, 1.0);
    gl_Position = MVP * pos;
}

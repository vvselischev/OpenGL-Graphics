#version 330 core
layout (location = 0) in vec3 aPosition;

out vec4 position;

void main()
{
    gl_Position = vec4(aPosition, 1.0);
    position = gl_Position;
}
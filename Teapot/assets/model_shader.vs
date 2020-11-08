#version 330 core

layout (location = 0)
in vec3 in_position;
layout (location = 1)
in vec3 normal;
layout (location = 2)
in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float waterLevel;
uniform float waterNormal;

out vec3 aPosition;
out vec3 aNormal;
out vec2 aTexCoords;

void main()
{
    aNormal = normal;
    aTexCoords = texcoords;
    vec4 pos = vec4(in_position, 1.0);
    aPosition = in_position;
    vec4 modelPosition = model * pos;
    gl_Position = projection * view * model * pos;
    gl_ClipDistance[0] = waterNormal * (modelPosition.y + 0.01 - waterLevel);
}

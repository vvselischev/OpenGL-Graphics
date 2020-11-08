#version 330 core

layout (location = 0)
in vec3 in_position;
//layout (location = 1)
//in vec2 in_normal;
layout (location = 1)
in vec2 in_texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix1;
uniform mat4 lightSpaceMatrix2;
uniform mat4 lightSpaceMatrix3;

uniform float waterLevel;
uniform float waterNormal;

out vec3 aPosition;
out vec3 aNormal;
out vec2 aTexCoords;
out vec4 aLightPosition1;
out vec4 aLightPosition2;
out vec4 aLightPosition3;
out float z;

void main()
{
    aTexCoords = in_texcoords;
    //aNormal = in_normal;
    vec4 pos = vec4(in_position, 1.0);
    aPosition = in_position;
    vec4 modelPosition = model * pos;
    aLightPosition1 = lightSpaceMatrix1 * modelPosition;
    aLightPosition2 = lightSpaceMatrix2 * modelPosition;
    aLightPosition3 = lightSpaceMatrix3 * modelPosition;
    gl_Position = projection * view * modelPosition;
    z = gl_Position.z;
    gl_ClipDistance[0] = waterNormal * (modelPosition.y - waterLevel);
}
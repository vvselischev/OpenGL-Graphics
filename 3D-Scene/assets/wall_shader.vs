#version 330 core

layout (location = 0)
in vec3 in_position;
layout (location = 1)
in vec3 normal;
layout (location = 2)
in vec2 texcoords;
layout (location = 3)
in vec3 tangent;
layout (location = 4)
in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 aPosition;
out vec2 aTexCoords;
out vec4 aClipCoords;
out vec4 aLightPosition;
out vec3 aNormal;
out vec3 aTangent;
out vec3 aBitangent;

void main()
{
   vec4 pos = vec4(in_position, 1.0);
   aPosition = in_position;
   aTexCoords = texcoords;
   aNormal = normal;
   aTangent = tangent;
   aBitangent = bitangent;
   aClipCoords = projection * view * model * pos;
   gl_Position = aClipCoords;
}
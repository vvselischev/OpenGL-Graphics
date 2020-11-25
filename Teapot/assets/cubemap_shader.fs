#version 330 core

out vec4 o_frag_color;

in vec3 TexCoords;

uniform samplerCube cubemap;

void main()
{
    o_frag_color = texture(cubemap, TexCoords);
}

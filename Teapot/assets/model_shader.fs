#version 330 core

out vec4 o_frag_color;

in vec3 aPosition;
in vec3 aNormal;

uniform vec3 cameraPosition;
uniform float ratio;
uniform samplerCube cubemap;

void main()
{
    float r = 1.00 / ratio;

    vec3 I = normalize(aPosition - cameraPosition);
    vec3 R = reflect(I, normalize(aNormal));

    float t = dot(-I, normalize(aNormal));
    float ratioSqr = r * r;
    float tSqr = t * t;
    float fresnel = ((2 * ratioSqr * tSqr - 2 * sqrt(1 - ratioSqr + ratioSqr * tSqr) * t * r + 1 - ratioSqr) / (ratioSqr - 1));
    fresnel = fresnel * fresnel;

    vec3 R2 = refract(I, normalize(aNormal), r);

    o_frag_color = mix(vec4(texture(cubemap, R).rgb, 1.0), vec4(texture(cubemap, R2).rgb, 1.0), 1 - fresnel);
}

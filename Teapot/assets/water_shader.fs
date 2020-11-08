#version 330 core

out vec4 o_frag_color;

in vec3 aPosition;
in vec4 aClipCoords;
in vec2 aTexCoords;

uniform sampler2D water_normal;
uniform sampler2D water_dudv;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;

uniform vec3 sunPosition;
uniform vec3 projectorPosition;
uniform vec3 projectorDirection;
uniform float projectorAngle;

uniform float windFactor;

uniform vec3 cameraPosition;

void main()
{
    float specularStrength = 0.6;
    float windStrength = 0.02;

    vec2 ndc = (aClipCoords.xy / aClipCoords.w) / 2.0 + 0.5;
    vec2 R = vec2(ndc.x, 1 - ndc.y);
    vec2 R2 = vec2(ndc.x, ndc.y);

    vec2 dudv_1 = texture(water_dudv, vec2(aTexCoords.x + windFactor, aTexCoords.y)).xy * 0.1;
    dudv_1 = aTexCoords + vec2(dudv_1.x, dudv_1.y + windFactor);
    vec2 dudv = (texture(water_dudv, dudv_1).xy * 2.0 - 1.0) * windStrength;

    vec4 normalMap = texture(water_normal, dudv_1);
    vec3 aNormal = vec3(normalMap.x * 2.0 - 1.0, normalMap.y, normalMap.z * 2.0 - 1.0);
    aNormal = normalize(aNormal);

    float ambientStrength = 0.8;
    vec3 sunColor = vec3(1, 1, 1) ;
    vec3 projectorColor = vec3(1, 1, 1);
    float sunAttenuation = 1;
    float projectorAttenuation = 0.06;
    float projectorAmbient = 0.4;

    vec3 projectorRay = aPosition - projectorPosition;
    float pointAngle = dot(normalize(projectorRay), normalize(projectorDirection));
    projectorAttenuation = 1.0 / (1.0 + projectorAttenuation * pow(length(projectorRay), 2));
    if (pointAngle < cos(projectorAngle)) {
        projectorAttenuation = 0;
    }

    vec3 sunAmbient = ambientStrength * sunColor;
    vec3 sunDirection = vec3(-1, 0.5, -1);
    vec3 sunDiffuse = max(dot(normalize(aNormal), sunDirection), 0.0) * sunColor;

    vec3 projectorDiffuse = projectorColor;

    vec3 I = normalize(cameraPosition - aPosition);

    float r = 1.0 / 1.333;
    float t = dot(I, vec3(0.0, 1.0, 0.0));
    float ratioSqr = r * r;
    float tSqr = t * t;
    float fresnel = ((2 * ratioSqr * tSqr - 2 * sqrt(1 - ratioSqr + ratioSqr * tSqr) * t * r + 1 - ratioSqr) / (ratioSqr - 1));
    fresnel = fresnel * fresnel;

    R += dudv;
    R = clamp(R, 0.001, 0.999);
    R2 += dudv;
    R2 = clamp(R2, 0.001, 0.999);

    vec3 sunR = reflect(-sunDirection, normalize(aNormal));
    vec3 sunSpecular = pow(max(dot(I, sunR), 0.0), 20) * specularStrength * sunColor;

    vec3 result = clamp(sunAmbient + sunDiffuse + projectorAttenuation * projectorDiffuse, 0.0, 1.0);

    o_frag_color = vec4(result * mix(vec4(texture(reflection_texture, R).rgb + sunSpecular, 1.0), vec4(0, 0.484, 0.610, 1.0), 0.2).xyz, 1.0);
    o_frag_color = clamp(o_frag_color, 0.0, 1.0);
}

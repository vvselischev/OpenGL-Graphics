#version 330 core

out vec4 o_frag_color;

in vec3 aPosition;
in vec2 aTexCoords;
in vec4 aLightPosition1;
in vec4 aLightPosition2;
in vec4 aLightPosition3;
in float z;

uniform sampler2D sand_texture;
uniform sampler2D grass_texture;
uniform sampler2D rock_texture;
uniform float sand_threshold;
uniform float grass_threshold;
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;
uniform sampler2D shadowMap3;

uniform vec3 sunPosition;
uniform vec3 projectorPosition;
uniform vec3 projectorDirection;
uniform float projectorAngle;

uniform float plane1;
uniform float plane2;
uniform float plane3;

uniform vec3 cameraPosition;

float get_shadow(int i, vec3 aLightPosition, vec3 aNormal, vec3 sunDirection) {
    vec3 projCoords = aLightPosition * 0.5 + 0.5;
    float closestDepth = 0;

    if (i == 1) {
        closestDepth = texture(shadowMap1, projCoords.xy).r;
    } else if (i == 2) {
        closestDepth = texture(shadowMap2, projCoords.xy).r;
    } else if (i == 3) {
        closestDepth = texture(shadowMap3, projCoords.xy).r;
    }

    float currentDepth = projCoords.z;

    float bias = max(0.005 * (1.0 - dot(aNormal, sunDirection)), 0.0005);
    float shadow = 0.0;
    vec2 texelSize = vec2(0, 0);
    if (i == 1) {
        texelSize = 1.0 / textureSize(shadowMap1, 0);
    } else if (i == 2) {
        texelSize = 1.0 / textureSize(shadowMap2, 0);
    } else if (i == 3) {
        texelSize = 1.0 / textureSize(shadowMap3, 0);
    }

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcfDepth = 0;
            if (i == 1) {
                pcfDepth =texture(shadowMap1, projCoords.xy + vec2(x, y) * texelSize).r;
            } else if (i == 2) {
                pcfDepth = texture(shadowMap2, projCoords.xy + vec2(x, y) * texelSize).r;
            } else if (i == 3) {
               pcfDepth = texture(shadowMap3, projCoords.xy + vec2(x, y) * texelSize).r;
            }
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0) {
        shadow = 0.0;
    }

    return shadow;
}

void main()
{
    vec3 aNormal = vec3(0, 1, 0);

    float ambientStrength = 0.6;
    vec3 sunColor = vec3(1, 1, 1);
    vec3 projectorColor = vec3(1, 1, 1) * 0.5;
    float sunAttenuation = 1;
    float projectorAttenuation = 0.06;

    vec3 projectorRay = aPosition - projectorPosition;
    float pointAngle = dot(normalize(projectorRay), normalize(projectorDirection));
    projectorAttenuation = 1.0 / (1.0 + projectorAttenuation * pow(length(projectorRay), 2));
    if (pointAngle < cos(projectorAngle)) {
        projectorAttenuation = 0;
    }

    vec3 sunAmbient = ambientStrength * sunColor;
    vec3 sunDirection = normalize(sunPosition);
    vec3 sunDiffuse = max(dot(normalize(aNormal), sunDirection), 0.0) * 0.7 * sunColor;

    vec3 projectorDiffuse = max(dot(normalize(aNormal), -projectorRay), 0.0) * projectorColor * projectorAttenuation * 0.7;

    vec3 I = normalize(cameraPosition - aPosition);

    vec3 sunR = reflect(-sunDirection, normalize(aNormal));
    vec3 sunSpecular = pow(max(dot(I, sunR), 0.0), 16) * sunColor;

    vec3 aLightPosition;
    int i;
    if (z <= plane1) {
        aLightPosition = aLightPosition1.xyz;
        i = 1;
    } else if (z <= plane2) {
       aLightPosition = aLightPosition2.xyz;
       i = 2;
    } else {
        aLightPosition = aLightPosition3.xyz;
        i = 3;
    }

    float shadow = get_shadow(i, aLightPosition, aNormal, sunDirection);

    vec4 result = vec4(clamp(sunAmbient + projectorDiffuse + (1.0 - shadow) * sunDiffuse, 0.0, 1.0), 1.0);

    if (aPosition.y < sand_threshold - 0.001) {
        o_frag_color = mix(texture(sand_texture, aTexCoords), texture(grass_texture, aTexCoords), clamp(0, 1, 1 / (sand_threshold - aPosition.y) / 100));
    } else if (aPosition.y < grass_threshold) {
        o_frag_color = texture(grass_texture, aTexCoords);
    } else {
       o_frag_color = mix(texture(grass_texture, aTexCoords), texture(rock_texture, aTexCoords), clamp(0, 1, pow((aPosition.y - grass_threshold) * 4, 2)));
    }

    o_frag_color *= result;
}

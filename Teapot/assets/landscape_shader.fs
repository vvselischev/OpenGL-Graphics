#version 330 core

out vec4 o_frag_color;

in vec3 aPosition;
in vec2 aTexCoords;

uniform sampler2D terrain_texture;
//uniform sampler2D terrain_normal;

uniform vec3 sunPosition;
uniform vec3 projectorPosition;
uniform vec3 projectorDirection;
uniform float projectorAngle;

uniform vec3 cameraPosition;

void main()
{
//     float specularStrength = 0.4;
//     float windStrength = 0.05;
//
//     vec3 dudvn_1 = texture(water_normal, vec2(aTexCoords.x + windFactor, aTexCoords.y)).xzy;
//     vec3 dudvn_2 = texture(water_normal, vec2(-aTexCoords.x + windFactor, aTexCoords.y + windFactor)).xzy;
//     dudvn_1 = (dudvn_1 * 2 - vec3(1)) * windStrength;
//     dudvn_2 = (dudvn_2 * 2 - vec3(1)) * windStrength;
//     vec3 aNormal = dudvn_1 + dudvn_2;
//
//     vec3 dudv_1 = vec3(texture(water_dudv, vec2(aTexCoords.x + windFactor, aTexCoords.y)).xy, 0);
//     vec3 dudv_2 = vec3(texture(water_dudv, vec2(-aTexCoords.x + windFactor, aTexCoords.y + windFactor)).xy, 0);
//     dudv_1 = (dudv_1 * 2 - vec3(1)) * windStrength;
//     dudv_2 = (dudv_2 * 2 - vec3(1)) * windStrength;
//     vec3 dudv = dudv_1 + dudv_2;
//
//     float ambientStrength = 0.5;
//     vec3 sunColor = vec3(1, 1, 1) ;
//     vec3 projectorColor = vec3(1, 1, 0);
//     float sunAttenuation = 1;
//     float projectorAttenuation = 1;
//
//     vec3 projectorRay = aPosition - projectorPosition;
//     float pointAngle = acos(dot(normalize(projectorRay), normalize(projectorDirection)));
//     projectorAttenuation = 1.0 / (1.0 + projectorAttenuation * pow(length(projectorRay), 2));
//     if (pointAngle > projectorAngle) {
//         projectorAttenuation = 0;
//     }
//
//     vec3 sunAmbient = ambientStrength * sunColor;
//     vec3 sunDirection = normalize(sunPosition);
//     vec3 sunDiffuse = max(dot(normalize(aNormal), sunDirection), 0.0) * sunColor;
//
//     vec3 projectorDiffuse = max(dot(normalize(aNormal), -normalize(projectorRay)), 0.0) * projectorColor;
//
//     vec3 I = normalize(cameraPosition - aPosition);
//     vec3 R = reflect(-I, normalize(aNormal));
//
//     float r = 1.0 / 1.333;
//     float t = dot(I, normalize(aNormal));
//     float ratioSqr = r * r;
//     float tSqr = t * t;
//     float fresnel = ((2 * ratioSqr * tSqr - 2 * sqrt(1 - ratioSqr + ratioSqr * tSqr) * t * r + 1 - ratioSqr) / (ratioSqr - 1));
//     fresnel = fresnel * fresnel;
//
//     vec3 R2 = refract(-I, normalize(aNormal), r);
//
//     R += dudv;
//     R = clamp(R, 0.001, 0.999);
//
//     vec3 sunR = reflect(-sunDirection, normalize(aNormal));
//     vec3 sunSpecular = pow(max(dot(I, sunR), 0.0), 16) * specularStrength * sunColor;
//
//     vec3 projectorR = reflect(normalize(projectorRay), normalize(aNormal));
//     vec3 projectorSpecular = pow(max(dot(I, projectorR), 0.0), 16) * specularStrength * projectorColor;
//
//     vec4 result = vec4(clamp(sunAmbient + sunDiffuse + sunSpecular +
//         projectorAttenuation * (projectorDiffuse + projectorSpecular), 0.0, 1.0), 1.0);
//
//     o_frag_color = result * mix(vec4(texture(cubemap, R).rgb, 1.0), vec4(texture(water_texture, R2.xz).rgb, 1.0), 1 - fresnel);
    o_frag_color = texture(terrain_texture, aTexCoords);
}

#version 330 core

out vec4 o_frag_color;

in vec3 aPosition;
in vec3 aNormal;
in vec2 aTexCoords;

uniform vec3 in_diffuse;
uniform vec3 in_specular;
uniform vec3 in_ambient;

uniform sampler2D texture_diffuse1;

uniform vec3 sunPosition;
uniform vec3 projectorPosition;
uniform vec3 projectorDirection;
uniform float projectorAngle;

uniform vec3 cameraPosition;

void main()
{
    float ambientStrength = 0.5;
    vec3 sunColor = vec3(1, 1, 1) * 1;
    vec3 projectorColor = vec3(1, 1, 0);
    float sunAttenuation = 1;
    float projectorAttenuation = 1;

    vec3 projectorRay = aPosition - projectorPosition;
    float pointAngle = acos(dot(normalize(projectorRay), normalize(projectorDirection)));
    projectorAttenuation = 1.0 / (1.0 + projectorAttenuation * pow(length(projectorRay), 2));
    if (pointAngle > projectorAngle) {
        projectorAttenuation = 0;
    }

    vec3 sunAmbient = (ambientStrength + in_ambient) * sunColor;
    vec3 sunDirection = normalize(sunPosition);
    vec3 sunDiffuse = max(dot(normalize(aNormal), sunDirection), 0.0) * in_diffuse * sunColor;

    vec3 projectorAmbient = in_ambient * projectorColor;
    vec3 projectorDiffuse = max(dot(normalize(aNormal), -normalize(projectorRay)), 0.0) * in_diffuse * projectorColor;

    vec3 I = normalize(cameraPosition - aPosition);

    vec3 sunR = reflect(-sunDirection, normalize(aNormal));
    vec3 sunSpecular = pow(max(dot(I, sunR), 0.0), 16) * in_specular * sunColor;

    vec3 projectorR = reflect(normalize(projectorRay), normalize(aNormal));
    vec3 projectorSpecular = pow(max(dot(I, projectorR), 0.0), 16) * in_specular * projectorColor;

    vec4 result = vec4(sunAmbient + projectorAmbient + sunDiffuse + sunSpecular +
        projectorAttenuation * (projectorDiffuse + projectorSpecular), 1.0);

    o_frag_color = result * texture(texture_diffuse1, aTexCoords);
}

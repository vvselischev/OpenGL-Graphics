#version 330 core

out vec4 o_frag_color;

in vec3 aPosition;
in vec2 aTexCoords;
in vec3 aNormal;
in vec3 aTangent;
in vec3 aBitangent;

uniform sampler2D wall_texture;
uniform sampler2D wall_normal;
uniform sampler2D wall_height;

uniform bool pom;
uniform bool normalBump;
uniform bool pomAndShadows;

uniform vec3 worldLightDir1;
uniform vec3 worldLightDir2;
uniform vec3 worldLightDir3;
uniform vec3 worldLightDir4;

uniform vec4 zBufferParams;

uniform float MAX_HEIGHT;
uniform int MAX_STEP_COUNT;
uniform float STEP_LENGTH;

uniform float reflectivity;

uniform vec3 cameraPosition;

void main()
{
	vec3 lights[4] = vec3[](worldLightDir1, worldLightDir2, worldLightDir3, worldLightDir4);
    vec3 lightColor = vec3(1, 0.953, 0.836);

    vec3 worldViewDir = normalize(aPosition - cameraPosition);

    mat3 tbn = mat3(aTangent, aBitangent, aNormal);
    tbn = transpose(tbn);

    vec3 tangentViewDir = normalize(tbn * worldViewDir);
    vec3 tangentNormal = normalize(tbn * aNormal);

    vec2 uv = aTexCoords;

    float depth = 0;
    if (pom || pomAndShadows) {
    	depth = texture(wall_height, uv).r;
        float currentDepth = 0.0;
        for (int j = 1; j < MAX_STEP_COUNT; j++) {
            if (currentDepth < depth) {
                uv += MAX_HEIGHT * tangentViewDir.xy / tangentViewDir.z / MAX_STEP_COUNT;
                depth = texture(wall_height, uv).r;
                currentDepth = j * 1.0 / MAX_STEP_COUNT;
            } else {
            	break;
            }
        }
    }

	float shadow = 0;
    if (pomAndShadows) {
        shadow = 1;
    	for (int i = 0; i < 4; i++) {
    		float currentShadow = 0;
    		vec3 worldLightDir = normalize(aPosition - lights[i]);
			vec3 tangentLightDir = normalize(tbn * worldLightDir);

			vec2 currentUV = uv;

			float surfaceDepth = MAX_HEIGHT * (1 - texture(wall_height, currentUV).r);
			float currentDepth = surfaceDepth;

			for (int j = 1; j < MAX_STEP_COUNT; j++) {
				if (currentDepth > surfaceDepth) {
					currentShadow += max(1.0, (currentDepth - surfaceDepth) / MAX_HEIGHT);
				}

				currentUV -= tangentLightDir.xy * (MAX_HEIGHT / MAX_STEP_COUNT);
				surfaceDepth = MAX_HEIGHT * (1 - texture(wall_height, currentUV).r);
				currentDepth -= MAX_HEIGHT / MAX_STEP_COUNT;
			}
			shadow = min(shadow, currentShadow);
        }
    }

	vec3 normal = aNormal;
    if (normalBump || pom || pomAndShadows) {
        vec4 normalMap = texture(wall_normal, uv);
        normalMap.x *= normalMap.w;
        normal.xy = normalMap.xy * 2 - 1;
        normal.z = sqrt(1 - clamp(dot(normal.xy, normal.xy), 0, 1));
        normal = normalize(tbn * normalize(normal));
    }


    vec3 ambient = vec3(0.6);

	vec3 diffuseLight = vec3(0);
	shadow = clamp(shadow, 0, 1);

	for (int i = 0; i < 4; i++) {
		vec3 worldLightDir = lights[i] - aPosition;
		float cosTheta = max(0, dot(normal, normalize(worldLightDir)));
		diffuseLight += max(0, cosTheta) * lightColor * max(0, 1 - shadow);
    }

    diffuseLight = clamp(diffuseLight, 0, 1);

    o_frag_color = texture(wall_texture, uv);
    o_frag_color = vec4((diffuseLight + ambient) * o_frag_color.xyz, 1);
}

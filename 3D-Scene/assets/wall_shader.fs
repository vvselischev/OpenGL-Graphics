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
        float angle = acos(-dot(tangentNormal, tangentViewDir));
        for (int j = 0; j < MAX_STEP_COUNT; j++) {
            depth = MAX_HEIGHT - texture(wall_height, uv).r * MAX_HEIGHT;
            float currentDepth = (j + 1) * STEP_LENGTH / tan(angle);
            if (currentDepth < depth) {
                uv += STEP_LENGTH * tangentViewDir.xz;
            } else {
            	break;
            }
        }
        depth = MAX_HEIGHT - texture(wall_height, uv).r * MAX_HEIGHT;
    }

	float shadow = 0;
    if (pomAndShadows) {
        shadow = 1;
    	for (int i = 0; i < 4; i++) {
    		float currentShadow = 0;
    		vec3 worldLightDir = lights[i] - aPosition;
			vec3 tangentLightDir = normalize(tbn * worldLightDir);
			float angle = acos(dot(tangentNormal, tangentLightDir));
			vec2 currentUV = uv;
			float rayDepth = MAX_HEIGHT - texture(wall_height, currentUV).r * MAX_HEIGHT;

			for (int j = 1; j <= MAX_STEP_COUNT; j++) {
				float currentDepth = rayDepth - j * STEP_LENGTH / tan(angle);
				currentUV += STEP_LENGTH * tangentLightDir.xz;
				float surfaceDepth = MAX_HEIGHT - texture(wall_height, currentUV).r * MAX_HEIGHT;

				if (currentDepth > surfaceDepth) {
					currentShadow += max(1.0, (currentDepth - surfaceDepth) / surfaceDepth);
				} else if (currentDepth <= 0){
					break;
				}
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

    //float z = -depth;
    //z = 1.0 / (zBufferParams.z * z + zBufferParams.w);
   // gl_FragDepth = (1 - zBufferParams.w * z) / (zBufferParams.z * z);
}

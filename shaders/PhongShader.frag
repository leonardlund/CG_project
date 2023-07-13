#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;


layout(binding = 1) uniform sampler2D tex;

layout(binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightDir;
	vec4 lightColor;
	vec3 eyePos;
	vec3 lightDirDoll;
	vec4 lightColorDoll;
	vec3 eyePosDoll;
} gubo;

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 Md, vec3 Ms, float gamma) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 L  - light vector (from the light model)
	//vec3 Md - main color of the surface
	//vec3 Ms - specular color of the surface
	//float gamma - Exponent for power specular term

	const float cosAlpha = dot(N, L);
    const float clampY = clamp(cosAlpha, 0.0, 1.0);
    
    vec3 fDiffuse = L*Md*clampY; // the diffuse color. the light times the color of the object times the angle
    
    vec3 reflectedRayDir = N*dot(L, N);

    vec3 dPerpendicular = reflectedRayDir-L;
    vec3 r = dPerpendicular*2 + L;
    
    vec3 fSpecular = Ms*pow(clamp(dot(V, r), 0.0, 1.0), gamma);
 
	return fDiffuse+fSpecular;
}
vec4 lightSky() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	
	vec3 lightDir = gubo.lightDir;
	vec3 lightColor = gubo.lightColor.rgb;

	vec3 DiffSpec = BRDF(EyeDir, Norm, lightDir, texture(tex, fragUV).rgb, vec3(1.0f), 160.0f);
	vec3 Ambient = texture(tex, fragUV).rgb * 0.05f;
	
	vec4 skyColor = vec4(clamp(0.95 * DiffSpec * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
	
	return skyColor;
}

vec4 lightDoll() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePosDoll - fragPos);
	
	vec3 lightDir = gubo.lightDirDoll;
	vec3 lightColor = gubo.lightColorDoll.rgb;

	vec3 DiffSpecDoll = BRDF(EyeDir, Norm, gubo.lightDirDoll, texture(tex, fragUV).rgb, vec3(1.0f), 160.0f);
	vec3 Ambient = texture(tex, fragUV).rgb * 0.05f;
	
	vec4 dollColor = vec4(clamp(0.95 * DiffSpecDoll * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
	return dollColor;
}

void main() {
	
	outColor = lightSky() + lightDoll();
}
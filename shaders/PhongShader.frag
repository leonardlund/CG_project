#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;


layout(binding = 1) uniform sampler2D tex;

layout(binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightPos;
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
vec3 BRDF_DOLL(vec3 V, vec3 N, vec3 L, vec3 Md, vec3 Ms, float gamma) {
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
 
	return L;
}


vec4 lightDoll() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	
	vec3 lightDir = normalize(gubo.eyePosDoll - fragPos);
	vec3 lightColor = gubo.lightColorDoll.rgb;

	vec3 DiffuseAndSpecularDoll = BRDF_DOLL(EyeDir, Norm, lightDir, texture(tex, fragUV).rgb, vec3(1.0f), 160.0f);

	// Remember to put these in gubo?
	float outerConeAngle = radians(20);
	float innerConeAngle = radians(15);

    float angleToSpot = dot(normalize(-lightDir), normalize(gubo.lightDirDoll));
	float spotFactor = smoothstep(cos(outerConeAngle), cos(innerConeAngle), angleToSpot);

	
	return vec4(clamp(0.95 * DiffuseAndSpecularDoll * lightColor.rgb * spotFactor, 0.0, 1.0), 1.0f);
}

vec4 lightSky() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	
	vec3 lightDir = normalize(gubo.lightPos - fragPos);
	vec3 lightColor = gubo.lightColor.rgb;

	vec3 DiffSpec = BRDF(EyeDir, Norm, lightDir, texture(tex, fragUV).rgb, vec3(1.0f), 160.0f);
	vec3 Ambient = texture(tex, fragUV).rgb * 0.1f;
	
	return vec4(clamp(0.95 * (DiffSpec) * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
}

void main() {
	
	outColor = clamp(lightSky() + lightDoll(), 0.0, 1.0);

}
#version 450

layout (binding = 1) uniform sampler2D samplerColor;
layout (binding = 2) uniform sampler2D samplerNormalMap;
layout (binding = 3) uniform sampler2D samplerMaterial;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inWorldPos;
layout (location = 3) in vec3 inTangent;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMaterial;

void main() 
{
	vec4 albedo = texture(samplerColor, inUV);
	vec4 material = texture(samplerMaterial, inUV);
	vec4 norm = texture(samplerNormalMap, inUV);

	float metalness = material.r;
	float roughness = material.g;
	float ambient = norm.a;

	outPosition = vec4(inWorldPos, 1.0);

	// Calculate normal in tangent space
	vec3 N = normalize(inNormal);
	N.y = -N.y;
	vec3 T = normalize(inTangent);
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);
	vec3 tnorm = TBN * normalize(norm.xyz * 2.0 - vec3(1.0));

	outNormal = vec4(tnorm, 1.0);
	outAlbedo = vec4(albedo.rgb, material.b);
	outMaterial = vec4(roughness, metalness, ambient, 1.0);
}
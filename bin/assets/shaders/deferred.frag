#version 450

layout (binding = 1) uniform sampler2D samplerposition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerAlbedo;
layout (binding = 4) uniform sampler2D samplerMaterial;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

struct Light {
	vec4 position;
	vec3 color;
	float radius;
};

layout (binding = 5) uniform UBO 
{
	vec4 viewPos;
} ubo;

layout(push_constant) uniform PushConstants {
	Light light;
} push;

float PI = 3.1415;

vec3 Diffuse(vec3 pAlbedo)
{
    return pAlbedo/ PI;
}

float NormalDistribution_GGX(float a, float NdH)
{
    // Isotropic ggx.
    float a2 = a*a;
    float NdH2 = NdH * NdH;

    float denominator = NdH2 * (a2 - 1.0f) + 1.0f;
    denominator *= denominator;
    denominator *= PI;

    return a2 / denominator;
}

float Specular_D(float a, float NdH)
{
    return NormalDistribution_GGX(a, NdH);
}

float Geometric_Smith_Schlick_GGX(float a, float NdV, float NdL)
{
    // Smith schlick-GGX.
    float k = a * 0.5f;
    float GV = NdV / (NdV * (1 - k) + k);
    float GL = NdL / (NdL * (1 - k) + k);

    return GV * GL;
}

float Specular_G(float a, float NdV, float NdL, float NdH, float VdH, float LdV)
{
	 return Geometric_Smith_Schlick_GGX(a, NdV, NdL);
}

vec3 Fresnel_Schlick(vec3 specularColor, vec3 h, vec3 v)
{
    return (specularColor + (1.0f - specularColor) * pow((1.0f - clamp(dot(v, h), 0.0, 1.0)), 5));
}

vec3 Specular_F(vec3 specularColor, vec3 h, vec3 v)
{
    return Fresnel_Schlick(specularColor, h, v);
}

vec3 Specular(vec3 specularColor, vec3 h, vec3 v, vec3 l, float a, float NdL, float NdV, float NdH, float VdH, float LdV)
{
    return ((Specular_D(a, NdH) * Specular_G(a, NdV, NdL, NdH, VdH, LdV)) * Specular_F(specularColor, h, v) ) / (4.0f * NdL * NdV + 0.0001);
}

vec3 ComputeLight(vec3 albedoColor,vec3 specularColor, vec3 normal, float roughness, vec3 lightColor, vec3 lightDir, vec3 viewDir)
{
    // Compute some useful values.
    float NdL = clamp(dot(normal, lightDir), 0.0, 1.0);
    float NdV = clamp(dot(normal, viewDir), 0.0, 1.0);
    vec3 h = normalize(lightDir + viewDir);
    float NdH = clamp(dot(normal, h), 0.0, 1.0);
    float VdH = clamp(dot(viewDir, h), 0.0, 1.0);
    float LdV = clamp(dot(lightDir, viewDir), 0.0, 1.0);
    float a = max(0.001, roughness * roughness);

    vec3 cDiff = Diffuse(albedoColor);
    vec3 cSpec = Specular(specularColor, h, viewDir, lightDir, a, NdL, NdV, NdH, VdH, LdV);
	
	//return VdH.xxx;
    return lightColor * NdL * (cDiff * (1.0 - cSpec) + cSpec);
}

void main() 
{
	vec3 fragPos = texture(samplerposition, inUV).rgb;
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec4 albedoEmissive = texture(samplerAlbedo, inUV);
	vec4 material = texture(samplerMaterial, inUV);
	vec3 emissive = albedoEmissive.rgb * albedoEmissive.a;
	vec3 albedo = albedoEmissive.rgb;

	vec3 toEye = normalize(ubo.viewPos.xyz - fragPos);
	vec3 tl = push.light.position.xyz - fragPos;
	float toLightDistance = length(tl);
	float distanceSqr = dot(tl, tl);
	tl = normalize(tl);

	float falloff = ((pow(clamp(1.0 - pow(toLightDistance / push.light.radius, 4),0.0,1.0), 2)) / (toLightDistance * toLightDistance + 1)) * 8;

	vec3 spec = mix(vec3(0.04), albedo, vec3(material.g));
	vec3 fragColor = ComputeLight(albedo, spec, normal, material.r, push.light.color, tl, toEye) * falloff;

	vec3 color = vec3(0.0);//albedo * material.b * 0.05; //ambient
	color += fragColor;

    outFragcolor = vec4(color, 1.0);
}
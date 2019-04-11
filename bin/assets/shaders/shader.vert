#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {
	mat4 model;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 1) out vec2 outUV;

void main()
{
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPosition, 1.0);
    outUV = inUV;
}
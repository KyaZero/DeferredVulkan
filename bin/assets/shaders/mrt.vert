#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {
	mat4 model;
} push;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out vec3 outTangent;

void main()
{
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPosition, 1.0);
	
	outUV = inUV;
	outUV.t = 1.0 - outUV.t;

	// Vertex position in world space
	outWorldPos = vec3(push.model * vec4(inPosition,1.0));
	// GL to Vulkan coord space
	// outWorldPos.y = -outWorldPos.y;
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(push.model)));
	outNormal = mNormal * normalize(inNormal);	
	outTangent = mNormal * normalize(inTangent);
}

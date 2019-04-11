#pragma once

#include <Frostwave/Core/Math/Matrix.h>
#include <Frostwave/Graphics/VulkanBuffer.h>
#include <Frostwave/Graphics/Camera.h>
#include <Frostwave/Graphics/Model.h>
#include <Frostwave/Core/Timer.h>
#include <Frostwave/Graphics/Lights.h>

#include <vulkan/vulkan.h>
#include <vector>

namespace frostwave
{
	struct UniformBufferObject 
	{
		alignas(16) fw::Mat4f view;
		alignas(16) fw::Mat4f projection;
	};	

	struct UniformBufferObjectFullscreen
	{
		fw::Vec4f cameraPos;
	};

	class ModelInstance;
	class VkFramework;
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void Init(VkFramework* aFramework);
		void Render(const std::vector<ModelInstance*>& aModels, const std::vector<PointLight>& aLights, fw::Camera* aCamera);
		void Destroy();

		fw::Buffer* GetUBO();

		const VkDescriptorSetLayout& GetDescriptorSetLayout() const;

	private:
		void Resize();
		void PrepareOffscreenFramebuffer();
		void CreatePoolAndBuffers();
		void SetupDescriptorSetLayout();
		void PreparePipelines();
		void GenerateQuad();
		void SetupDescriptorSet();
		void PrepareUniformBuffers();
		void BuildDeferredCommandBuffers(VkFramebuffer aFramebuffer, Camera* aCamera, const std::vector<PointLight>& aLights);
		void SetupDescriptorPool();
		
		struct PipelineLayouts
		{
			VkPipelineLayout forward, offscreen, deferred;
		} myPipelineLayouts;

		struct {
			VkPipeline forward;
			VkPipeline deferred;
			VkPipeline offscreen;
		} myPipelines;

		struct FrameBufferAttachment
		{
			VkImage image;
			VkDeviceMemory memory;
			VkImageView view;
			VkFormat format;
		};

		void CreateAttachment(VkFormat aFormat, VkImageUsageFlagBits aUsage, FrameBufferAttachment* aAttachment);

		struct FrameBuffer
		{
			i32 width, height;
			VkFramebuffer frameBuffer;
			FrameBufferAttachment position, normal, albedo, material;
			FrameBufferAttachment depth;
			VkRenderPass renderPass;
		} myOffscreenFramebuffer;

		void DestroyOffscreenFrameBuffer();

		VkDescriptorSet myDescriptorSet;
		VkDescriptorSetLayout myDescriptorSetLayout;
		VkSampler myColorSampler;

		Model myQuad;

		VkSemaphore myOffscreenSemaphore;
		VkDescriptorPool myDescriptorPool;
		VkCommandPool myCommandPool;
		VkFramework* myFramework;
		std::vector<VkCommandBuffer> myCommandBuffers;
		VkCommandBuffer myDeferredCommandBuffer;
		fw::Timer myTimer;

		struct
		{
			Buffer offscreen;
			Buffer fullscreen;
		} myUniformBuffers;

		UniformBufferObject myUBO;
		UniformBufferObjectFullscreen myUBOFullscreen;
	};
}
namespace fw = frostwave;
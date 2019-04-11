#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <Frostwave/Core/Types.h>
#include <Frostwave/Debug/Logger.h>
#include <Frostwave/Core/Common.h>
#include <Frostwave/Settings.h>

#include "VulkanInitializers.h"
#include "VulkanUtils.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"
//#include "Texture.h"
#include "Model.h"

inline fw::VertexLayout layout = fw::VertexLayout({
	fw::VERTEX_COMPONENT_POSITION,
	fw::VERTEX_COMPONENT_UV,
	fw::VERTEX_COMPONENT_NORMAL,
	fw::VERTEX_COMPONENT_TANGENT
});

namespace frostwave
{
	struct QueueFamilyIndices
	{
		i32 graphicsFamily = -1;
		i32 presentFamily = -1;

		bool IsComplete()
		{
			return graphicsFamily != -1 && presentFamily != -1;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VkFramework
	{
	public:
		VkFramework() : myPhysicalDevice(VK_NULL_HANDLE), myCurrentFrame(0), myFramebufferResized(false), myMSAASamples(VK_SAMPLE_COUNT_1_BIT) {}
		~VkFramework();
		void Init(GLFWwindow* aWindow, GraphicsSettings aSettings);
		u32 BeginFrame();
		bool EndFrame(u32 aIndex, VkSemaphore aSignalSemaphore);

		void WaitIdle();

		void SetFramebufferResized(bool aResized);

		VkDevice GetDevice() const;
		VkPhysicalDevice GetPhysicalDevice() const;

		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetCommandPool() const;

		VkRenderPass GetRenderPass() const;

		const VkDescriptorSetLayout& GetDescriptorSetLayout() const;
		VkDescriptorPool& GetDescriptorPool();

		VkPipeline GetPipeline() const;
		VkPipelineLayout GetPipelineLayout() const;

		VkSampleCountFlagBits GetMSAASamples() const;

		QueueFamilyIndices GetQueueFamilyIndices() const;

		VkCommandBufferInheritanceInfo BeginCommandBufferRecording(u32 aImageIndex, VkRenderPassBeginInfo aRenderPassInfo);
		bool EndCommandBufferRecording(u32 aImageIndex, std::vector<VkCommandBuffer> aSecondaryCommands);

		u32 GetFrameBufferCount();
		VkExtent2D GetSwapchainExtent() const;
		VkFormat FindDepthFormat();
		VkShaderModule CreateShaderModule(const std::vector<char>& aCode);

		const std::vector<VkFramebuffer>& GetFrameBuffers() const;

	private:
		friend class Renderer;
		bool CreateInstance();
		std::vector<const char*> GetRequiredExtensions();
		bool CheckValidationLayerSupport();

		bool InitVulkan();
		bool CreateSyncObjects();
		bool CreateCommandBuffers();
		bool CreateCommandPool();
		bool CreateFrameBuffers();
		bool CreateUniformBuffers();
		bool CreateRenderPass();
		bool CreateDescriptorPool();
		bool CreateDescriptorSetLayout();
		bool CreateGraphicsPipeline();
		bool CreateImageViews();
		bool CreateSwapChain();
		bool CreateSurface();
		bool CreateLogicalDevice();
		bool CreateTexture();
		bool CreateDepthResources();
		bool CreateColorResources();

		bool RecreateSwapChain();
		bool CleanupSwapChain();

		void CreateImage(u32 aWidth, u32 aHeight, u32 aMipLevels, VkSampleCountFlagBits aNumSamples, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags aProperties, VkImage& aImage, VkDeviceMemory& aImageMemory);
		VkImageView CreateImageView(VkImage aImage, VkFormat aFormat, VkImageAspectFlags aAspectFlags, u32 aMipLevels);

		void TransitionImageLayout(VkImage aImage, VkFormat aFormat, VkImageLayout aOldLayout, VkImageLayout aNewLayout, u32 aMipLevels);

		VkSampleCountFlagBits GetMaxUsableSampleCount();

		i32 RateDeviceSuitability(VkPhysicalDevice aDevice);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice aDevice);
		QueueFamilyIndices FindQueueFamily(VkPhysicalDevice aDevice) const;
		bool IsDeviceSuitable(VkPhysicalDevice aDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice aDevice);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& aAvailableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> aAvailablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& aCapabilities);

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& aCandidates, VkImageTiling aTiling, VkFormatFeatureFlags aFeatures);

		bool PickPhysicalDevice();
		bool SetupDebugMessenger();

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT aMessageSeverity, VkDebugUtilsMessageTypeFlagsEXT aMessageType, const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData, void* aUserData);

	private:
		GLFWwindow* myWindow;
		GraphicsSettings mySettings;

		VkInstance myInstance;
		VkDebugUtilsMessengerEXT myDebugMessenger;
		VkPhysicalDevice myPhysicalDevice;
		VkDevice myDevice;
		VkQueue myGraphicsQueue, myPresentQueue;
		VkSurfaceKHR mySurface;

		VkSwapchainKHR mySwapChain;
		std::vector<VkImage> mySwapChainImages;
		VkFormat mySwapChainFormat;
		VkExtent2D mySwapChainExtent;
		std::vector<VkImageView> mySwapChainImageViews;

		VkRenderPass myRenderPass;
		VkDescriptorPool myDescriptorPool;
		VkDescriptorSetLayout myDescriptorSetLayout;
		VkPipelineLayout myPipelineLayout;
		VkPipeline myGraphicsPipeline;

		std::vector<VkFramebuffer> mySwapChainFramebuffers;
		VkCommandPool myCommandPool;
		std::vector<VkCommandBuffer> myCommandBuffers;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffers;

		std::vector<VkSemaphore> myImageAvailableSemaphores;
		std::vector<VkSemaphore> myRenderFinishedSemaphores;
		std::vector<VkFence> myInFlightFences;
		size_t myCurrentFrame;

		bool myFramebufferResized;

		VulkanImage myDepthImage;

		VkSampleCountFlagBits myMSAASamples;
		VulkanImage myColorImage;
	};
}

namespace fw = frostwave;
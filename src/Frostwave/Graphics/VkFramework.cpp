#include "stdafx.h"
#include "VkFramework.h"
#include <Frostwave/Engine.h>

#include <Frostwave/Graphics/VulkanInitializers.h>

#include <Graphics/imgui/imgui.h>
#include <Graphics/imgui/imgui_impl_glfw.h>
#include <Graphics/imgui/imgui_impl_vulkan.h>

constexpr u32 WIDTH = 800;
constexpr u32 HEIGHT = 600;

const string MODEL_PATH = "assets/meshes/2b.obj";
const string TEXTURE_PATH = "assets/textures/2b.png";

constexpr i32 MaxNumBufferedFrames = 1;
const std::vector<const char*> ValidationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance aInstance, const VkDebugUtilsMessengerCreateInfoEXT* aCreateInfo,
	const VkAllocationCallbacks* aAllocator, VkDebugUtilsMessengerEXT* aDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(aInstance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(aInstance, aCreateInfo, aAllocator, aDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance aInstance, VkDebugUtilsMessengerEXT aDebugMessenger, const VkAllocationCallbacks* aAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(aInstance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(aInstance, aDebugMessenger, aAllocator);
	}
}

frostwave::VkFramework::~VkFramework()
{
	WaitIdle();

	CleanupSwapChain();

	vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(myDevice, myDescriptorSetLayout, nullptr);

	for (size_t i = 0; i < MaxNumBufferedFrames; ++i)
	{
		vkDestroySemaphore(myDevice, myRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(myDevice, myImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(myDevice, myInFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(myDevice, myCommandPool, nullptr);

	vkDestroyDevice(myDevice, nullptr);

	if (mySettings.validation)
	{
		DestroyDebugUtilsMessengerEXT(myInstance, myDebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(myInstance, mySurface, nullptr);
	vkDestroyInstance(myInstance, nullptr);
}

void frostwave::VkFramework::Init(GLFWwindow* aWindow, GraphicsSettings aSettings)
{
	myWindow = aWindow;
	mySettings = aSettings;
	InitVulkan();
}

u32 frostwave::VkFramework::BeginFrame()
{
	vkWaitForFences(myDevice, 1, &myInFlightFences[myCurrentFrame], VK_TRUE, std::numeric_limits<u64>::max());

	u32 imageIndex;
	VkResult result = vkAcquireNextImageKHR(myDevice, mySwapChain, std::numeric_limits<u64>::max(), myImageAvailableSemaphores[myCurrentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return -1;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		FATAL_LOG("Failed to acquire swap chain image!");
	}

	return imageIndex;
}

bool frostwave::VkFramework::EndFrame(u32 aIndex, VkSemaphore aSignalSemaphore)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &aSignalSemaphore;

	VkSwapchainKHR swapChains[] = { mySwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &aIndex;
	presentInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(myPresentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || myFramebufferResized)
	{
		myFramebufferResized = false;
		RecreateSwapChain();
		return false;
	}
	else if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to present swap chain image!");
	}

	myCurrentFrame = (myCurrentFrame + 1) % MaxNumBufferedFrames;
	return true;
}

void frostwave::VkFramework::WaitIdle()
{
	vkDeviceWaitIdle(myDevice);
}

void frostwave::VkFramework::SetFramebufferResized(bool aResized)
{
	myFramebufferResized = aResized;
}

VkDevice frostwave::VkFramework::GetDevice() const
{
	assert(myDevice != VK_NULL_HANDLE);
	return myDevice;
}

VkPhysicalDevice frostwave::VkFramework::GetPhysicalDevice() const
{
	assert(myPhysicalDevice != VK_NULL_HANDLE);
	return myPhysicalDevice;
}

VkQueue frostwave::VkFramework::GetGraphicsQueue() const
{
	assert(myGraphicsQueue != VK_NULL_HANDLE);
	return myGraphicsQueue;
}

VkCommandPool frostwave::VkFramework::GetCommandPool() const
{
	assert(myCommandPool != VK_NULL_HANDLE);
	return myCommandPool;
}

VkRenderPass frostwave::VkFramework::GetRenderPass() const
{
	return myRenderPass;
}

const VkDescriptorSetLayout& frostwave::VkFramework::GetDescriptorSetLayout() const
{
	return myDescriptorSetLayout;
}

VkDescriptorPool& frostwave::VkFramework::GetDescriptorPool()
{
	return myDescriptorPool;
}

VkPipeline frostwave::VkFramework::GetPipeline() const
{
	return myGraphicsPipeline;
}

VkPipelineLayout frostwave::VkFramework::GetPipelineLayout() const
{
	return myPipelineLayout;
}

VkSampleCountFlagBits frostwave::VkFramework::GetMSAASamples() const
{
	return myMSAASamples;
}

frostwave::QueueFamilyIndices frostwave::VkFramework::GetQueueFamilyIndices() const
{
	return FindQueueFamily(myPhysicalDevice);
}

bool frostwave::VkFramework::CreateInstance()
{
	if (mySettings.validation && !CheckValidationLayerSupport())
	{
		FATAL_LOG("Validation layers requested, but are not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Frostwave Test";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "Frostwave";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	u32 glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	if (mySettings.validation)
	{
		createInfo.enabledLayerCount = (u32)(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	auto ext = GetRequiredExtensions();
	createInfo.enabledExtensionCount = (u32)ext.size();
	createInfo.ppEnabledExtensionNames = ext.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &myInstance);

	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create VK Instance!");
		return false;
	}
	return true;
}

std::vector<const char*> frostwave::VkFramework::GetRequiredExtensions()
{
	u32 glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (mySettings.validation)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL frostwave::VkFramework::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT aMessageSeverity, VkDebugUtilsMessageTypeFlagsEXT aMessageType, const VkDebugUtilsMessengerCallbackDataEXT * aCallbackData, void * aUserData)
{
	aUserData; aMessageType;
	if (aMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		ERROR_LOG("Validation Layer: %s", aCallbackData->pMessage);
	}
	else if (aMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		WARNING_LOG("Validation Layer: %s", aCallbackData->pMessage);
	}
	else if (aMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		INFO_LOG("Validation Layer: %s", aCallbackData->pMessage);
	}
	else if (aMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		VERBOSE_LOG("Validation Layer: %s", aCallbackData->pMessage);
	}
	return VK_FALSE;
}

u32 frostwave::VkFramework::GetFrameBufferCount()
{
	return (u32)mySwapChainFramebuffers.size();
}

VkExtent2D frostwave::VkFramework::GetSwapchainExtent() const
{
	return mySwapChainExtent;
}

bool frostwave::VkFramework::CheckValidationLayerSupport()
{
	u32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (auto* layerName : ValidationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

bool frostwave::VkFramework::InitVulkan()
{
	if (!CreateInstance()) return false;
	VERBOSE_LOG("Created Vulkan Instance");
	if (!SetupDebugMessenger()) return false;
	VERBOSE_LOG("Set up debug messenger");
	if (!CreateSurface()) return false;
	VERBOSE_LOG("Created window surface");
	if (!PickPhysicalDevice()) return false;
	if (!CreateLogicalDevice()) return false;
	VERBOSE_LOG("Created logical device");
	if (!CreateSwapChain()) return false;
	VERBOSE_LOG("Created swapchain");
	if (!CreateImageViews()) return false;
	VERBOSE_LOG("Created swapchain image views");
	if (!CreateRenderPass()) return false;
	VERBOSE_LOG("Created render pass");
	if (!CreateDescriptorSetLayout()) return false;
	VERBOSE_LOG("Created descriptor set layout");
	if (!CreateGraphicsPipeline()) return false;
	VERBOSE_LOG("Created graphics pipeline");
	if (!CreateCommandPool()) return false;
	VERBOSE_LOG("Created command pool");
	if (!CreateColorResources()) return false;
	VERBOSE_LOG("Created color resource attachments");
	if (!CreateDepthResources()) return false;
	VERBOSE_LOG("Created depth resource attachments");
	if (!CreateFrameBuffers()) return false;
	VERBOSE_LOG("Created frame buffers");
	if (!CreateTexture()) return false;
	VERBOSE_LOG("Created texture image");
	if (!CreateUniformBuffers()) return false;
	VERBOSE_LOG("Created uniform buffers");
	if (!CreateDescriptorPool()) return false;
	VERBOSE_LOG("Created descriptor pool");
	if (!CreateCommandBuffers()) return false;
	VERBOSE_LOG("Created command buffers");
	if (!CreateSyncObjects()) return false;
	VERBOSE_LOG("Created synchronization objects");

	INFO_LOG("Initialized Vulkan");
	return true;
}

bool frostwave::VkFramework::CreateSyncObjects()
{
	myImageAvailableSemaphores.resize(MaxNumBufferedFrames);
	myRenderFinishedSemaphores.resize(MaxNumBufferedFrames);
	myInFlightFences.resize(MaxNumBufferedFrames);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MaxNumBufferedFrames; ++i)
	{
		if (vkCreateSemaphore(myDevice, &semaphoreInfo, nullptr, &myImageAvailableSemaphores[i]) ||
			vkCreateSemaphore(myDevice, &semaphoreInfo, nullptr, &myRenderFinishedSemaphores[i]) ||
			vkCreateFence(myDevice, &fenceInfo, nullptr, &myInFlightFences[i]) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create synchronization objects for a frame!");
			return false;
		}
	}
	return true;
}

VkCommandBufferInheritanceInfo frostwave::VkFramework::BeginCommandBufferRecording(u32 aImageIndex, VkRenderPassBeginInfo aRenderPassInfo)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pInheritanceInfo = nullptr;

	VkResult result = vkBeginCommandBuffer(myCommandBuffers[aImageIndex], &beginInfo);

	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to begin recording command buffer!");
	}

	vkCmdBeginRenderPass(myCommandBuffers[aImageIndex], &aRenderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	VkCommandBufferInheritanceInfo inheritanceInfo = { };
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = aRenderPassInfo.renderPass;
	inheritanceInfo.framebuffer = aRenderPassInfo.framebuffer;

	return inheritanceInfo;
}

bool frostwave::VkFramework::EndCommandBufferRecording(u32 aImageIndex, std::vector<VkCommandBuffer> aSecondaryCommands)
{
	if (aSecondaryCommands.size() > 0)
	{
		vkCmdExecuteCommands(myCommandBuffers[aImageIndex], (u32)aSecondaryCommands.size(), aSecondaryCommands.data());
	}

	vkCmdEndRenderPass(myCommandBuffers[aImageIndex]);

	VkResult result = vkEndCommandBuffer(myCommandBuffers[aImageIndex]);

	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to record command buffer!");
		return false;
	}

	return false;
}

bool frostwave::VkFramework::CreateCommandBuffers()
{
	myCommandBuffers.resize(mySwapChainFramebuffers.size());
	mySecondaryCommandBuffers.resize(mySwapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = myCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (u32)myCommandBuffers.size();

	VkResult result = vkAllocateCommandBuffers(myDevice, &allocInfo, myCommandBuffers.data());
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate command buffers!");
		return false;
	}


	VkCommandBufferAllocateInfo secondAllocInfo = {};
	secondAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	secondAllocInfo.commandPool = myCommandPool;
	secondAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	secondAllocInfo.commandBufferCount = (u32)mySecondaryCommandBuffers.size();

	result = vkAllocateCommandBuffers(myDevice, &secondAllocInfo, mySecondaryCommandBuffers.data());
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate secondary command buffer!");
		return false;
	}

	return true;
}

bool frostwave::VkFramework::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamily(myPhysicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkResult result = vkCreateCommandPool(myDevice, &poolInfo, nullptr, &myCommandPool);

	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create command pool!");
		return false;
	}
	return true;
}

bool frostwave::VkFramework::CreateFrameBuffers()
{
	mySwapChainFramebuffers.resize(mySwapChainImageViews.size());

	VkResult result = VK_SUCCESS;
	for (size_t i = 0; i < mySwapChainImageViews.size(); ++i)
	{
		std::array<VkImageView, 2> attachments = {
			mySwapChainImageViews[i],
			myDepthImage.GetImageView()
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = myRenderPass;
		framebufferInfo.attachmentCount = (u32)attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = mySwapChainExtent.width;
		framebufferInfo.height = mySwapChainExtent.height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(myDevice, &framebufferInfo, nullptr, &mySwapChainFramebuffers[i]);
		if (result != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create framebuffer!");
			return false;
		}
	}
	return true;
}

bool frostwave::VkFramework::CreateUniformBuffers()
{

	return true;
}

bool frostwave::VkFramework::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = mySwapChainFormat;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachment.samples = myMSAASamples;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = mySwapChainFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRef = { };
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.samples = myMSAASamples;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	//subpass.pResolveAttachments = &colorAttachmentResolveRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (u32)attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkResult result = vkCreateRenderPass(myDevice, &renderPassInfo, nullptr, &myRenderPass);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create render pass!");
		return false;
	}
	return true;
}

bool frostwave::VkFramework::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 100;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 100;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (u32)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 100;

	VkResult result = vkCreateDescriptorPool(myDevice, &poolInfo, nullptr, &myDescriptorPool);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create descriptor pool!");
		return false;
	}

	return true;
}

bool frostwave::VkFramework::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (u32)bindings.size();
	layoutInfo.pBindings = bindings.data();
	VkResult result = vkCreateDescriptorSetLayout(myDevice, &layoutInfo, nullptr, &myDescriptorSetLayout);

	if (result != VK_SUCCESS) {
		FATAL_LOG("Failed to create descriptor set layout!");
		return false;
	}

	return true;
}

VkFormat frostwave::VkFramework::FindDepthFormat()
{
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkShaderModule frostwave::VkFramework::CreateShaderModule(const std::vector<char>& aCode)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = aCode.size();
	createInfo.pCode = (const u32*)aCode.data();

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(myDevice, &createInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		ERROR_LOG("Failed to create shader module!");
	}

	return shaderModule;
}

const std::vector<VkFramebuffer>& frostwave::VkFramework::GetFrameBuffers() const
{
	return mySwapChainFramebuffers;
}

bool frostwave::VkFramework::CreateGraphicsPipeline()
{
	auto vertShaderCode = ReadFile("assets/shaders/vert.spv");
	auto fragShaderCode = ReadFile("assets/shaders/frag.spv");

	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[2] = { vertShaderStageInfo, fragShaderStageInfo };

	std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
		fw::initializers::VertexInputBindingDescription(0, layout.Stride(), VK_VERTEX_INPUT_RATE_VERTEX)
	};

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
		fw::initializers::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),					//position
		fw::initializers::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(f32) * 3),		//texcoord
		fw::initializers::VertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32B32_SFLOAT, sizeof(f32) * 5),	//normal
		fw::initializers::VertexInputAttributeDescription(0, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(f32) * 8),	//tangent
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = (u32)bindingDescriptions.size();
	vertexInputInfo.vertexAttributeDescriptionCount = (u32)attributeDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (f32)mySwapChainExtent.width;
	viewport.height = (f32)mySwapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = mySwapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.rasterizationSamples = myMSAASamples;
	multisampling.minSampleShading = 0.2f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.size = sizeof(fw::Mat4f);
	pushConstantRange.offset = 0;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &myDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	VkResult result = vkCreatePipelineLayout(myDevice, &pipelineLayoutInfo, nullptr, &myPipelineLayout);

	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create pipeline layout!");
		return false;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = myPipelineLayout;
	pipelineInfo.renderPass = myRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myGraphicsPipeline);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create graphics pipeline!");
		return false;
	}

	vkDestroyShaderModule(myDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(myDevice, vertShaderModule, nullptr);
	return true;
}

bool frostwave::VkFramework::CreateImageViews()
{
	mySwapChainImageViews.resize(mySwapChainImages.size());

	for (u64 i = 0; i < (u64)mySwapChainImages.size(); ++i)
	{
		mySwapChainImageViews[i] = CreateImageView(mySwapChainImages[i], mySwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
	return true;
}

bool frostwave::VkFramework::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(myPhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mySurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamily(myPhysicalDevice);
	u32 queueFamilyIndices[] = { (u32)indices.graphicsFamily, (u32)indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(myDevice, &createInfo, nullptr, &mySwapChain);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(myDevice, mySwapChain, &imageCount, nullptr);
	mySwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(myDevice, mySwapChain, &imageCount, mySwapChainImages.data());

	mySwapChainFormat = surfaceFormat.format;
	mySwapChainExtent = extent;
	return true;
}

bool frostwave::VkFramework::CreateSurface()
{
	VkResult result = glfwCreateWindowSurface(myInstance, myWindow, nullptr, &mySurface);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create window surface!");
		return false;
	}
	return true;
}

bool frostwave::VkFramework::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamily(myPhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<u32> uniqueQueueFamilies = { (u32)indices.graphicsFamily, (u32)indices.presentFamily };

	float queuePriority = 1.0f;
	for (auto& queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = (u32)DeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

	if (mySettings.validation)
	{
		createInfo.enabledLayerCount = (u32)ValidationLayers.size();
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	VkResult result = vkCreateDevice(myPhysicalDevice, &createInfo, nullptr, &myDevice);

	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create logical device!");
		return false;
	}

	vkGetDeviceQueue(myDevice, indices.graphicsFamily, 0, &myGraphicsQueue);
	vkGetDeviceQueue(myDevice, indices.presentFamily, 0, &myPresentQueue);
	return true;
}

bool frostwave::VkFramework::CreateTexture()
{
	//ImageCreateInfo info = {};
	//info.type = ImageType::Texture;
	//info.width = 0;
	//info.height = 0;
	//info.format = VK_FORMAT_R8G8B8A8_UNORM;
	//info.samples = VK_SAMPLE_COUNT_1_BIT;
	//info.path = TEXTURE_PATH;
	//info.generateMips = true;
	//myTexture.Create(this, info);
	//myTexture.Load(TEXTURE_PATH, VK_FORMAT_R8G8B8A8_UNORM, this, myGraphicsQueue);
	return true;
}

bool frostwave::VkFramework::CreateDepthResources()
{
	VkFormat depthFormat = FindDepthFormat();
	ImageCreateInfo info = {};
	info.type = ImageType::Depth;
	info.width = mySwapChainExtent.width;
	info.height = mySwapChainExtent.height;
	info.format = depthFormat;
	info.samples = myMSAASamples;
	myDepthImage.Create(this, info);
	return true;
}

bool frostwave::VkFramework::CreateColorResources()
{
	VkFormat colorFormat = mySwapChainFormat;
	ImageCreateInfo info = {};
	info.type = ImageType::ColorAttachment;
	info.width = mySwapChainExtent.width;
	info.height = mySwapChainExtent.height;
	info.format = colorFormat;
	info.samples = myMSAASamples;
	myColorImage.Create(this, info);
	return true;
}

bool frostwave::VkFramework::RecreateSwapChain()
{
	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(myWindow, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(myDevice);

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateColorResources();
	CreateDepthResources();
	CreateFrameBuffers();
	CreateCommandBuffers();

	VERBOSE_LOG("Recreated swapchain");

	return true;
}

bool frostwave::VkFramework::CleanupSwapChain()
{
	myColorImage.Destroy();

	myDepthImage.Destroy();

	for (size_t i = 0; i < mySwapChainFramebuffers.size(); ++i)
	{
		vkDestroyFramebuffer(myDevice, mySwapChainFramebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(myDevice, myCommandPool, (u32)myCommandBuffers.size(), myCommandBuffers.data());

	vkDestroyPipeline(myDevice, myGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(myDevice, myPipelineLayout, nullptr);
	vkDestroyRenderPass(myDevice, myRenderPass, nullptr);

	for (size_t i = 0; i < mySwapChainImageViews.size(); ++i)
	{
		vkDestroyImageView(myDevice, mySwapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(myDevice, mySwapChain, nullptr);

	VERBOSE_LOG("Cleaned up swapchain");

	return true;
}

void frostwave::VkFramework::CreateImage(u32 aWidth, u32 aHeight, u32 aMipLevels, VkSampleCountFlagBits aNumSamples, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags aProperties, VkImage& aImage, VkDeviceMemory& aImageMemory)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = aWidth;
	imageInfo.extent.height = aHeight;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = aFormat;
	imageInfo.tiling = aTiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = aUsage;
	imageInfo.samples = aNumSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.mipLevels = aMipLevels;

	VkResult result = vkCreateImage(myDevice, &imageInfo, nullptr, &aImage);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(myDevice, aImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(myPhysicalDevice, memRequirements.memoryTypeBits, aProperties);

	result = vkAllocateMemory(myDevice, &allocInfo, nullptr, &aImageMemory);

	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate image memory!");
	}

	vkBindImageMemory(myDevice, aImage, aImageMemory, 0);
}

VkImageView frostwave::VkFramework::CreateImageView(VkImage aImage, VkFormat aFormat, VkImageAspectFlags aAspectFlags, u32 aMipLevels)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = aImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = aFormat;
	viewInfo.subresourceRange.aspectMask = aAspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = aMipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VkResult result = vkCreateImageView(myDevice, &viewInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create texture image view!");
	}
	return imageView;
}

void frostwave::VkFramework::TransitionImageLayout(VkImage aImage, VkFormat aFormat, VkImageLayout aOldLayout, VkImageLayout aNewLayout, u32 aMipLevels)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(this);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = aOldLayout;
	barrier.newLayout = aNewLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = aImage;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = aMipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (aNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(aFormat))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (aOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && aNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (aOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && aNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (aOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && aNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (aOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && aNewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else
	{
		FATAL_LOG("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	EndSingleTimeCommands(commandBuffer, this);
}

VkSampleCountFlagBits frostwave::VkFramework::GetMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(myPhysicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = fw::Min(physicalDeviceProperties.limits.framebufferColorSampleCounts, physicalDeviceProperties.limits.framebufferDepthSampleCounts);
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

i32 frostwave::VkFramework::RateDeviceSuitability(VkPhysicalDevice aDevice)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(aDevice, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(aDevice, &deviceFeatures);

	i32 score = 0;
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 500;

	score += deviceProperties.limits.maxImageDimension2D;

	if (!deviceFeatures.geometryShader)
	{
		return 0;
	}

	return score;
}

frostwave::SwapChainSupportDetails frostwave::VkFramework::QuerySwapChainSupport(VkPhysicalDevice aDevice)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(aDevice, mySurface, &details.capabilities);

	u32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(aDevice, mySurface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(aDevice, mySurface, &formatCount, details.formats.data());
	}

	u32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(aDevice, mySurface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(aDevice, mySurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

frostwave::QueueFamilyIndices frostwave::VkFramework::FindQueueFamily(VkPhysicalDevice aDevice) const
{
	QueueFamilyIndices indices;

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(aDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(aDevice, &queueFamilyCount, queueFamilies.data());

	for (i32 i = 0; i < queueFamilies.size(); ++i)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(aDevice, i, mySurface, &presentSupport);

		if (queueFamilies[i].queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}
	}

	return indices;
}

bool frostwave::VkFramework::IsDeviceSuitable(VkPhysicalDevice aDevice)
{
	QueueFamilyIndices indices = FindQueueFamily(aDevice);
	bool extensionsSupported = CheckDeviceExtensionSupport(aDevice);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(aDevice);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(aDevice, &supportedFeatures);

	return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool frostwave::VkFramework::CheckDeviceExtensionSupport(VkPhysicalDevice aDevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(aDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(aDevice, nullptr, &extensionCount, availableExtensions.data());
	std::set<string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

VkSurfaceFormatKHR frostwave::VkFramework::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& aAvailableFormats)
{
	if (aAvailableFormats.size() == 1 && aAvailableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& available : aAvailableFormats)
	{
		if (available.format == VK_FORMAT_B8G8R8A8_UNORM && available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return available;
		}
	}

	return aAvailableFormats[0];
}

VkPresentModeKHR frostwave::VkFramework::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> aAvailablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	if (!mySettings.vsync)
	{
		for (const auto& availablePresentMode : aAvailablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				bestMode = availablePresentMode;
			}
		}
	}

	return bestMode;
}

VkExtent2D frostwave::VkFramework::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR & aCapabilities)
{
	if (aCapabilities.currentExtent.width != std::numeric_limits<u32>::max())
	{
		return aCapabilities.currentExtent;
	}
	else
	{
		i32 width, height;
		glfwGetFramebufferSize(myWindow, &width, &height);

		VkExtent2D actualExtent{ (u32)width, (u32)height };

		actualExtent.width = fw::Max(aCapabilities.minImageExtent.width, fw::Min(aCapabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = fw::Max(aCapabilities.minImageExtent.height, fw::Min(aCapabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkFormat frostwave::VkFramework::FindSupportedFormat(const std::vector<VkFormat>& aCandidates, VkImageTiling aTiling, VkFormatFeatureFlags aFeatures)
{
	for (VkFormat format : aCandidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(myPhysicalDevice, format, &props);

		if (aTiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & aFeatures) == aFeatures)
		{
			return format;
		}
		else if (aTiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & aFeatures) == aFeatures)
		{
			return format;
		}
	}

	FATAL_LOG("Failed to find supported format!");
	return VkFormat();
}

bool frostwave::VkFramework::PickPhysicalDevice()
{
	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices(myInstance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		FATAL_LOG("Failed to acquire a physical device with Vulkan support!");
		return false;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(myInstance, &deviceCount, devices.data());

	std::multimap<i32, VkPhysicalDevice> candidates;

	for (const auto& device : devices)
	{
		i32 score = RateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0)
	{
		auto& device = candidates.rbegin()->second;

		if (!IsDeviceSuitable(device))
		{
			FATAL_LOG("Failed to find a suitable physical device!");
			return false;
		}

		myPhysicalDevice = device;
		myMSAASamples = VK_SAMPLE_COUNT_1_BIT; //GetMaxUsableSampleCount();

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

		INFO_LOG("Picked physical device: %s with %lluMB VRAM", deviceProperties.deviceName, (u64)memoryProperties.memoryHeaps[0].size / 1024 / 1024);
	}
	else
	{
		FATAL_LOG("Failed to find a suitable physical device!");
		return false;
	}
	return true;
}

bool frostwave::VkFramework::SetupDebugMessenger()
{
	if (!mySettings.validation) return true;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	if (mySettings.validation & GraphicsSettings::Verbose)	createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	if (mySettings.validation & GraphicsSettings::Info)		createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	if (mySettings.validation & GraphicsSettings::Warning)	createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	if (mySettings.validation & GraphicsSettings::Error)	createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	if (mySettings.validationType & GraphicsSettings::General)		createInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
	if (mySettings.validationType & GraphicsSettings::Validation)	createInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	if (mySettings.validationType & GraphicsSettings::Performance)	createInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;

	VkResult result;
	result = CreateDebugUtilsMessengerEXT(myInstance, &createInfo, nullptr, &myDebugMessenger);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to set up debug messenger!");
		return false;
	}
	return true;
}
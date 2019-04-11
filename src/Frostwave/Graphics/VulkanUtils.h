#pragma once

#include <Frostwave/Core/Types.h>
#include <Frostwave/Debug/Logger.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace frostwave
{
	class VkFramework;

	u32 FindMemoryType(VkPhysicalDevice aPhysicalDevice, u32 aTypeFilter, VkMemoryPropertyFlags aProperties);

	bool HasStencilComponent(VkFormat aFormat);

	VkCommandBuffer BeginSingleTimeCommands(const VkFramework* aFramework);

	void EndSingleTimeCommands(VkCommandBuffer aCommandBuffer, const VkFramework* aFramework);

	void TransitionImageLayout(VkCommandBuffer aBuffer, VkImage aImage, VkImageLayout aOldLayout, VkImageLayout aNewLayout, VkImageSubresourceRange aSubresourceRange, 
		VkPipelineStageFlags aSrcFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags aDstFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	VkPipelineShaderStageCreateInfo LoadShader(const string& aPath, VkShaderStageFlagBits aStage, VkFramework* aFramework);

	std::vector<char> ReadFile(const string& aFilename);
}
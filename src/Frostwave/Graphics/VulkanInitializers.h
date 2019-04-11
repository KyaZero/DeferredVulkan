#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace frostwave::initializers
{
	inline VkViewport Viewport( float width,float height,float minDepth,float maxDepth)
	{
		VkViewport viewport{};
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;
		return viewport;
	}

	inline VkRect2D Rect2D(int32_t width, int32_t height, int32_t offsetX, int32_t offsetY)
	{
		VkRect2D rect2D{};
		rect2D.extent.width = width;
		rect2D.extent.height = height;
		rect2D.offset.x = offsetX;
		rect2D.offset.y = offsetY;
		return rect2D;
	}

	inline VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType aType, VkShaderStageFlags aStageFlags, uint32_t aBinding, uint32_t aDescriptorCount = 1)
	{
		VkDescriptorSetLayoutBinding desc{};
		desc.descriptorType = aType;
		desc.stageFlags = aStageFlags;
		desc.binding = aBinding;
		desc.descriptorCount = aDescriptorCount;
		return desc;
	}	
	inline VkDescriptorPoolSize DescriptorPoolSize(VkDescriptorType aType, uint32_t aDescriptorCount)
	{
		VkDescriptorPoolSize desc{};
		desc.type = aType;
		desc.descriptorCount = aDescriptorCount;
		return desc;
	}

	inline VkVertexInputBindingDescription VertexInputBindingDescription(u32 aBinding, u32 aStride, VkVertexInputRate aInputRate)
	{
		VkVertexInputBindingDescription desc{};
		desc.binding = aBinding;
		desc.stride = aStride;
		desc.inputRate = aInputRate;
		return desc;
	}

	inline VkVertexInputAttributeDescription VertexInputAttributeDescription(u32 aBinding, u32 aLocation, VkFormat aFormat, u32 aOffset)
	{
		VkVertexInputAttributeDescription desc{};
		desc.binding = aBinding;
		desc.location = aLocation;
		desc.format = aFormat;
		desc.offset = aOffset;
		return desc;
	}

	inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet aDstSet, VkDescriptorType aType, u32 aBinding, VkDescriptorBufferInfo* aBufferInfo, u32 aDescriptorCount = 1)
	{
		VkWriteDescriptorSet desc{};
		desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		desc.dstSet = aDstSet;
		desc.descriptorType = aType;
		desc.dstBinding = aBinding;
		desc.pBufferInfo = aBufferInfo;
		desc.descriptorCount = aDescriptorCount;
		return desc;
	}

	inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet aDstSet, VkDescriptorType aType,	u32 aBinding, VkDescriptorImageInfo* aImageInfo, u32 aDescriptorCount = 1)
	{
		VkWriteDescriptorSet desc{};
		desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		desc.dstSet = aDstSet;
		desc.descriptorType = aType;
		desc.dstBinding = aBinding;
		desc.pImageInfo = aImageInfo;
		desc.descriptorCount = aDescriptorCount;
		return desc;
	}

	inline VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState(VkColorComponentFlags aColorWriteMask, VkBool32 aBlendEnable)
	{
		VkPipelineColorBlendAttachmentState desc{};
		desc.colorWriteMask = aColorWriteMask;
		desc.blendEnable = aBlendEnable;
		return desc;
	}
}
namespace fw = frostwave;
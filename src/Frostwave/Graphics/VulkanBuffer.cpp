#include "stdafx.h"
#include "VulkanBuffer.h"

#include "VkFramework.h"

VkResult frostwave::CreateBuffer(const VkFramework* aFramework, VkBufferUsageFlags aUsageFlags, VkMemoryPropertyFlags aMemoryPropertyFlags, fw::Buffer * aBuffer, VkDeviceSize aSize, void * data)

{
	aBuffer->device = aFramework->GetDevice();

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = aUsageFlags;
	bufferCreateInfo.size = aSize;
	VkResult result = vkCreateBuffer(aFramework->GetDevice(), &bufferCreateInfo, nullptr, &aBuffer->buffer);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create buffer");
	}

	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkGetBufferMemoryRequirements(aFramework->GetDevice(), aBuffer->buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = FindMemoryType(aFramework->GetPhysicalDevice(), memReqs.memoryTypeBits, aMemoryPropertyFlags);
	result = vkAllocateMemory(aFramework->GetDevice(), &memAlloc, nullptr, &aBuffer->memory);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate memory for buffer!");
	}

	aBuffer->alignment = memReqs.alignment;
	aBuffer->size = memAlloc.allocationSize;
	aBuffer->usageFlags = aUsageFlags;
	aBuffer->memoryPropertyFlags = aMemoryPropertyFlags;

	if (data != nullptr)
	{
		result = aBuffer->Map();
		if (result != VK_SUCCESS)
		{
			FATAL_LOG("Failed to map buffer!");
		}
		memcpy(aBuffer->mapped, data, aSize);
		if ((aMemoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			aBuffer->Flush();

		aBuffer->Unmap();
	}

	aBuffer->SetupDescriptor();

	return aBuffer->Bind();
}

void frostwave::CopyBuffer(const VkFramework * aFramework, fw::Buffer * aSrc, fw::Buffer * aDst, VkBufferCopy * aCopyRegion)
{
	assert(aDst->size <= aSrc->size);
	assert(aSrc->buffer);
	VkCommandBuffer copyCmd = BeginSingleTimeCommands(aFramework);
	VkBufferCopy bufferCopy{};
	if (aCopyRegion == nullptr)
	{
		bufferCopy.size = aSrc->size;
	}
	else
	{
		bufferCopy = *aCopyRegion;
	}

	vkCmdCopyBuffer(copyCmd, aSrc->buffer, aDst->buffer, 1, &bufferCopy);

	EndSingleTimeCommands(copyCmd, aFramework);
}

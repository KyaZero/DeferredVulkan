#pragma once

#include <vulkan/vulkan.h>

namespace frostwave
{
	class VkFramework;
	struct Buffer
	{
		VkDevice device;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;
		VkDeviceSize size = 0;
		VkDeviceSize alignment = 0;
		void* mapped = nullptr;

		VkBufferUsageFlags usageFlags;
		VkMemoryPropertyFlags memoryPropertyFlags;

		VkResult Map(VkDeviceSize aSize = VK_WHOLE_SIZE, VkDeviceSize aOffset = 0)
		{
			return vkMapMemory(device, memory, aOffset, aSize, 0, &mapped);
		}

		void Unmap()
		{
			if (mapped)
			{
				vkUnmapMemory(device, memory);
				mapped = nullptr;
			}
		}

		VkResult Bind(VkDeviceSize aOffset = 0)
		{
			return vkBindBufferMemory(device, buffer, memory, aOffset);
		}

		void SetupDescriptor(VkDeviceSize aSize = VK_WHOLE_SIZE, VkDeviceSize aOffset = 0)
		{
			descriptor.offset = aOffset;
			descriptor.buffer = buffer;
			descriptor.range = aSize;
		}

		void CopyTo(void* aData, VkDeviceSize aSize)
		{
			assert(mapped);
			memcpy(mapped, aData, aSize);
		}

		VkResult Flush(VkDeviceSize aSize = VK_WHOLE_SIZE, VkDeviceSize aOffset = 0)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = memory;
			mappedRange.offset = aOffset;
			mappedRange.size = aSize;
			return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
		}

		VkResult Invalidate(VkDeviceSize aSize = VK_WHOLE_SIZE, VkDeviceSize aOffset = 0)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = memory;
			mappedRange.offset = aOffset;
			mappedRange.size = aSize;
			return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
		}

		void Destroy()
		{
			if (buffer) vkDestroyBuffer(device, buffer, nullptr);
			if (memory) vkFreeMemory(device, memory, nullptr);
		}
	};

	VkResult CreateBuffer(const VkFramework* aFramework, VkBufferUsageFlags aUsageFlags, VkMemoryPropertyFlags aMemoryPropertyFlags, Buffer* aBuffer, VkDeviceSize aSize, void *data = nullptr);

	void CopyBuffer(const VkFramework* aFramework, Buffer* aSrc, Buffer* aDst, VkBufferCopy* aCopyRegion = nullptr);
}
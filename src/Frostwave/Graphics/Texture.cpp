#include "stdafx.h"
#include "Texture.h"

#include "VkFramework.h"

#include <Frostwave/Core/FileManip.h>
#include "VulkanUtils.h"

#include <gli/gli.hpp>

void frostwave::Texture::UpdateDescriptor()
{
	myDescriptor.sampler = mySampler;
	myDescriptor.imageView = myView;
	myDescriptor.imageLayout = myImageLayout;
}

void frostwave::Texture::Destroy()
{
	vkDestroyImageView(myFramework->GetDevice(), myView, nullptr);
	vkDestroyImage(myFramework->GetDevice(), myImage, nullptr);
	if (mySampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(myFramework->GetDevice(), mySampler, nullptr);
	}
	vkFreeMemory(myFramework->GetDevice(), myDeviceMemory, nullptr);
}

VkImageView frostwave::Texture::GetView()
{
	return myView;
}

VkImage frostwave::Texture::GetImage()
{
	return myImage;
}

VkSampler frostwave::Texture::GetSampler()
{
	return mySampler;
}

VkDescriptorImageInfo frostwave::Texture::GetInfo()
{
	return myDescriptor;
}

void frostwave::Texture2D::Load(const string& aFilename, VkFormat aFormat, const VkFramework* aFramework, VkQueue aCopyQueue, VkImageUsageFlags aImageUsageFlags, VkImageLayout aImageLayout)
{
	if (!fs::exists(aFilename))
	{
		FATAL_LOG("Could not load texture '%s'", aFilename.c_str());
	}
	gli::texture2d tex2D(gli::load(aFilename.c_str()));
	assert(!tex2D.empty());

	myFramework = aFramework;
	myWidth = (u32)tex2D[0].extent().x;
	myHeight = (u32)tex2D[0].extent().y;
	myMipLevels = (u32)tex2D.levels();

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(myFramework->GetPhysicalDevice(), aFormat, &formatProperties);

	VkMemoryAllocateInfo memAllocInfo = { };
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	VkCommandBuffer copyCmd = BeginSingleTimeCommands(myFramework);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCreateInfo = { };
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = tex2D.size();
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(myFramework->GetDevice(), &bufferCreateInfo, nullptr, &stagingBuffer);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to begin command buffer!");
	}

	vkGetBufferMemoryRequirements(myFramework->GetDevice(), stagingBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = FindMemoryType(myFramework->GetPhysicalDevice(), memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	result = vkAllocateMemory(myFramework->GetDevice(), &memAllocInfo, nullptr, &stagingMemory);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate memory for staging buffer!");
	}
	result = vkBindBufferMemory(myFramework->GetDevice(), stagingBuffer, stagingMemory, 0);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to bind buffer memory");
	}

	void* data;
	result = vkMapMemory(myFramework->GetDevice(), stagingMemory, 0, memReqs.size, 0, &data);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to map memory for staging buffer!");
	}
	memcpy(data, tex2D.data(), tex2D.size());
	vkUnmapMemory(myFramework->GetDevice(), stagingMemory);

	std::vector<VkBufferImageCopy> copyRegions;
	u32 offset = 0;

	for (u32 i = 0; i < myMipLevels; ++i)
	{
		VkBufferImageCopy copyRegion = { };
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = i;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent.width = (u32)tex2D[i].extent().x;
		copyRegion.imageExtent.height = (u32)tex2D[i].extent().y;
		copyRegion.imageExtent.depth = 1;
		copyRegion.bufferOffset = offset;

		copyRegions.push_back(copyRegion);

		offset += (u32)tex2D[i].size();
	}

	VkImageCreateInfo imageCreateInfo = { };
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.format = aFormat;
	imageCreateInfo.mipLevels = myMipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { myWidth, myHeight, 1 };
	imageCreateInfo.usage = aImageUsageFlags;
	if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	result = vkCreateImage(myFramework->GetDevice(), &imageCreateInfo, nullptr, &myImage);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create image!");
	}

	vkGetImageMemoryRequirements(myFramework->GetDevice(), myImage, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = FindMemoryType(myFramework->GetPhysicalDevice(), memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	result = vkAllocateMemory(myFramework->GetDevice(), &memAllocInfo, nullptr, &myDeviceMemory);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate memory for buffer!");
	}

	result = vkBindImageMemory(myFramework->GetDevice(), myImage, myDeviceMemory, 0);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to bind image memory!");
	}

	VkImageSubresourceRange subresourceRange = { };
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = myMipLevels;
	subresourceRange.layerCount = 1;

	myImageLayout = aImageLayout;
	TransitionImageLayout(copyCmd, myImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
	vkCmdCopyBufferToImage(copyCmd, stagingBuffer, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (u32)copyRegions.size(), copyRegions.data());
	TransitionImageLayout(copyCmd, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, myImageLayout, subresourceRange);

	EndSingleTimeCommands(copyCmd, myFramework);
	vkDestroyBuffer(myFramework->GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(myFramework->GetDevice(), stagingMemory, nullptr);

	VkSamplerCreateInfo samplerCreateInfo = { };
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = (f32)myMipLevels;
	samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.anisotropyEnable = true;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	result = vkCreateSampler(myFramework->GetDevice(), &samplerCreateInfo, nullptr, &mySampler);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create sampler for texture!");
	}

	VkImageViewCreateInfo viewCreateInfo = { };
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = aFormat;
	viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewCreateInfo.subresourceRange.levelCount = myMipLevels;
	viewCreateInfo.image = myImage;
	result = vkCreateImageView(myFramework->GetDevice(), &viewCreateInfo, nullptr, &myView);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create image view for texture!");
	}

	UpdateDescriptor();
}

void frostwave::Texture2D::FromBuffer(void* aBuffer, VkDeviceSize aBufferSize, VkFormat aFormat, u32 aTexWidth, u32 aTexHeight, const VkFramework* aFramework, VkQueue aCopyQueue, VkFilter aFilter, VkImageUsageFlags aImageUsageFlags, VkImageLayout aImageLayout)
{
	assert(aBuffer);

	myFramework = aFramework;
	myWidth = aTexWidth;
	myHeight = aTexHeight;
	myMipLevels = 1;

	VkMemoryAllocateInfo memAllocInfo = { };
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	VkCommandBuffer copyCmd = BeginSingleTimeCommands(myFramework);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCreateInfo = { };
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = aBufferSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(myFramework->GetDevice(), &bufferCreateInfo, nullptr, &stagingBuffer);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to begin command buffer!");
	}

	vkGetBufferMemoryRequirements(myFramework->GetDevice(), stagingBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = FindMemoryType(myFramework->GetPhysicalDevice(), memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	result = vkAllocateMemory(myFramework->GetDevice(), &memAllocInfo, nullptr, &stagingMemory);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate memory for staging buffer!");
	}
	result = vkBindBufferMemory(myFramework->GetDevice(), stagingBuffer, stagingMemory, 0);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to bind buffer memory");
	}

	void* data;
	result = vkMapMemory(myFramework->GetDevice(), stagingMemory, 0, memReqs.size, 0, &data);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to map memory for staging buffer!");
	}
	memcpy(data, aBuffer, aBufferSize);
	vkUnmapMemory(myFramework->GetDevice(), stagingMemory);

	VkBufferImageCopy copyRegion = {};
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageExtent.width = myWidth;
	copyRegion.imageExtent.height = myHeight;
	copyRegion.imageExtent.depth = 1;
	copyRegion.bufferOffset = 0;

	VkImageCreateInfo imageCreateInfo = { };
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = aFormat;
	imageCreateInfo.mipLevels = myMipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { myWidth, myHeight, 1 };
	imageCreateInfo.usage = aImageUsageFlags;
	if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	result = vkCreateImage(myFramework->GetDevice(), &imageCreateInfo, nullptr, &myImage);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create image!");
	}

	vkGetImageMemoryRequirements(myFramework->GetDevice(), myImage, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = FindMemoryType(myFramework->GetPhysicalDevice(), memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	result = vkAllocateMemory(myFramework->GetDevice(), &memAllocInfo, nullptr, &myDeviceMemory);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate memory for buffer!");
	}

	result = vkBindImageMemory(myFramework->GetDevice(), myImage, myDeviceMemory, 0);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to bind image memory!");
	}

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = myMipLevels;
	subresourceRange.layerCount = 1;

	myImageLayout = aImageLayout;
	TransitionImageLayout(copyCmd, myImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
	vkCmdCopyBufferToImage(copyCmd, stagingBuffer, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
	TransitionImageLayout(copyCmd, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, myImageLayout, subresourceRange);

	EndSingleTimeCommands(copyCmd, myFramework);
	vkDestroyBuffer(myFramework->GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(myFramework->GetDevice(), stagingMemory, nullptr);

	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = aFilter;
	samplerCreateInfo.minFilter = aFilter;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	result = vkCreateSampler(myFramework->GetDevice(), &samplerCreateInfo, nullptr, &mySampler);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create sampler for texture!");
	}

	VkImageViewCreateInfo viewCreateInfo = { };
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = aFormat;
	viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.image = myImage;
	result = vkCreateImageView(myFramework->GetDevice(), &viewCreateInfo, nullptr, &myView);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create image view for texture!");
	}

	UpdateDescriptor();
}
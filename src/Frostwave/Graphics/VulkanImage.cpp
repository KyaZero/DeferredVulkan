#include "stdafx.h"
#include "VulkanImage.h"
#include "VulkanUtils.h"
#include "VkFramework.h"

#include <Frostwave/Debug/Logger.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

frostwave::VulkanImage::VulkanImage() :
	myImage(VK_NULL_HANDLE), myImageView(VK_NULL_HANDLE),
	myImageMemory(VK_NULL_HANDLE), mySampler(VK_NULL_HANDLE), 
	myFramework(nullptr), myMipLevels(1)
{
}

frostwave::VulkanImage::VulkanImage(const VkFramework* aFramework, const ImageCreateInfo& aCreateInfo)
	: myImage(VK_NULL_HANDLE), myImageView(VK_NULL_HANDLE), myImageMemory(VK_NULL_HANDLE), mySampler(VK_NULL_HANDLE), myMipLevels(1)
{
	Create(myFramework, aCreateInfo);
}

frostwave::VulkanImage::~VulkanImage()
{
	//Destroy();
}

frostwave::VulkanImage::VulkanImage(const VulkanImage& aOther)
{
	operator=(aOther);
}

frostwave::VulkanImage& frostwave::VulkanImage::operator=(const VulkanImage& aOther)
{
	myExtent = aOther.myExtent;
	myFormat = aOther.myFormat;
	myImage = aOther.myImage;
	myImageView = aOther.myImageView;
	myImageMemory = aOther.myImageMemory;
	myType = aOther.myType;
	myFilePath = aOther.myFilePath;
	mySampler = aOther.mySampler;
	myFramework = aOther.myFramework;

	return *this;
}

frostwave::VulkanImage::VulkanImage(VulkanImage&& aOther)
{
	operator=(aOther);
}

frostwave::VulkanImage& frostwave::VulkanImage::operator=(VulkanImage&& aOther)
{
	myExtent = aOther.myExtent;
	myFormat = aOther.myFormat;
	myImage = aOther.myImage;
	myImageView = aOther.myImageView;
	myImageMemory = aOther.myImageMemory;
	myType = aOther.myType;
	myFilePath = aOther.myFilePath;
	mySampler = aOther.mySampler;
	myFramework = aOther.myFramework;

	aOther.myImageView = VK_NULL_HANDLE;
	aOther.myImage = VK_NULL_HANDLE;
	aOther.myImageMemory = VK_NULL_HANDLE;
	aOther.mySampler = VK_NULL_HANDLE;

	return *this;
}

VkImage frostwave::VulkanImage::GetImage()
{
	return myImage;
}

VkImageView frostwave::VulkanImage::GetImageView()
{
	return myImageView;
}

VkSampler frostwave::VulkanImage::GetSampler()
{
	return mySampler;
}

void frostwave::VulkanImage::Create(const VkFramework* aFramework, const ImageCreateInfo& aCreateInfo)
{
	if (myImage != VK_NULL_HANDLE || myImageView != VK_NULL_HANDLE || myImageMemory != VK_NULL_HANDLE || mySampler != VK_NULL_HANDLE) Destroy();

	myFramework = aFramework;
	myType = aCreateInfo.type;
	myExtent = { aCreateInfo.width, aCreateInfo.height };
	myMSAASamples = aCreateInfo.samples;
	myFilePath = aCreateInfo.path;
	myFormat = aCreateInfo.format;

	switch (myType)
	{
	case frostwave::ImageType::Depth:
	{
		CreateImage();
		CreateImageView();
		VkImageSubresourceRange range = { };
		range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		range.baseMipLevel = 0;
		range.layerCount = 1;
		range.levelCount = myMipLevels;
		auto cmd = BeginSingleTimeCommands(myFramework);
		TransitionImageLayout(cmd, myImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, range);
		EndSingleTimeCommands(cmd, myFramework);
	} break;
	case frostwave::ImageType::Texture:
	{
		if (myFilePath.length() <= 0)
		{
			FATAL_LOG("Cannot create texture without valid file path!");
		}

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(myFilePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		myExtent.width = texWidth;
		myExtent.height = texHeight;

		myMipLevels = (u32)std::floor(std::log2(fw::Max(texWidth, texHeight))) + 1;

		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
		{
			FATAL_LOG("Failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(myFramework->GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, (size_t)imageSize);
		vkUnmapMemory(myFramework->GetDevice(), stagingBufferMemory);

		stbi_image_free(pixels);

		CreateImage();

		VkImageSubresourceRange range = { };
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = 1;
		range.levelCount = myMipLevels;

		auto cmd = BeginSingleTimeCommands(myFramework);
		TransitionImageLayout(cmd, myImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range);
		EndSingleTimeCommands(cmd, myFramework);
		CopyBufferToImage(stagingBuffer, myImage, (u32)texWidth, (u32)texHeight);

		if (!aCreateInfo.generateMips)
		{
			myMipLevels = 1;
			VkImageSubresourceRange range = { };
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseArrayLayer = 0;
			range.baseMipLevel = 0;
			range.layerCount = 1;
			range.levelCount = myMipLevels;
			auto cmd = BeginSingleTimeCommands(myFramework);
			TransitionImageLayout(cmd, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range);
			EndSingleTimeCommands(cmd, myFramework);
		}
		else
		{
			GenerateMipmaps();
		}

		vkDestroyBuffer(myFramework->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(myFramework->GetDevice(), stagingBufferMemory, nullptr);

		CreateImageView();
		CreateImageSampler();
	} break;
	case frostwave::ImageType::ColorAttachment:
		CreateImage();
		CreateImageView();
		CreateImageSampler();
		break;
	default:
		break;
	}
}

void frostwave::VulkanImage::Destroy()
{
	VkDevice device = myFramework->GetDevice();

	if (mySampler != VK_NULL_HANDLE) { vkDestroySampler(device, mySampler, nullptr); mySampler = VK_NULL_HANDLE; }
	if (myImageView != VK_NULL_HANDLE) { vkDestroyImageView(device, myImageView, nullptr); myImageView = VK_NULL_HANDLE; }
	if (myImage != VK_NULL_HANDLE) { vkDestroyImage(device, myImage, nullptr); myImage = VK_NULL_HANDLE; }
	if (myImageMemory != VK_NULL_HANDLE) { vkFreeMemory(device, myImageMemory, nullptr); myImageMemory = VK_NULL_HANDLE; }
}

void frostwave::VulkanImage::CreateImage()
{
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	VkMemoryPropertyFlags properties;
	switch (myType)
	{
	case frostwave::ImageType::Depth:
		tiling = VK_IMAGE_TILING_OPTIMAL;
		usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	case frostwave::ImageType::Texture:
		tiling = VK_IMAGE_TILING_OPTIMAL;
		usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	case frostwave::ImageType::ColorAttachment:
		tiling = VK_IMAGE_TILING_OPTIMAL;
		usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	}

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = myExtent.width;
	imageInfo.extent.height = myExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = myMipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = myFormat;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = myMSAASamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkDevice device = myFramework->GetDevice();

	VkResult result = vkCreateImage(device, &imageInfo, nullptr, &myImage);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, myImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(myFramework->GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

	result = vkAllocateMemory(device, &allocInfo, nullptr, &myImageMemory);

	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate image memory!");
	}

	vkBindImageMemory(device, myImage, myImageMemory, 0);
}

void frostwave::VulkanImage::CreateImageView()
{
	VkImageAspectFlags aspectFlags;
	switch (myType)
	{
	case frostwave::ImageType::Depth:
		if (HasStencilComponent(myFormat))
		{
			aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
		{
			aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		break;
	case frostwave::ImageType::Texture:
	case frostwave::ImageType::ColorAttachment:
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		break;
	}

	VkImageViewCreateInfo viewInfo = { };
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = myImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = myFormat;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = myMipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(myFramework->GetDevice(), &viewInfo, nullptr, &myImageView);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create image view!");
	}
}

void frostwave::VulkanImage::CreateImageSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.maxLod = (f32)myMipLevels;

	switch (myType)
	{
	case frostwave::ImageType::ColorAttachment:
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_FALSE;
		break;
	case frostwave::ImageType::Texture:
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 4;
		break;
	default:
		FATAL_LOG("Tried to create sampler for image of unknown format!");
		break;
	}

	VkResult result = vkCreateSampler(myFramework->GetDevice(), &samplerInfo, nullptr, &mySampler);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create texture sampler!");
	}
}

void frostwave::VulkanImage::GenerateMipmaps()
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(myFramework->GetPhysicalDevice(), myFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		FATAL_LOG("Texture image does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(myFramework);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = myImage;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	i32 mipWidth = myExtent.width;
	i32 mipHeight = myExtent.height;

	for (u32 i = 1; i < myMipLevels; ++i)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer, myImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = myMipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	EndSingleTimeCommands(commandBuffer, myFramework);
}

void frostwave::VulkanImage::CopyBufferToImage(VkBuffer aBuffer, VkImage aImage, u32 aWidth, u32 aHeight)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(myFramework);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0,0,0 };
	region.imageExtent = { aWidth, aHeight, 1 };

	vkCmdCopyBufferToImage(commandBuffer, aBuffer, aImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	EndSingleTimeCommands(commandBuffer, myFramework);
}

void frostwave::VulkanImage::CopyBuffer(VkBuffer aSourceBuffer, VkBuffer aDestinationBuffer, VkDeviceSize aSize)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(myFramework);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = aSize;
	vkCmdCopyBuffer(commandBuffer, aSourceBuffer, aDestinationBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer, myFramework);
}

void frostwave::VulkanImage::CreateBuffer(VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags aProperties, VkBuffer& aBuffer, VkDeviceMemory& aBufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = aSize;
	bufferInfo.usage = aUsage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(myFramework->GetDevice(), &bufferInfo, nullptr, &aBuffer);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(myFramework->GetDevice(), aBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(myFramework->GetPhysicalDevice(), memRequirements.memoryTypeBits, aProperties);

	result = vkAllocateMemory(myFramework->GetDevice(), &allocInfo, nullptr, &aBufferMemory);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(myFramework->GetDevice(), aBuffer, aBufferMemory, 0);

}


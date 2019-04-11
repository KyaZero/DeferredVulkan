#pragma once

#include <vulkan/vulkan.h>

#include <Frostwave/Core/Types.h>

namespace frostwave
{
	class VkFramework;
	class Texture
	{
	public:
		void Destroy();

		VkImageView GetView();
		VkImage GetImage();
		VkSampler GetSampler();
		VkDescriptorImageInfo GetInfo();

	protected:
		void UpdateDescriptor();

		const VkFramework* myFramework;
		VkImage myImage;
		VkImageLayout myImageLayout;
		VkDeviceMemory myDeviceMemory;
		VkImageView myView;
		u32 myWidth, myHeight;
		u32 myMipLevels;
		u32 myLayerCount;
		VkDescriptorImageInfo myDescriptor;
		
		VkSampler mySampler;
	};

	class Texture2D : public Texture
	{
	public:
		void Load(const string& aFilename, VkFormat aFormat, const VkFramework* aFramework, VkQueue aCopyQueue, 
			VkImageUsageFlags aImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout aImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		void FromBuffer(void* aBuffer, VkDeviceSize aBufferSize, VkFormat aFormat, u32 aTexWidth, u32 aTexHeight, const VkFramework* aFramework, VkQueue aCopyQueue,
			VkFilter aFilter = VK_FILTER_LINEAR, VkImageUsageFlags aImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout aImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};
}
namespace fw = frostwave;
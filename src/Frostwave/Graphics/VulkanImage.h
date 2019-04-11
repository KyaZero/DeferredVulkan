#pragma once

#include <Frostwave/Core/Types.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace frostwave
{
	class VkFramework;

	enum class ImageType
	{
		Depth,
		Texture,
		ColorAttachment
	};

	struct ImageCreateInfo
	{
		ImageType type;
		u32 width;
		u32 height;
		VkFormat format;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		string path = "";
		string normalPath = "";
		string materialPath = "";
		bool generateMips = true;
	};

	class VulkanImage
	{
	public:
		VulkanImage();
		VulkanImage(const VkFramework* aFramework, const ImageCreateInfo& aCreateInfo);
		~VulkanImage();

		VulkanImage(const VulkanImage& aOther);
		VulkanImage& operator=(const VulkanImage& aOther);
		VulkanImage(VulkanImage&& aOther);
		VulkanImage& operator=(VulkanImage&& aOther);

		VkImage GetImage();
		VkImageView GetImageView();
		VkSampler GetSampler();

		void Create(const VkFramework* aFramework, const ImageCreateInfo& aCreateInfo);
		void Destroy();

	private:
		void CreateImage();
		void CreateImageView();
		void CreateImageSampler();

		void GenerateMipmaps();

		void CopyBufferToImage(VkBuffer aBuffer, VkImage aImage, u32 aWidth, u32 aHeight);
		void CopyBuffer(VkBuffer aSourceBuffer, VkBuffer aDestinationBuffer, VkDeviceSize aSize);
		void CreateBuffer(VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags aProperties, VkBuffer& aBuffer, VkDeviceMemory& aBufferMemory);

		VkExtent2D myExtent;
		VkFormat myFormat;
		VkImage myImage;
		VkImageView myImageView;
		VkDeviceMemory myImageMemory;
		ImageType myType;

		string myFilePath;
		VkSampler mySampler;

		const VkFramework* myFramework;

		VkSampleCountFlagBits myMSAASamples;

		u32 myMipLevels;
	};
}
namespace fw = frostwave;
#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <Frostwave/Core/Types.h>
#include <Frostwave/Debug/Logger.h>
#include <Frostwave/Core/Common.h>
#include <Frostwave/Core/Math/Vector.h>

#include <Frostwave/Graphics/VulkanImage.h>
#include <Frostwave/Graphics/VulkanBuffer.h>

#include <vector>

namespace frostwave
{
	class Renderer;

	typedef enum Component {
		VERTEX_COMPONENT_POSITION = 0x0,
		VERTEX_COMPONENT_NORMAL = 0x1,
		VERTEX_COMPONENT_COLOR = 0x2,
		VERTEX_COMPONENT_UV = 0x3,
		VERTEX_COMPONENT_TANGENT = 0x4,
		VERTEX_COMPONENT_BITANGENT = 0x5
	} Component;

	static constexpr int DefaultFlags = aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;

	struct VertexLayout
	{
		VertexLayout(std::vector<Component> aComponents) { components = std::move(aComponents); }

		u32 Stride()
		{
			u32 res = 0;
			for (auto& c : components)
			{
				switch (c)
				{
				case VERTEX_COMPONENT_UV: res += 2 * sizeof(f32); break;
				default: res += 3 * sizeof(f32); break;
				}
			}
			return res;
		}

		std::vector<Component> components;
	};

	struct ModelCreateInfo
	{
		fw::Vec3f center = 0;
		fw::Vec3f scale = 1;
		fw::Vec2f uvscale = 1;
	};

	class Mesh
	{
	public:
		struct Dimensions
		{
			fw::Vec3f min = FLT_MAX;
			fw::Vec3f max = -FLT_MAX;
			fw::Vec3f size;
		};
		struct ModelPart
		{
			u32 vertexBase;
			u32 vertexCount;
			u32 indexBase;
			u32 indexCount;
		};

	public:
		bool Load(const string& aFilename, VertexLayout aLayout, ModelCreateInfo* aCreateInfo, const VkFramework* aFramework, VkQueue aCopyQueue, const i32 aFlags = DefaultFlags);
		void Destroy();

	private:
		friend class Model;
		Dimensions myDimensions;
		std::vector<ModelPart> myParts;

		std::vector<f32> myVertices;
		std::vector<u32> myIndices;

		Buffer myVertexBuffer;
		Buffer myIndexBuffer;
		u32 myVertexCount = 0;
		u32 myIndexCount = 0;
	};

	class Model
	{
	public:
		Model() : myDiffuse(nullptr), myNormalMap(nullptr), myMaterial(nullptr) { }
		bool Load(const string& aFilename, Renderer* aRenderer, VertexLayout aLayout, ImageCreateInfo& aImageInfo, ModelCreateInfo* aCreateInfo, const VkFramework* aFramework, VkQueue aCopyQueue, const i32 aFlags = DefaultFlags);
		void Destroy();

		const Mesh::Dimensions& GetDimensions() const;

		std::vector<f32>& GetVertices();
		std::vector<u32>& GetIndices();

		Buffer& GetVertexBuffer();
		const Buffer& GetVertexBuffer() const;
		Buffer& GetIndexBuffer();
		const Buffer& GetIndexBuffer() const;

		void SetVertexCount(u32 aCount);
		u32 GetVertexCount() const;
		void SetIndexCount(u32 aCount);
		u32 GetIndexCount() const;

		const VkDescriptorSet& GetDescriptorSet() const;

	private:
		void SetupDescriptorSets();

		const VkFramework* myFramework;
		const Renderer* myRenderer;

		VkDescriptorSet myDescriptorSet;
		Buffer* myUBO;
		VulkanImage* myDiffuse;
		VulkanImage* myNormalMap;
		VulkanImage* myMaterial;
		Mesh myMesh;
	};
}
namespace fw = frostwave;
#include "stdafx.h"
#include "Model.h"
#include "VkFramework.h"
#include <Frostwave/Graphics/Renderer.h>

void frostwave::Mesh::Destroy()
{
	myVertexBuffer.Destroy();
	myIndexBuffer.Destroy();
}

bool frostwave::Mesh::Load(const string& aFilename, VertexLayout aLayout, ModelCreateInfo* aCreateInfo, const VkFramework* aFramework, VkQueue aCopyQueue, const i32 aFlags)
{
	Assimp::Importer importer;
	const aiScene* scene;
	scene = importer.ReadFile(aFilename, aFlags);
	if (!scene)
	{
		FATAL_LOG("Error parsing '%s' : '%s'", aFilename.c_str(), importer.GetErrorString());
		return false;
	}

	myParts.clear();
	myParts.resize(scene->mNumMeshes);

	fw::Vec3f scale(1.0f);
	fw::Vec2f uvscale(1.0f);
	fw::Vec3f center(0.0f);
	if (aCreateInfo)
	{
		scale = aCreateInfo->scale;
		center = aCreateInfo->center;
		uvscale = aCreateInfo->uvscale;
	}

	myIndexCount = 0;
	myVertexCount = 0;

	for (u32 i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];

		myParts[i] = { };
		myParts[i].vertexBase = myVertexCount;
		myParts[i].indexBase = myIndexCount;

		myVertexCount += mesh->mNumVertices;

		aiColor3D color(0.0f, 0.0f, 0.0f);
		scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);

		const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

		for (u32 j = 0; j < mesh->mNumVertices; ++j)
		{
			const aiVector3D* pos = &(mesh->mVertices[j]);
			const aiVector3D* normal = &(mesh->mNormals[j]);
			const aiVector3D* texCoord = (mesh->HasTextureCoords(0)) ? &(mesh->mTextureCoords[0][j]) : &Zero3D;
			const aiVector3D* tangent = (mesh->HasTangentsAndBitangents()) ? &(mesh->mTangents[j]) : &Zero3D;
			const aiVector3D* biTangent = (mesh->HasTangentsAndBitangents()) ? &(mesh->mBitangents[j]) : &Zero3D;

			for (auto& c : aLayout.components)
			{
				switch (c)
				{
				case frostwave::VERTEX_COMPONENT_POSITION:
					myVertices.push_back(pos->x * scale.x + center.x);
					myVertices.push_back(pos->y * scale.y + center.y);
					myVertices.push_back(pos->z * scale.z + center.z);
					break;
				case frostwave::VERTEX_COMPONENT_NORMAL:
					myVertices.push_back(normal->x);
					myVertices.push_back(normal->y);
					myVertices.push_back(normal->z);
					break;
				case frostwave::VERTEX_COMPONENT_UV:
					myVertices.push_back(texCoord->x * uvscale.x);
					myVertices.push_back(texCoord->y * uvscale.y);
					break;
				case frostwave::VERTEX_COMPONENT_COLOR:
					myVertices.push_back(color.r);
					myVertices.push_back(color.g);
					myVertices.push_back(color.b);
					break;
				case frostwave::VERTEX_COMPONENT_TANGENT:
					myVertices.push_back(tangent->x);
					myVertices.push_back(tangent->y);
					myVertices.push_back(tangent->z);
					break;
				case frostwave::VERTEX_COMPONENT_BITANGENT:
					myVertices.push_back(biTangent->x);
					myVertices.push_back(biTangent->y);
					myVertices.push_back(biTangent->z);
					break;
				}
			}

			myDimensions.max.x = fw::Max(pos->x, myDimensions.max.x);
			myDimensions.max.y = fw::Max(pos->y, myDimensions.max.y);
			myDimensions.max.z = fw::Max(pos->z, myDimensions.max.z);

			myDimensions.min.x = fw::Min(pos->x, myDimensions.min.x);
			myDimensions.min.y = fw::Min(pos->y, myDimensions.min.y);
			myDimensions.min.z = fw::Min(pos->z, myDimensions.min.z);
		}

		myDimensions.size = myDimensions.max - myDimensions.min;

		myParts[i].vertexCount = mesh->mNumVertices;

		u32 indexBase = (u32)myIndices.size();
		for (u32 j = 0; j < mesh->mNumFaces; ++j)
		{
			const aiFace& Face = mesh->mFaces[j];
			if (Face.mNumIndices != 3)
			{
				continue;
			}
			myIndices.push_back(indexBase + Face.mIndices[0]);
			myIndices.push_back(indexBase + Face.mIndices[1]);
			myIndices.push_back(indexBase + Face.mIndices[2]);
			myParts[i].indexCount += 3;
			myIndexCount += 3;
		}
	}

	u32 vBufferSize = (u32)myVertices.size() * sizeof(f32);
	u32 iBufferSize = (u32)myIndices.size() * sizeof(u32);

	fw::Buffer vertexStaging, indexStaging;

	CreateBuffer(aFramework,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&vertexStaging, vBufferSize, myVertices.data()
	);

	CreateBuffer(aFramework,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&indexStaging, iBufferSize, myIndices.data()
	);

	CreateBuffer(aFramework,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&myVertexBuffer, vBufferSize
	);

	CreateBuffer(aFramework,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&myIndexBuffer, iBufferSize
	);

	VkCommandBuffer copyCmd = BeginSingleTimeCommands(aFramework);

	VkBufferCopy copyRegion = {};
	copyRegion.size = myVertexBuffer.size;
	vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, myVertexBuffer.buffer, 1, &copyRegion);

	copyRegion.size = myIndexBuffer.size;
	vkCmdCopyBuffer(copyCmd, indexStaging.buffer, myIndexBuffer.buffer, 1, &copyRegion);

	EndSingleTimeCommands(copyCmd, aFramework);

	vertexStaging.Destroy();
	indexStaging.Destroy();

	return true;
}

bool frostwave::Model::Load(const string& aFilename, Renderer* aRenderer, VertexLayout aLayout, ImageCreateInfo& aImageInfo, ModelCreateInfo* aCreateInfo, const VkFramework* aFramework, VkQueue aCopyQueue, const i32 aFlags)
{
	static std::unordered_map<string, Mesh> loadedMeshes;

	myFramework = aFramework;
	myRenderer = aRenderer;

	myDiffuse = new VulkanImage;
	myDiffuse->Create(myFramework, aImageInfo);

	aImageInfo.path = aImageInfo.normalPath;
	myNormalMap = new VulkanImage;
	myNormalMap->Create(myFramework, aImageInfo);

	aImageInfo.path = aImageInfo.materialPath;
	myMaterial = new VulkanImage;
	myMaterial->Create(myFramework, aImageInfo);

	if (loadedMeshes.find(aFilename) != loadedMeshes.end())
	{
		myMesh = loadedMeshes.at(aFilename);
	}
	else
	{
		auto res = myMesh.Load(aFilename, aLayout, aCreateInfo, aFramework, aCopyQueue, aFlags);
		if (!res) return false;
		loadedMeshes[aFilename] = myMesh;
	}

	myUBO = aRenderer->GetUBO();

	SetupDescriptorSets();

	return true;
}

void frostwave::Model::Destroy()
{
	myMesh.Destroy();
	if (myDiffuse) myDiffuse->Destroy();
	if (myNormalMap) myNormalMap->Destroy();
	if (myMaterial) myMaterial->Destroy();
}

const fw::Mesh::Dimensions& frostwave::Model::GetDimensions() const
{
	return myMesh.myDimensions;
}

std::vector<f32>& frostwave::Model::GetVertices()
{
	return myMesh.myVertices;
}

std::vector<u32>& frostwave::Model::GetIndices()
{
	return myMesh.myIndices;
}

frostwave::Buffer& frostwave::Model::GetVertexBuffer()
{
	return myMesh.myVertexBuffer;
}

const fw::Buffer& frostwave::Model::GetVertexBuffer() const
{
	return myMesh.myVertexBuffer;
}

frostwave::Buffer& frostwave::Model::GetIndexBuffer()
{
	return myMesh.myIndexBuffer;
}

const fw::Buffer& frostwave::Model::GetIndexBuffer() const
{
	return myMesh.myIndexBuffer;
}

void frostwave::Model::SetVertexCount(u32 aCount)
{
	myMesh.myVertexCount = aCount;
}

u32 frostwave::Model::GetVertexCount() const
{
	return myMesh.myVertexCount;
}

void frostwave::Model::SetIndexCount(u32 aCount)
{
	myMesh.myIndexCount = aCount;
}

u32 frostwave::Model::GetIndexCount() const
{
	return myMesh.myIndexCount;
}

const VkDescriptorSet& frostwave::Model::GetDescriptorSet() const
{
	return myDescriptorSet;
}

void frostwave::Model::SetupDescriptorSets()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = ((VkFramework*)myFramework)->GetDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &myRenderer->GetDescriptorSetLayout();

	VkResult result = vkAllocateDescriptorSets(myFramework->GetDevice(), &allocInfo, &myDescriptorSet);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate descriptor sets!");
		return;
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = myUBO->buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = myDiffuse->GetImageView();
	imageInfo.sampler = myDiffuse->GetSampler();

	VkDescriptorImageInfo normalMap = {};
	normalMap.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	normalMap.imageView = myNormalMap->GetImageView();
	normalMap.sampler = myNormalMap->GetSampler();

	VkDescriptorImageInfo material = {};
	material.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	material.imageView = myMaterial->GetImageView();
	material.sampler = myMaterial->GetSampler();

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo),
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &imageInfo),
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &normalMap),
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &material)
	};

	vkUpdateDescriptorSets(myFramework->GetDevice(), (u32)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}
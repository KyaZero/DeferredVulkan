#include "stdafx.h"
#include "Renderer.h"
#include "Model.h"
#include "VkFramework.h"
#include <Frostwave/Graphics/Model.h>
#include <Frostwave/Graphics/ModelInstance.h>
#include <Frostwave/Graphics/VulkanUtils.h>

frostwave::Renderer::Renderer() : myCommandPool(VK_NULL_HANDLE)
{
}

frostwave::Renderer::~Renderer()
{
}

void frostwave::Renderer::Init(VkFramework* aFramework)
{
	myFramework = aFramework;

	myCommandBuffers.resize(1);
	CreatePoolAndBuffers();

	myPipelines.forward = aFramework->GetPipeline();
	myPipelineLayouts.forward = aFramework->GetPipelineLayout();

	VkSemaphoreCreateInfo info = { };
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(myFramework->GetDevice(), &info, nullptr, &myOffscreenSemaphore);

	GenerateQuad();
	PrepareOffscreenFramebuffer();
	PrepareUniformBuffers();
	SetupDescriptorSetLayout();
	PreparePipelines();
	SetupDescriptorPool();
	SetupDescriptorSet();
}

void frostwave::Renderer::Render(const std::vector<ModelInstance*>& aModels, const std::vector<PointLight>& aLights, fw::Camera* aCamera)
{
	if (aModels.size() > myCommandBuffers.size())
	{
		myCommandBuffers.resize(aModels.size());
		CreatePoolAndBuffers();
	}

	u32 idx = myFramework->BeginFrame();

	myTimer.Update();

	myUBO = { };
	myUBO.view = aCamera->GetView();
	myUBO.projection = aCamera->GetProjection();
	memcpy(myUniformBuffers.offscreen.mapped, &myUBO, sizeof(myUBO));

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = myOffscreenFramebuffer.renderPass;
	renderPassInfo.framebuffer = myOffscreenFramebuffer.frameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent.width = myOffscreenFramebuffer.width;
	renderPassInfo.renderArea.extent.height = myOffscreenFramebuffer.height;

	std::array<VkClearValue, 5> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[2].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[3].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[4].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = (u32)clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	auto inheritanceInfo = myFramework->BeginCommandBufferRecording(idx, renderPassInfo);

	for (u32 i = 0; i < (u32)aModels.size(); ++i)
	{
		ModelInstance* instance = aModels[i];
		const Model* model = aModels[i]->GetModel();

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceInfo;

		vkBeginCommandBuffer(myCommandBuffers[i], &beginInfo);

		vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myPipelines.offscreen);

		VkViewport viewport = fw::initializers::Viewport((float)myOffscreenFramebuffer.width, (float)myOffscreenFramebuffer.height, 0.0f, 1.0f);
		vkCmdSetViewport(myCommandBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = fw::initializers::Rect2D(myOffscreenFramebuffer.width, myOffscreenFramebuffer.height, 0, 0);
		vkCmdSetScissor(myCommandBuffers[i], 0, 1, &scissor);

		VkDeviceSize offsets[] = { 0 };

		vkCmdPushConstants(myCommandBuffers[i], myPipelineLayouts.offscreen, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(fw::Mat4f), &instance->GetTransform());

		vkCmdBindVertexBuffers(myCommandBuffers[i], 0, 1, &model->GetVertexBuffer().buffer, offsets);
		vkCmdBindIndexBuffer(myCommandBuffers[i], model->GetIndexBuffer().buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myPipelineLayouts.offscreen, 0, 1, &model->GetDescriptorSet(), 0, nullptr);
		vkCmdDrawIndexed(myCommandBuffers[i], (u32)model->GetIndexCount(), 1, 0, 0, 0);

		vkEndCommandBuffer(myCommandBuffers[i]);
	}

	myFramework->EndCommandBufferRecording(idx, aModels.size() > 0 ? myCommandBuffers : std::vector<VkCommandBuffer>());

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { myFramework->myImageAvailableSemaphores[myFramework->myCurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &myFramework->myCommandBuffers[idx];

	VkSemaphore signalSemaphores[] = { myOffscreenSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(myFramework->myDevice, 1, &myFramework->myInFlightFences[myFramework->myCurrentFrame]);

	VkResult result = vkQueueSubmit(myFramework->myGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to submit draw command buffer");
	}

	BuildDeferredCommandBuffers(myFramework->mySwapChainFramebuffers[idx], aCamera, aLights);

	submitInfo.pWaitSemaphores = &myOffscreenSemaphore;
	submitInfo.pSignalSemaphores = &myFramework->myRenderFinishedSemaphores[myFramework->myCurrentFrame];
	submitInfo.pCommandBuffers = &myDeferredCommandBuffer;
	submitInfo.commandBufferCount = (u32)1;

	result = vkQueueSubmit(myFramework->myGraphicsQueue, 1, &submitInfo, myFramework->myInFlightFences[myFramework->myCurrentFrame]);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to submit draw command buffer");
	}

	bool shouldResize = !myFramework->EndFrame(idx, myFramework->myRenderFinishedSemaphores[myFramework->myCurrentFrame]);

	if (shouldResize)
	{
		Resize();
	}
}

void frostwave::Renderer::Destroy()
{
	DestroyOffscreenFrameBuffer();

	myQuad.Destroy();

	myUniformBuffers.offscreen.Destroy();
	myUniformBuffers.fullscreen.Destroy();
	if (myCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(myFramework->GetDevice(), myCommandPool, nullptr);
	}

	vkDestroyDescriptorSetLayout(myFramework->GetDevice(), myDescriptorSetLayout, nullptr);

	vkDestroyPipeline(myFramework->GetDevice(), myPipelines.deferred, nullptr);
	vkDestroyPipeline(myFramework->GetDevice(), myPipelines.offscreen, nullptr);

	vkDestroyPipelineLayout(myFramework->GetDevice(), myPipelineLayouts.deferred, nullptr);
	vkDestroyPipelineLayout(myFramework->GetDevice(), myPipelineLayouts.offscreen, nullptr);

	vkDestroySemaphore(myFramework->GetDevice(), myOffscreenSemaphore, nullptr);
	vkDestroySampler(myFramework->GetDevice(), myColorSampler, nullptr);
	vkDestroyDescriptorPool(myFramework->GetDevice(), myDescriptorPool, nullptr);
}

fw::Buffer* frostwave::Renderer::GetUBO()
{
	return &myUniformBuffers.offscreen;
}

const VkDescriptorSetLayout& frostwave::Renderer::GetDescriptorSetLayout() const
{
	return myDescriptorSetLayout;
}

void frostwave::Renderer::Resize()
{
	myFramework->WaitIdle();

	//PrepareOffscreenFramebuffer();
	//PreparePipelines();
}

void frostwave::Renderer::PrepareOffscreenFramebuffer()
{
	myOffscreenFramebuffer.width = myFramework->GetSwapchainExtent().width;
	myOffscreenFramebuffer.height = myFramework->GetSwapchainExtent().height;

	CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &myOffscreenFramebuffer.position);
	CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &myOffscreenFramebuffer.normal);
	CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &myOffscreenFramebuffer.material);
	CreateAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &myOffscreenFramebuffer.albedo);

	VkFormat depthFormat = myFramework->FindDepthFormat();
	CreateAttachment(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &myOffscreenFramebuffer.depth);

	std::array<VkAttachmentDescription, 5> attachmentDescriptions = { };
	for (u32 i = 0; i < attachmentDescriptions.size(); ++i)
	{
		attachmentDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if (i == attachmentDescriptions.size() - 1)
		{
			attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	attachmentDescriptions[0].format = myOffscreenFramebuffer.position.format;
	attachmentDescriptions[1].format = myOffscreenFramebuffer.normal.format;
	attachmentDescriptions[2].format = myOffscreenFramebuffer.albedo.format;
	attachmentDescriptions[3].format = myOffscreenFramebuffer.material.format;
	attachmentDescriptions[4].format = myOffscreenFramebuffer.depth.format;

	std::vector<VkAttachmentReference> colorReferences;
	colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 4;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = { };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = (u32)colorReferences.size();
	subpass.pDepthStencilAttachment = &depthReference;

	std::array<VkSubpassDependency, 2> dependencies;
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = { };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.attachmentCount = (u32)attachmentDescriptions.size();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	VkResult result = vkCreateRenderPass(myFramework->GetDevice(), &renderPassInfo, nullptr, &myOffscreenFramebuffer.renderPass);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create renderpass for offscreen framebuffer!");
	}

	std::array<VkImageView, 5> attachments;
	attachments[0] = myOffscreenFramebuffer.position.view;
	attachments[1] = myOffscreenFramebuffer.normal.view;
	attachments[2] = myOffscreenFramebuffer.albedo.view;
	attachments[3] = myOffscreenFramebuffer.material.view;
	attachments[4] = myOffscreenFramebuffer.depth.view;

	VkFramebufferCreateInfo frameBufferCreateInfo = { };
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = NULL;
	frameBufferCreateInfo.renderPass = myOffscreenFramebuffer.renderPass;
	frameBufferCreateInfo.pAttachments = attachments.data();
	frameBufferCreateInfo.attachmentCount = (u32)attachments.size();
	frameBufferCreateInfo.width = myOffscreenFramebuffer.width;
	frameBufferCreateInfo.height = myOffscreenFramebuffer.height;
	frameBufferCreateInfo.layers = 1;
	result = vkCreateFramebuffer(myFramework->GetDevice(), &frameBufferCreateInfo, nullptr, &myOffscreenFramebuffer.frameBuffer);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create offscreen framebuffer!");
	}

	VkSamplerCreateInfo sampler = { };
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	result = vkCreateSampler(myFramework->GetDevice(), &sampler, nullptr, &myColorSampler);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create sampler for offscreen framebuffer!");
	}
}

void frostwave::Renderer::CreatePoolAndBuffers()
{
	if (myCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(myFramework->GetDevice(), myCommandPool, nullptr);
	}

	VkCommandPoolCreateInfo info = { };
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	info.queueFamilyIndex = myFramework->GetQueueFamilyIndices().graphicsFamily;
	vkCreateCommandPool(myFramework->GetDevice(), &info, nullptr, &myCommandPool);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = { };
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = myCommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	commandBufferAllocateInfo.commandBufferCount = (u32)myCommandBuffers.size();
	VkResult result = vkAllocateCommandBuffers(myFramework->GetDevice(), &commandBufferAllocateInfo, myCommandBuffers.data());

	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate command buffers");
	}

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = myCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(myFramework->GetDevice(), &allocInfo, &myDeferredCommandBuffer);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate command buffers!");
	}
}

void frostwave::Renderer::SetupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		fw::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		fw::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		fw::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
		fw::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
		fw::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),
		fw::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 5)
	};

	VkDescriptorSetLayoutCreateInfo layout = { };
	layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout.pBindings = bindings.data();
	layout.bindingCount = (u32)bindings.size();

	VkResult result = vkCreateDescriptorSetLayout(myFramework->GetDevice(), &layout, nullptr, &myDescriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create descriptor set layout for renderer");
	}

	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.size = sizeof(fw::Mat4f);
	pushConstantRange.offset = 0;

	VkPipelineLayoutCreateInfo pipelineLayout = { };
	pipelineLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayout.pSetLayouts = &myDescriptorSetLayout;
	pipelineLayout.setLayoutCount = 1;
	pipelineLayout.pushConstantRangeCount = 1;
	pipelineLayout.pPushConstantRanges = &pushConstantRange;

	result = vkCreatePipelineLayout(myFramework->GetDevice(), &pipelineLayout, nullptr, &myPipelineLayouts.offscreen);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create pipeline layout for offscreen");
	}

	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.size = sizeof(PointLight);
	pushConstantRange.offset = 0;

	//pipelineLayout.pushConstantRangeCount = 1;
	//pipelineLayout.pPushConstantRanges = &pushConstantRange;

	result = vkCreatePipelineLayout(myFramework->GetDevice(), &pipelineLayout, nullptr, &myPipelineLayouts.deferred);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create pipeline layout for deferred");
	}
}

void frostwave::Renderer::PreparePipelines()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { };
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;
	inputAssemblyState.flags = 0;

	VkPipelineRasterizationStateCreateInfo rasterizationState = { };
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.depthClampEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState blendAttachmentState = { };
	blendAttachmentState.colorWriteMask = 0xF;
	blendAttachmentState.blendEnable = VK_TRUE;
	blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { };
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = { };
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.front = depthStencilState.back;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineViewportStateCreateInfo viewportState = { };
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	viewportState.flags = 0;

	VkPipelineMultisampleStateCreateInfo multisampleState = { };
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.flags = 0;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
	fw::initializers::VertexInputBindingDescription(0, layout.Stride(), VK_VERTEX_INPUT_RATE_VERTEX)
	};

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
		fw::initializers::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),					//position
		fw::initializers::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(f32) * 3),		//texcoord
		fw::initializers::VertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32B32_SFLOAT, sizeof(f32) * 5),	//normal
		fw::initializers::VertexInputAttributeDescription(0, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(f32) * 8),	//tangent
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = (u32)bindingDescriptions.size();
	vertexInputInfo.vertexAttributeDescriptionCount = (u32)attributeDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState = { };
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = (u32)dynamicStateEnables.size();
	dynamicState.flags = 0;

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = { };
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout = myPipelineLayouts.deferred;
	pipelineCreateInfo.renderPass = myFramework->GetRenderPass();
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = (u32)shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();

	shaderStages[0] = LoadShader("assets/shaders/deferred_vs.spv", VK_SHADER_STAGE_VERTEX_BIT, myFramework);
	shaderStages[1] = LoadShader("assets/shaders/deferred_fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, myFramework);

	VkPipelineVertexInputStateCreateInfo emptyInputState = { };
	emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineCreateInfo.pVertexInputState = &emptyInputState;
	pipelineCreateInfo.layout = myPipelineLayouts.deferred;

	VkResult result = vkCreateGraphicsPipelines(myFramework->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &myPipelines.deferred);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create deferred pipeline!");
	}

	vkDestroyShaderModule(myFramework->GetDevice(), shaderStages[0].module, nullptr);
	vkDestroyShaderModule(myFramework->GetDevice(), shaderStages[1].module, nullptr);

	shaderStages[0] = LoadShader("assets/shaders/mrt_vs.spv", VK_SHADER_STAGE_VERTEX_BIT, myFramework);
	shaderStages[1] = LoadShader("assets/shaders/mrt_fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, myFramework);

	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.layout = myPipelineLayouts.offscreen;
	pipelineCreateInfo.renderPass = myOffscreenFramebuffer.renderPass;

	std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates = {
		fw::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
		fw::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
		fw::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
		fw::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};

	colorBlendState.attachmentCount = (u32)blendAttachmentStates.size();
	colorBlendState.pAttachments = blendAttachmentStates.data();

	result = vkCreateGraphicsPipelines(myFramework->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &myPipelines.offscreen);

	vkDestroyShaderModule(myFramework->GetDevice(), shaderStages[0].module, nullptr);
	vkDestroyShaderModule(myFramework->GetDevice(), shaderStages[1].module, nullptr);
}

void frostwave::Renderer::GenerateQuad()
{
	struct Vertex {
		float pos[3];
		float uv[2];
		float normal[3];
	};

	std::vector<Vertex> vertexBuffer;

	float x = 0.0f;
	float y = 0.0f;
	for (uint32_t i = 0; i < 3; i++)
	{
		vertexBuffer.push_back({ { x + 1.0f, y + 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } });
		vertexBuffer.push_back({ { x,      y + 1.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f} });
		vertexBuffer.push_back({ { x,      y,      0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } });
		vertexBuffer.push_back({ { x + 1.0f, y,      0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } });
		x += 1.0f;
		if (x > 1.0f)
		{
			x = 0.0f;
			y += 1.0f;
		}
	}

	VkResult result = CreateBuffer(myFramework, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&myQuad.GetVertexBuffer(), vertexBuffer.size() * sizeof(Vertex), vertexBuffer.data());
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create vertex buffer for quad!");
	}

	std::vector<uint32_t> indexBuffer = { 0,1,2, 2,3,0 };
	for (uint32_t i = 0; i < 3; ++i)
	{
		uint32_t indices[6] = { 0,1,2, 2,3,0 };
		for (auto index : indices)
		{
			indexBuffer.push_back(i * 4 + index);
		}
	}
	myQuad.SetIndexCount((u32)indexBuffer.size());

	result = CreateBuffer(myFramework, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&myQuad.GetIndexBuffer(), indexBuffer.size() * sizeof(uint32_t), indexBuffer.data());
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create index buffer for quad!");
	}
}

void frostwave::Renderer::SetupDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &myDescriptorSetLayout;
	allocInfo.descriptorPool = myDescriptorPool;

	VkResult result = vkAllocateDescriptorSets(myFramework->GetDevice(), &allocInfo, &myDescriptorSet);

	VkDescriptorImageInfo texDescriptorPosition = { };
	texDescriptorPosition.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	texDescriptorPosition.imageView = myOffscreenFramebuffer.position.view;
	texDescriptorPosition.sampler = myColorSampler;

	VkDescriptorImageInfo texDescriptorNormal = { };
	texDescriptorNormal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	texDescriptorNormal.imageView = myOffscreenFramebuffer.normal.view;
	texDescriptorNormal.sampler = myColorSampler;

	VkDescriptorImageInfo texDescriptorAlbedo = { };
	texDescriptorAlbedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	texDescriptorAlbedo.imageView = myOffscreenFramebuffer.albedo.view;
	texDescriptorAlbedo.sampler = myColorSampler;

	VkDescriptorImageInfo texDescriptorMaterial = { };
	texDescriptorMaterial.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	texDescriptorMaterial.imageView = myOffscreenFramebuffer.material.view;
	texDescriptorMaterial.sampler = myColorSampler;

	writeDescriptorSets = {
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &myUniformBuffers.fullscreen.descriptor),
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptorPosition),
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &texDescriptorNormal),
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &texDescriptorAlbedo),
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &texDescriptorMaterial),
		fw::initializers::WriteDescriptorSet(myDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, &myUniformBuffers.fullscreen.descriptor),
	};

	vkUpdateDescriptorSets(myFramework->GetDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void frostwave::Renderer::PrepareUniformBuffers()
{
	CreateBuffer(myFramework, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&myUniformBuffers.offscreen, sizeof(myUBO));
	CreateBuffer(myFramework, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&myUniformBuffers.fullscreen, sizeof(myUBOFullscreen));

	VkResult result = myUniformBuffers.fullscreen.Map();
	if (result != VK_SUCCESS) FATAL_LOG("Failed to map persistant!");

	result = myUniformBuffers.offscreen.Map();
	if (result != VK_SUCCESS) FATAL_LOG("Failed to map persistant!");
}

void frostwave::Renderer::BuildDeferredCommandBuffers(VkFramebuffer aFramebuffer, Camera* aCamera, const std::vector<PointLight>& aLights)
{
	myUBOFullscreen.cameraPos = fw::Vec4f(aCamera->GetPosition(), 0.0f);
	memcpy(myUniformBuffers.fullscreen.mapped, &myUBOFullscreen, sizeof(myUBOFullscreen));

	VkCommandBufferBeginInfo cmdBufInfo = { };
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearValues[2];
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = { };
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = myFramework->GetRenderPass();
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = myFramework->GetSwapchainExtent();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = aFramebuffer;

	VkResult result = vkBeginCommandBuffer(myDeferredCommandBuffer, &cmdBufInfo);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to begin command buffer recording!");
	}

	VkViewport viewport = { };
	viewport.width = (f32)myOffscreenFramebuffer.width;
	viewport.height = (f32)myOffscreenFramebuffer.height;

	vkCmdSetViewport(myDeferredCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor = { };
	scissor.extent.width = myOffscreenFramebuffer.width;
	scissor.extent.height = myOffscreenFramebuffer.height;

	vkCmdSetScissor(myDeferredCommandBuffer, 0, 1, &scissor);

	vkCmdBeginRenderPass(myDeferredCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkDeviceSize offsets[1] = { 0 };

	for (auto light : aLights)
	{
		light.position.y *= -1.0f;
		vkCmdPushConstants(myDeferredCommandBuffer, myPipelineLayouts.deferred, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLight), &light);

		vkCmdBindDescriptorSets(myDeferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myPipelineLayouts.deferred, 0, 1, &myDescriptorSet, 0, nullptr);
		vkCmdBindPipeline(myDeferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myPipelines.deferred);
		vkCmdBindVertexBuffers(myDeferredCommandBuffer, 0, 1, &myQuad.GetVertexBuffer().buffer, offsets);
		vkCmdBindIndexBuffer(myDeferredCommandBuffer, myQuad.GetIndexBuffer().buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(myDeferredCommandBuffer, 6, 1, 0, 0, 1);
	}

	vkCmdEndRenderPass(myDeferredCommandBuffer);

	result = vkEndCommandBuffer(myDeferredCommandBuffer);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to end command buffer recording!");
	}
}

void frostwave::Renderer::SetupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		fw::initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8),
		fw::initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 9)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = { };
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = (u32)poolSizes.size();
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = 3;

	VkResult result = vkCreateDescriptorPool(myFramework->GetDevice(), &descriptorPoolInfo, nullptr, &myDescriptorPool);
}

void frostwave::Renderer::CreateAttachment(VkFormat aFormat, VkImageUsageFlagBits aUsage, FrameBufferAttachment* aAttachment)
{
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;

	aAttachment->format = aFormat;

	if (aUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (aUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	assert(aspectMask > 0);

	VkImageCreateInfo image = { };
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = aFormat;
	image.extent.width = myOffscreenFramebuffer.width;
	image.extent.height = myOffscreenFramebuffer.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = aUsage | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryAllocateInfo memAlloc = { };
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkMemoryRequirements memReqs;

	VkResult result = vkCreateImage(myFramework->GetDevice(), &image, nullptr, &aAttachment->image);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create image for framebuffer attachment!");
	}

	vkGetImageMemoryRequirements(myFramework->GetDevice(), aAttachment->image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = FindMemoryType(myFramework->GetPhysicalDevice(), memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	result = vkAllocateMemory(myFramework->GetDevice(), &memAlloc, nullptr, &aAttachment->memory);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to allocate memory for framebuffer attachment!");
	}

	result = vkBindImageMemory(myFramework->GetDevice(), aAttachment->image, aAttachment->memory, 0);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to bind image memory for framebuffer attachment!");
	}

	VkImageViewCreateInfo imageView = { };
	imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageView.format = aFormat;
	imageView.subresourceRange = {};
	imageView.subresourceRange.aspectMask = aspectMask;
	imageView.subresourceRange.baseMipLevel = 0;
	imageView.subresourceRange.levelCount = 1;
	imageView.subresourceRange.baseArrayLayer = 0;
	imageView.subresourceRange.layerCount = 1;
	imageView.image = aAttachment->image;
	result = vkCreateImageView(myFramework->GetDevice(), &imageView, nullptr, &aAttachment->view);
	if (result != VK_SUCCESS)
	{
		FATAL_LOG("Failed to create image view for framebuffer attachment!");
	}
}

void frostwave::Renderer::DestroyOffscreenFrameBuffer()
{
	vkDestroyFramebuffer(myFramework->GetDevice(), myOffscreenFramebuffer.frameBuffer, nullptr);
	vkDestroyRenderPass(myFramework->GetDevice(), myOffscreenFramebuffer.renderPass, nullptr);

	vkDestroyImageView(myFramework->GetDevice(), myOffscreenFramebuffer.albedo.view, nullptr);
	vkFreeMemory(myFramework->GetDevice(), myOffscreenFramebuffer.albedo.memory, nullptr);
	vkDestroyImage(myFramework->GetDevice(), myOffscreenFramebuffer.albedo.image, nullptr);

	vkDestroyImageView(myFramework->GetDevice(), myOffscreenFramebuffer.normal.view, nullptr);
	vkFreeMemory(myFramework->GetDevice(), myOffscreenFramebuffer.normal.memory, nullptr);
	vkDestroyImage(myFramework->GetDevice(), myOffscreenFramebuffer.normal.image, nullptr);

	vkDestroyImageView(myFramework->GetDevice(), myOffscreenFramebuffer.position.view, nullptr);
	vkFreeMemory(myFramework->GetDevice(), myOffscreenFramebuffer.position.memory, nullptr);
	vkDestroyImage(myFramework->GetDevice(), myOffscreenFramebuffer.position.image, nullptr);

	vkDestroyImageView(myFramework->GetDevice(), myOffscreenFramebuffer.material.view, nullptr);
	vkFreeMemory(myFramework->GetDevice(), myOffscreenFramebuffer.material.memory, nullptr);
	vkDestroyImage(myFramework->GetDevice(), myOffscreenFramebuffer.material.image, nullptr);

	vkDestroyImageView(myFramework->GetDevice(), myOffscreenFramebuffer.depth.view, nullptr);
	vkFreeMemory(myFramework->GetDevice(), myOffscreenFramebuffer.depth.memory, nullptr);
	vkDestroyImage(myFramework->GetDevice(), myOffscreenFramebuffer.depth.image, nullptr);
}

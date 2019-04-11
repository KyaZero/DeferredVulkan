#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <Frostwave/Graphics/Lights.h>

namespace frostwave
{
	class Renderer;
	class ModelInstance;
	class Camera;
	class Scene
	{
	public:
		Scene();
		~Scene();
		
		void Submit(ModelInstance* aModel);
		void Submit(PointLight aLight);

		void Render(Renderer* aRenderer, Camera* aCamera);

		const std::vector<ModelInstance*>& GetModels() const;
	private: 
		std::vector<PointLight> myLights;
		std::vector<ModelInstance*> myModels;
	};
}
namespace fw = frostwave;
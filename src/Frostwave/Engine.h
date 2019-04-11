#pragma once

#include <Frostwave/Core/Common.h>
#include <Frostwave/Core/Types.h>
#include <Frostwave/Core/Timer.h>
#include <Frostwave/Debug/Logger.h>
#include <Frostwave/Settings.h>

#include <Frostwave/Graphics/VkFramework.h>
#include <Frostwave/Graphics/Renderer.h>
#include <Frostwave/Graphics/Scene.h>
#include <Frostwave/Graphics/ModelInstance.h>

namespace frostwave
{
	class Engine
	{
	public:
	public:
		Engine();
		~Engine();
		void Init(Settings aSettings = {});
		void Run();
		void Shutdown();
		void Destroy();

		VkFramework* GetFramework();

		bool GetShouldRenderModel() const { return myShouldRenderModel; }
		void SetShouldRenderModel(bool aShouldRenderModel) { myShouldRenderModel = aShouldRenderModel; }

		static void FramebufferResizeCallback(GLFWwindow* aWindow, i32 aWidth, i32 aHeight);

	private:
		void RenderFrame();
		void InitWindow();
		GLFWwindow* myWindow;
		Settings mySettings;
		VkFramework myVKFramework;
		Scene myScene;
		Renderer myRenderer;
		bool myShouldRun, myShouldRenderModel;
		fw::Model myModel, myFloor;
		fw::ModelInstance* myInstances[10];
		fw::ModelInstance* myFloorInstance;
		fw::Camera myCamera;
		fw::Timer myTimer;
	};
}
namespace fw = frostwave;

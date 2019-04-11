#include "stdafx.h"
#include "Engine.h"
#include "GLFWExtras.h"
#include <thread>

frostwave::Engine::Engine() : myShouldRun(true), myShouldRenderModel(true)
{
}

frostwave::Engine::~Engine()
{
	glfwDestroyWindow(myWindow);
	glfwTerminate();
}

void frostwave::Engine::RenderFrame()
{
	myScene.Render(&myRenderer, &myCamera);
}

void KeyCallback(GLFWwindow* aWindow, i32 aKey, i32 aScancode, i32 aAction, i32 aMods)
{
	if (aKey == GLFW_KEY_E && aAction == GLFW_PRESS)
	{
		auto* engine = (frostwave::Engine*)(glfwGetWindowUserPointer(aWindow));
		engine->SetShouldRenderModel(!engine->GetShouldRenderModel());
	}
}

void frostwave::Engine::FramebufferResizeCallback(GLFWwindow* aWindow, i32 aWidth, i32 aHeight)
{
	auto* engine = (frostwave::Engine*)(glfwGetWindowUserPointer(aWindow));
	engine->GetFramework()->SetFramebufferResized(true);
	engine->myCamera.UpdateAspectRatio(aWidth, aHeight);
}

void frostwave::Engine::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWmonitor* monitor = nullptr;
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	u32 width = mySettings.window.width;
	u32 height = mySettings.window.height;

	if (mySettings.window.mode & WindowSettings::Fullscreen)
	{
		monitor = glfwGetPrimaryMonitor();
		width = mode->width;
		height = mode->height;
	}

	if (mySettings.window.mode & WindowSettings::Borderless)
	{
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		glfwWindowHint(GLFW_AUTO_ICONIFY, false);
		glfwWindowHint(GLFW_DECORATED, false);
	}

	myWindow = glfwCreateWindow(width, height, mySettings.window.title.c_str(), monitor, nullptr);

	//if windowed, center window...
	if ((mySettings.window.mode & WindowSettings::Windowed) || !(mySettings.window.mode & WindowSettings::Fullscreen))
		glfwSetWindowCenter(myWindow);

	glfwSetWindowUserPointer(myWindow, this);
	glfwSetFramebufferSizeCallback(myWindow, FramebufferResizeCallback);
	glfwSetKeyCallback(myWindow, KeyCallback);
}

void frostwave::Engine::Shutdown()
{
	myShouldRun = false;
}

void frostwave::Engine::Destroy()
{
	myVKFramework.WaitIdle();
	myRenderer.Destroy();
	myModel.Destroy();
	myFloor.Destroy();
}

frostwave::VkFramework* frostwave::Engine::GetFramework()
{
	return &myVKFramework;
}

const string MODEL_PATH = "assets/meshes/chest.fbx";
const string TEXTURE_PATH = "assets/textures/chest_ALB.png";
const string NORMAL_PATH = "assets/textures/chest_NAO.png";
const string MATERIAL_PATH = "assets/textures/chest_MRE.png";

void frostwave::Engine::Init(Settings aSettings)
{
	mySettings = aSettings;
	InitWindow();
	myVKFramework.Init(myWindow, aSettings.graphics);
	myRenderer.Init(&myVKFramework);

	ImageCreateInfo iinfo = {};
	iinfo.type = ImageType::Texture;
	iinfo.width = 0;
	iinfo.height = 0;
	iinfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	iinfo.samples = VK_SAMPLE_COUNT_1_BIT;
	iinfo.path = TEXTURE_PATH;
	iinfo.normalPath = NORMAL_PATH;
	iinfo.materialPath = MATERIAL_PATH;
	iinfo.generateMips = false;

	ModelCreateInfo info;
	info.scale = 0.01f;
	myModel.Load(MODEL_PATH, &myRenderer, layout, iinfo, &info, &myVKFramework, myVKFramework.GetGraphicsQueue());

	iinfo.path = "assets/textures/floor_ALB.png";
	iinfo.normalPath = "assets/textures/floor_NAO.png";
	iinfo.materialPath = "assets/textures/floor_MRE.png";

	info.scale = 0.5f;
	myFloor.Load("assets/meshes/floor.fbx", &myRenderer, layout, iinfo, &info, &myVKFramework, myVKFramework.GetGraphicsQueue());

	for (i32 i = 0; i < 10; ++i)
		myInstances[i] = new fw::ModelInstance(&myModel);

	myFloorInstance = new fw::ModelInstance(&myFloor);

	myCamera.Init(90.0f, mySettings.window.width / (f32)mySettings.window.height, 0.01f, 100.0f);
	myCamera.SetPosition({ 8,8,8 });
}

void frostwave::Engine::Run()
{
	while (!glfwWindowShouldClose(myWindow) && myShouldRun)
	{
		glfwPollEvents();
		myTimer.Update();
		glfwSetWindowTitle(myWindow, (mySettings.window.title + " | " + fw::ToString(myTimer.GetDeltaTime() * 1000.0f) + "ms").c_str());

		if (mySettings.window.focusFreeze && !glfwGetWindowAttrib(myWindow, GLFW_FOCUSED))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		myScene.Submit(PointLight{ {tanf((f32)myTimer.GetTotalTime() * -0.1f) * 4.0f,10.0f,tanf((f32)myTimer.GetTotalTime() * -0.1f) * 4.0f,0}, fw::Vec3f{1.5f,1.5f,0.0f} * 16.0f, 20.0f });
		myScene.Submit(PointLight{ {-4.0f * cosf((f32)myTimer.GetTotalTime() * -1.5f),3.0f,-4.0f * sinf((f32)myTimer.GetTotalTime() * -1.5f),0}, {0.0f,1.0f,1.0f}, 10.5f });
		myScene.Submit(PointLight{ {2.0f * cosf((f32)myTimer.GetTotalTime()),2.0f,2.0f * sinf((f32)myTimer.GetTotalTime()),0}, {1.0f,0.0f,1.0f}, 5.0f });
		myScene.Submit(PointLight{ {0,4.0f,0,0}, {2.0f,2.0f,2.0f}, 50.0f });

		myFloorInstance->SetPosition({ -100.0f,-1,-100.0f });
		myScene.Submit(myFloorInstance);

		for (i32 i = 0; i < 10; ++i)
		{
			if (myShouldRenderModel)
			{
				myScene.Submit(myInstances[i]);
			}

			myInstances[i]->SetPosition({ cosf((f32)myTimer.GetTotalTime() + (i * 4)) * (i * 4), 0, sinf((f32)myTimer.GetTotalTime() + (i * 4)) * (i * 4) });
			myInstances[i]->SetRotation(fw::Quatf({ 0,1,0 }, (f32)myTimer.GetTotalTime() * (-i)));
			//	fw::Quatf({ 1,0,1 }, (f32)myTimer.GetTotalTime() * (i)) *
			//	fw::Quatf({ 0,1,1 }, (f32)myTimer.GetTotalTime() * (i)));
		}

		myCamera.SetRotation(fw::Mat4f::CreateLookAt(0, myCamera.GetPosition(), { 0,1,0 }));
		myCamera.Update();

		RenderFrame();
	}

	//shutdown has happened
	Destroy();
}
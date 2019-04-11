#include "stdafx.h"
#include "Scene.h"

#include "Renderer.h"
#include "Model.h"

#include <Frostwave/Graphics/Camera.h>

frostwave::Scene::Scene()
{

}

frostwave::Scene::~Scene()
{

}

void frostwave::Scene::Submit(ModelInstance* aModel)
{
	myModels.push_back(aModel);
}

void frostwave::Scene::Submit(PointLight aLight)
{
	myLights.push_back(aLight);
}

void frostwave::Scene::Render(Renderer* aRenderer, fw::Camera* aCamera)
{
	aRenderer->Render(myModels, myLights, aCamera);
	myModels.clear();
	myLights.clear();
}

const std::vector<fw::ModelInstance*>& frostwave::Scene::GetModels() const
{
	return myModels;
}

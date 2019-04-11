#include "stdafx.h"
#include "ModelInstance.h"

frostwave::ModelInstance::ModelInstance(Model* aModel) : myModel(aModel)
{
}

frostwave::ModelInstance::~ModelInstance()
{
}

const fw::Model* frostwave::ModelInstance::GetModel() const
{
	return myModel;
}

void frostwave::ModelInstance::SetPosition(const fw::Vec3f& aPosition)
{
	myPosition = aPosition;
}

void frostwave::ModelInstance::SetRotation(const fw::Quatf& aRotation)
{
	myRotation = aRotation;
}

const fw::Vec3f& frostwave::ModelInstance::GetPosition() const
{
	return myPosition;
}

const fw::Quatf& frostwave::ModelInstance::GetRotation() const
{
	return myRotation;
}

fw::Mat4f frostwave::ModelInstance::GetTransform() const
{
	return fw::Mat4f::CreateTransform(myPosition, myRotation, 1.0f);
}

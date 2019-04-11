#include "stdafx.h"
#include "Camera.h"

void frostwave::Camera::Init(f32 aFOV, f32 aAspectRatio, f32 aNearZ, f32 aFarZ)
{
	myFOV = aFOV;
	myNearZ = aNearZ;
	myFarZ = aFarZ;
	myProjection = fw::Mat4f::CreatePerspectiveProjection(myFOV, aAspectRatio, myNearZ, myFarZ);
}

void frostwave::Camera::Update()
{
	fw::Mat4f transform = fw::Mat4f::CreateTransform(myPosition, myRotation, 1.0f);
	myView = fw::Mat4f::FastInverse(transform);
}

void frostwave::Camera::SetPosition(const fw::Vec3f& aPosition)
{
	myPosition = aPosition;
}

void frostwave::Camera::SetRotation(const fw::Quatf& aRotation)
{
	myRotation = aRotation;
}

void frostwave::Camera::UpdateAspectRatio(i32 aWidth, i32 aHeight)
{
	Init(myFOV, (f32)aWidth / (f32)aHeight, myNearZ, myFarZ);
}

const fw::Vec3f& frostwave::Camera::GetPosition() const
{
	return myPosition;
}

const fw::Quatf& frostwave::Camera::GetRotation() const
{
	return myRotation;
}

fw::Mat4f& frostwave::Camera::GetView()
{
	return myView;
}

fw::Mat4f& frostwave::Camera::GetProjection()
{
	return myProjection;
}

#pragma once
#include <Frostwave/Core/Math/Vector.h>

struct PointLight
{
	fw::Vec4f position;
	fw::Vec3f color;
	f32 radius;
};
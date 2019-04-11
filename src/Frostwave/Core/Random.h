#pragma once
#include <Frostwave/stdafx.h>

namespace frostwave
{
	template <typename T>
	inline T RandomRange(T aMin, T aMax)
	{
		if (aMin == fw::Max(aMin, aMax)) std::swap(aMin, aMax);
		static std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<double> dist(static_cast<double>(aMin), static_cast<double>(aMax));
		return static_cast<T>(dist(gen));
	}

	inline f32 Rand() { return RandomRange(0.0f, 1.0f); }
	inline f32 Rand11() { return RandomRange(-1.0f, 1.0f); }
}
namespace fw = frostwave;

#pragma once

#include <Frostwave/Core/Math/Matrix.h>
#include <Frostwave/Core/Math/Quaternion.h>

namespace frostwave
{
	class Camera
	{
	public:
		Camera() { }
		~Camera() { }

		void Init(f32 aFOV, f32 aAspectRatio, f32 aNearZ, f32 aFarZ);
		void Update();

		void SetPosition(const fw::Vec3f& aPosition);
		void SetRotation(const fw::Quatf& aRotation);

		void UpdateAspectRatio(i32 aWidth, i32 aHeight);

		const fw::Vec3f& GetPosition() const;
		const fw::Quatf& GetRotation() const;

		fw::Mat4f& GetView();
		fw::Mat4f& GetProjection();
	private:
		fw::Mat4f myView, myProjection;
		fw::Vec3f myPosition;
		fw::Quatf myRotation;
		f32 myFOV, myNearZ, myFarZ;
	};
}
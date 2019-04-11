#pragma once

#include <Frostwave/Core/Math/Vector.h>
#include <Frostwave/Core/Math/Quaternion.h>

namespace frostwave
{
	class Model;
	class ModelInstance
	{
	public:
		ModelInstance(Model* aModel);
		~ModelInstance();

		const Model* GetModel() const;

		void SetPosition(const fw::Vec3f& aPosition);
		void SetRotation(const fw::Quatf& aRotation);

		const fw::Vec3f& GetPosition() const;
		const fw::Quatf& GetRotation() const;

		fw::Mat4f GetTransform() const;
	private:
		Model* const myModel;
		fw::Vec3f myPosition;
		fw::Quatf myRotation;
	};
}
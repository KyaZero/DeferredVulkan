#pragma once
#include "stdafx.h"
#include "Vector3.h"
#include "Matrix4x4.h"

namespace frostwave
{
	template <class T>
	class Matrix4x4;

	template <class T>
	class Matrix3x3
	{
	public:
		friend class Matrix4x4<T>;
		Matrix3x3<T>();
		Matrix3x3(const std::initializer_list<T>& aInitList);
		Matrix3x3<T>(const Matrix3x3<T>& aMatrix);
		Matrix3x3<T>(const Matrix4x4<T>& aMatrix);

		static Matrix3x3<T> CreateRotationAroundX(T aAngleInRadians);
		static Matrix3x3<T> CreateRotationAroundY(T aAngleInRadians);
		static Matrix3x3<T> CreateRotationAroundZ(T aAngleInRadians);

		static Matrix3x3<T> CreateTransform(const Vector2<T>& aPosition, const f32 aRotation, const Vector2<T>& aScale);

		static Matrix3x3<T> Transpose(const Matrix3x3<T>& aMatrixToTranspose);
		static T Determinant(const Matrix3x3<T>& aMatrix);
		static Matrix3x3<T> Inverse(const Matrix3x3<T>& aMatrixToInvert);

		inline Matrix3x3<T>& operator=(const Matrix3x3<T>& aMatrix);
		inline T operator()(unsigned short aIndex0, unsigned short aIndex1) const { return myNumbers[aIndex0 - 1 + (aIndex1 - 1) * 3]; }
		inline Matrix3x3<T> operator+(const Matrix3x3<T>& aMatrix) const;
		inline Matrix3x3<T> operator-(const Matrix3x3<T>& aMatrix) const;
		inline Matrix3x3<T> operator*(const Matrix3x3<T>& aMatrix) const;
		inline Matrix3x3<T> operator*(const T& aScalar) const;
		inline bool operator==(const Matrix3x3<T>& aMatrix) const;

		inline void operator+=(const Matrix3x3<T>& aMatrix) { *this = *this + aMatrix; }
		inline void operator-=(const Matrix3x3<T>& aMatrix) { *this = *this - aMatrix; }
		inline void operator*=(const Matrix3x3<T>& aMatrix) { *this = *this * aMatrix; }
		inline void operator*=(const T& aScalar) { *this = *this * aScalar; };

		friend inline Vector3<T> operator*(const Vector3<T>& aVector, const Matrix3x3<T>& aMatrix)
		{
			return Vector3<T>(
				aVector.x * aMatrix.myNumbers[0] + aVector.y * aMatrix.myNumbers[3] + aVector.z * aMatrix.myNumbers[6],
				aVector.x * aMatrix.myNumbers[1] + aVector.y * aMatrix.myNumbers[4] + aVector.z * aMatrix.myNumbers[7],
				aVector.x * aMatrix.myNumbers[2] + aVector.y * aMatrix.myNumbers[5] + aVector.z * aMatrix.myNumbers[8]
				);
		}
		friend inline void operator*=(Vector3<T>& aVector, const Matrix3x3<T>& aMatrix) { aVector = aVector * aMatrix; };

		T myNumbers[9];//flyttad till public för att det är så i matrix4x4
	private:
	};

	template<class T>
	inline Matrix3x3<T>::Matrix3x3() : myNumbers{ 1, 0, 0, 0, 1, 0, 0, 0, 1 }
	{
	}
	template<class T>
	inline Matrix3x3<T>::Matrix3x3(const std::initializer_list<T>& aInitList)
	{
		assert(aInitList.size() == 9 && "Initializer list for Matrix3x3 must contain exactly 9 elements.");
		auto begin = aInitList.begin();
		for (size_t i = 0; i < 9; ++i)
		{
			myNumbers[i] = *(begin + i);
		}
	}
	template<class T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundX(T aAngleInRadians)
	{
		T c = cos(aAngleInRadians);
		T s = sin(aAngleInRadians);
		return Matrix3x3<T>({
			1, 0, 0,
			0, c, s,
			0, -s, c
		});
	}
	template<class T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundY(T aAngleInRadians)
	{
		T c = cos(aAngleInRadians);
		T s = sin(aAngleInRadians);
		return Matrix3x3<T>({
			c, 0, -s,
			0, 1, 0,
			s, 0, c
		});
	}
	template<class T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundZ(T aAngleInRadians)
	{
		T c = cos(aAngleInRadians);
		T s = sin(aAngleInRadians);
		return Matrix3x3<T>({
			c, s, 0,
			-s, c, 0,
			0, 0, 1
		});
	}
	template<class T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateTransform(const Vector2<T>& aPosition, const f32 aRotation, const Vector2<T>& aScale)
	{
		fw::Matrix3x3<T> scale = {
			aScale.x, 0, 0,
			0, aScale.y, 0,
			0, 0, 1,
		};

		fw::Matrix3x3<T> rotation = fw::Matrix3x3<T>::CreateRotationAroundZ(aRotation);

		fw::Matrix3x3<T> translation = {
			1, 0, 0,
			0, 1, 0,
			aPosition.x, aPosition.y, 1
		};
		return scale * rotation * translation;
	}
	template<class T>
	inline Matrix3x3<T>::Matrix3x3(const Matrix3x3<T>& aMatrix)
	{
		*this = aMatrix;
	}
	template<class T>
	inline Matrix3x3<T>::Matrix3x3(const Matrix4x4<T>& aMatrix)
	{
		myNumbers[0] = aMatrix.myNumbers[0];
		myNumbers[1] = aMatrix.myNumbers[1];
		myNumbers[2] = aMatrix.myNumbers[2];

		myNumbers[3] = aMatrix.myNumbers[4];
		myNumbers[4] = aMatrix.myNumbers[5];
		myNumbers[5] = aMatrix.myNumbers[6];

		myNumbers[6] = aMatrix.myNumbers[8];
		myNumbers[7] = aMatrix.myNumbers[9];
		myNumbers[8] = aMatrix.myNumbers[10];
	}
	template<class T>
	inline Matrix3x3<T> Matrix3x3<T>::Transpose(const Matrix3x3<T>& aMatrixToTranspose)
	{
		Matrix3x3<T> result(aMatrixToTranspose);

		result.myNumbers[1] = aMatrixToTranspose.myNumbers[3];
		result.myNumbers[3] = aMatrixToTranspose.myNumbers[1];

		result.myNumbers[2] = aMatrixToTranspose.myNumbers[6];
		result.myNumbers[6] = aMatrixToTranspose.myNumbers[2];

		result.myNumbers[5] = aMatrixToTranspose.myNumbers[7];
		result.myNumbers[7] = aMatrixToTranspose.myNumbers[5];

		return result;
	}
	template<class T>
	inline T Matrix3x3<T>::Determinant(const Matrix3x3<T>& aMatrix)
	{
		return (aMatrix.myNumbers[0] * aMatrix.myNumbers[4] * aMatrix.myNumbers[8] +
			aMatrix.myNumbers[1] * aMatrix.myNumbers[5] * aMatrix.myNumbers[6] +
			aMatrix.myNumbers[3] * aMatrix.myNumbers[7] * aMatrix.myNumbers[2]) -
			(aMatrix.myNumbers[2] * aMatrix.myNumbers[4] * aMatrix.myNumbers[6] +
				aMatrix.myNumbers[5] * aMatrix.myNumbers[7] * aMatrix.myNumbers[0] +
				aMatrix.myNumbers[1] * aMatrix.myNumbers[3] * aMatrix.myNumbers[8]);
	}
	template<class T>
	inline Matrix3x3<T> Matrix3x3<T>::Inverse(const Matrix3x3<T>& aMatrixToInvert)
	{
		T det = Determinant(aMatrixToInvert);
		assert(det != 0 && "Non-invertible matrix");
		Matrix3x3<T> result{
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[8] - aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[7],
			-(aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[8] - aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[7]),
			aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[5] - aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[4],
			-(aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[8] - aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[6]),
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[8] - aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[6],
			-(aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[5] - aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[3]),
			aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[7] - aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[6],
			-(aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[7] - aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[6]),
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[4] - aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[3] };
		return result * (1 / det);
	}
	template<class T>
	Matrix3x3<T>& Matrix3x3<T>::operator=(const Matrix3x3<T>& aMatrix)
	{
		for (size_t i = 0; i < 9; ++i)
		{
			myNumbers[i] = aMatrix.myNumbers[i];
		}
		return *this;
	}
	template<class T>
	Matrix3x3<T> Matrix3x3<T>::operator+(const Matrix3x3<T>& aMatrix) const
	{
		Matrix3x3<T> result;
		for (size_t i = 0; i < 9; i++)
		{
			result.myNumbers[i] = myNumbers[i] + aMatrix.myNumbers[i];
		}
		return result;
	}
	template<class T>
	Matrix3x3<T> Matrix3x3<T>::operator-(const Matrix3x3<T>& aMatrix) const
	{
		Matrix3x3<T> result;
		for (size_t i = 0; i < 9; i++)
		{
			result.myNumbers[i] = myNumbers[i] - aMatrix.myNumbers[i];
		}
		return result;
	}
	template<class T>
	Matrix3x3<T> Matrix3x3<T>::operator*(const Matrix3x3<T>& aMatrix) const
	{
		Matrix3x3<T> result;
		for (size_t i = 0; i <= 6; i += 3)
		{
			for (size_t j = 0; j < 3; j++)
			{
				result.myNumbers[i + j] = myNumbers[i] * aMatrix.myNumbers[j] + myNumbers[1 + i] * aMatrix.myNumbers[3 + j] + myNumbers[2 + i] * aMatrix.myNumbers[6 + j];
			}
		}
		return result;
	}
	template<class T>
	inline Matrix3x3<T> Matrix3x3<T>::operator*(const T & aScalar) const
	{
		Matrix3x3<T> result;
		for (size_t i = 0; i < 9; i++)
		{
			result.myNumbers[i] = myNumbers[i] * aScalar;
		}
		return result;
	}
	template<class T>
	bool Matrix3x3<T>::operator==(const Matrix3x3<T>& aMatrix) const
	{
		for (size_t i = 0; i < 9; i++)
		{
			if (myNumbers[i] != aMatrix.myNumbers[i])
			{
				return false;
			}
		}
		return true;
	}
	using Matrix3x3f = Matrix3x3<f32>;
	using Matrix3x3i = Matrix3x3<i32>;
}
namespace fw = frostwave;

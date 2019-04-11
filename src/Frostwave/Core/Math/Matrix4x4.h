#pragma once
#include <Frostwave/Core/Types.h>
#include "Vector4.h"

namespace frostwave
{
	template <class T>
	class Matrix3x3;

	template <class T>
	class Quaternion;
}

namespace frostwave
{
	template <class T>
	class Matrix3x3;

	template <class T>
	class Quaternion;

	template <class T>
	class Matrix4x4
	{
	public:
		friend class Matrix3x3<T>;
		Matrix4x4<T>();
		Matrix4x4(const std::initializer_list<T>& aInitList);
		Matrix4x4<T>(const Matrix4x4<T>& aMatrix);

		const Vector3<T> GetPosition() const;
		void SetPosition(const Vector3<T>& aPosition);

		const Vector3<T> GetEulerAngles() const;
		const Vector3<T> GetScale() const;

		static Matrix4x4<T> CreateRotationAroundX(T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundY(T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundZ(T aAngleInRadians);

		static Matrix4x4<T> CreatePerspectiveProjection(T aFov, T aAspect, T aNear, T aFar);
		static Matrix4x4<T> CreateOrthographicProjection(T aWidth, T aHeight, T aAspect, T aNear, T aFar);
		static Matrix4x4<T> CreateTransform(const Vector3<T>& aPosition, const Quaternion<T>& aRotation, const Vector3<T>& aScale);
		static Matrix4x4<T> CreateLookAt(const Vector3<T>& aLookAt, const Vector3<T>& aEye, const Vector3<T>& aUp);

		static Matrix4x4<T> Transpose(const Matrix4x4<T>& aMatrixToTranspose);
		static T Determinant(const Matrix4x4<T>& aMatrix);
		static Matrix4x4<T> Inverse(const Matrix4x4<T>& aMatrixToInvert);
		static Matrix4x4<T> FastInverse(const Matrix4x4<T>& aMatrixToInvert);

		inline Matrix4x4<T> GetInversed() const;

		static inline Matrix4x4<T> CreateTransformationMatrix(const Vector3<T>& aPosition, const Vector3<T>& aRotation, const Vector3<T>& aScale);
		static inline Matrix4x4<T> CreateScaleMatrix(const T aX, const T aY, const T aZ);
		static inline Matrix4x4<T> CreateScaleMatrix(const Vector3<T>& aScale);
		static inline Matrix4x4<T> CreateTranslationMatrix(const T aX, const T aY, const T aZ);
		static inline Matrix4x4<T> CreateTranslationMatrix(const Vector3<T>& aPosition);
		static inline Matrix4x4<T> CreateYawPitchRollMatrix(const T aYawAngleInRadians, const T aPitchAngleInRadians, const T aRollAngleInRadians);
		static inline Matrix4x4<T> CreateYawPitchRollMatrix(const Vector3<T>& aRotation);

		inline Matrix4x4<T>& operator=(const Matrix4x4<T>& aMatrix);
		inline T operator()(unsigned short aIndex0, unsigned short aIndex1) const { return myNumbers[aIndex0 - 1 + (aIndex1 - 1) * 4]; }
		inline T& operator()(unsigned short aIndex0, unsigned short aIndex1) { return myNumbers[aIndex0 - 1 + (aIndex1 - 1) * 4]; }
		inline Matrix4x4<T> operator+(const Matrix4x4<T>& aMatrix) const;
		inline Matrix4x4<T> operator-(const Matrix4x4<T>& aMatrix) const;
		inline Matrix4x4<T> operator*(const Matrix4x4<T>& aMatrix) const;
		inline Matrix4x4<T> operator*(const T& aScalar) const;
		inline bool operator==(const Matrix4x4<T>& aMatrix) const;

		inline void operator+=(const Matrix4x4<T>& aMatrix) { *this = *this + aMatrix; }
		inline void operator-=(const Matrix4x4<T>& aMatrix) { *this = *this - aMatrix; }
		inline void operator*=(const Matrix4x4<T>& aMatrix) { *this = *this * aMatrix; }
		inline void operator*=(const T& aScalar) { *this = *this * aScalar; };

		friend inline Vector4<T> operator*(const Vector4<T>& aVector, const Matrix4x4<T>& aMatrix)
		{
			return Vector4<T>(
				aVector.x * aMatrix.myNumbers[0] + aVector.y * aMatrix.myNumbers[4] + aVector.z * aMatrix.myNumbers[8] + aVector.w * aMatrix.myNumbers[12],
				aVector.x * aMatrix.myNumbers[1] + aVector.y * aMatrix.myNumbers[5] + aVector.z * aMatrix.myNumbers[9] + aVector.w * aMatrix.myNumbers[13],
				aVector.x * aMatrix.myNumbers[2] + aVector.y * aMatrix.myNumbers[6] + aVector.z * aMatrix.myNumbers[10] + aVector.w * aMatrix.myNumbers[14],
				aVector.x * aMatrix.myNumbers[3] + aVector.y * aMatrix.myNumbers[7] + aVector.z * aMatrix.myNumbers[11] + aVector.w * aMatrix.myNumbers[15]
				);
		}
		friend inline void operator*=(Vector4<T>& aVector, const Matrix4x4<T>& aMatrix) { aVector = aVector * aMatrix; };

		friend inline Vector3<T> operator*(const Vector3<T>& aVector, const Matrix4x4<T>& aMatrix) // >:)
		{
			return Vector3<T>(
				aVector.x * aMatrix.myNumbers[0] + aVector.y * aMatrix.myNumbers[4] + aVector.z * aMatrix.myNumbers[8] + aMatrix.myNumbers[12],
				aVector.x * aMatrix.myNumbers[1] + aVector.y * aMatrix.myNumbers[5] + aVector.z * aMatrix.myNumbers[9] + aMatrix.myNumbers[13],
				aVector.x * aMatrix.myNumbers[2] + aVector.y * aMatrix.myNumbers[6] + aVector.z * aMatrix.myNumbers[10] + aMatrix.myNumbers[14]
				);
		}
		friend inline void operator*=(Vector3<T>& aVector, const Matrix4x4<T>& aMatrix) { aVector = aVector * aMatrix; };

#pragma warning(disable : 4201)
#pragma warning(disable : 26495)
		union
		{
			struct { T myNumbers[16]; };
			Vector4<T> myRows[4];
			struct
			{
				__m128 m1;
				__m128 m2;
				__m128 m3;
				__m128 m4;
			};
			struct
			{
				Vector3<T> myRightAxis; T myRightW;
				Vector3<T> myUpAxis; T myUpW;
				Vector3<T> myForwardAxis; T myForwardW;
				Vector3<T> myPosition; T myW;
			};
		};
#pragma warning(default : 26495)
#pragma warning(default : 4201)

		inline T & operator[](const u32& aIndex);
		inline const T & operator[](const u32& aIndex) const;
	};

	template<class T>
	inline Matrix4x4<T>::Matrix4x4() : myNumbers{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }
	{
	}
	template<class T>
	inline Matrix4x4<T>::Matrix4x4(const std::initializer_list<T>& aInitList)
	{
		assert(aInitList.size() == 16 && "Initializer list for Matrix4x4 must contain exactly 16 elements.");
		auto begin = aInitList.begin();
		for (size_t i = 0; i < 16; ++i)
		{
			myNumbers[i] = *(begin + i);
		}
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundX(T aAngleInRadians)
	{
		T c = cos(aAngleInRadians);
		T s = sin(aAngleInRadians);
		return Matrix4x4<T>({
			1, 0, 0, 0,
			0, c, s, 0,
			0, -s, c, 0,
			0, 0, 0, 1
			});
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundY(T aAngleInRadians)
	{
		T c = cos(aAngleInRadians);
		T s = sin(aAngleInRadians);
		return Matrix4x4<T>({
			c, 0, -s, 0,
			0, 1, 0, 0,
			s, 0, c, 0,
			0, 0, 0, 1
			});
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundZ(T aAngleInRadians)
	{
		T c = cos(aAngleInRadians);
		T s = sin(aAngleInRadians);
		return Matrix4x4<T>({
			c, s, 0, 0,
			-s, c, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
			});
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreatePerspectiveProjection(T aFov, T aAspect, T aNear, T aFar)
	{
		T yFov = aFov / aAspect;

		T B = (T)1.0f / (std::tan(fw::ToRadians(yFov) * (T)0.5f));
		T A = B / aAspect;
		T C = aFar / (aFar - aNear);
		T D = (T)1.0f;
		T E = -aNear * aFar / (aFar - aNear);

		return fw::Matrix4x4<T>{
			A, 0, 0, 0,
			0,-B, 0, 0,
			0, 0, C, D,
			0, 0, E, 0
		};
	}

	template<class T>
	inline Matrix4x4<T>::Matrix4x4(const Matrix4x4<T>& aMatrix)
	{
		*this = aMatrix;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateOrthographicProjection(T aWidth, T aHeight, T aAspect, T aNear, T aFar)
	{
		T B = (T)2.0 / aHeight;
		T A = (T)2.0 / (aWidth * aAspect);
		T C = (T)1.0 / (aFar - aNear);
		T E = aNear / (aNear - aFar);
		return Matrix4x4<T>{
			A, 0, 0, 0,
				0, B, 0, 0,
				0, 0, C, 0,
				0, 0, E, 1
		};
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateTransform(const Vector3<T>& aPosition, const Quaternion<T>& aRotation, const Vector3<T>& aScale)
	{
		fw::Matrix4x4<T> scale = {
			aScale.x, 0, 0, 0,
			0, aScale.y, 0, 0,
			0, 0, aScale.z, 0,
			0, 0, 0, 1,
		};

		fw::Matrix4x4<T> rotation = aRotation.GetRotationMatrix44();

		fw::Matrix4x4<T> translation = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			aPosition.x, aPosition.y, aPosition.z, 1
		};
		return scale * rotation * translation;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateLookAt(const Vector3<T>& aLookAt, const Vector3<T>& aEye, const Vector3<T>& aUp)
	{
		Vector3<T> zaxis = (aLookAt - aEye).GetNormalized();
		Vector3<T> xaxis = (aUp.Cross(zaxis)).GetNormalized();
		Vector3<T> yaxis = zaxis.Cross(xaxis);
		return Matrix4x4<T>{
			xaxis.x,			yaxis.x,			zaxis.x,			0,
			xaxis.y,			yaxis.y,			zaxis.y,			0,
			xaxis.z,			yaxis.z,			zaxis.z,			0,
			-xaxis.Dot(aEye), -yaxis.Dot(aEye), -zaxis.Dot(aEye),	1
		};
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::Transpose(const Matrix4x4<T>& aMatrixToTranspose)
	{
		Matrix4x4<T> result(aMatrixToTranspose);

		result[1] = aMatrixToTranspose[4];
		result[4] = aMatrixToTranspose[1];

		result[2] = aMatrixToTranspose[8];
		result[8] = aMatrixToTranspose[2];

		result[3] = aMatrixToTranspose[12];
		result[12] = aMatrixToTranspose[3];

		result[6] = aMatrixToTranspose[9];
		result[9] = aMatrixToTranspose[6];

		result[7] = aMatrixToTranspose[13];
		result[13] = aMatrixToTranspose[7];

		result[11] = aMatrixToTranspose[14];
		result[14] = aMatrixToTranspose[11];

		return result;
	}
	template<class T>
	inline T Matrix4x4<T>::Determinant(const Matrix4x4<T>& aMatrix)
	{
		return aMatrix.myNumbers[0] * ((aMatrix.myNumbers[5] * aMatrix.myNumbers[10] * aMatrix.myNumbers[15] + aMatrix.myNumbers[6] * aMatrix.myNumbers[11] * aMatrix.myNumbers[13] + aMatrix.myNumbers[9] * aMatrix.myNumbers[14] * aMatrix.myNumbers[7]) - (aMatrix.myNumbers[7] * aMatrix.myNumbers[10] * aMatrix.myNumbers[13] + aMatrix.myNumbers[11] * aMatrix.myNumbers[14] * aMatrix.myNumbers[5] + aMatrix.myNumbers[6] * aMatrix.myNumbers[9] * aMatrix.myNumbers[15])) -
			aMatrix.myNumbers[1] * ((aMatrix.myNumbers[4] * aMatrix.myNumbers[10] * aMatrix.myNumbers[15] + aMatrix.myNumbers[6] * aMatrix.myNumbers[11] * aMatrix.myNumbers[12] + aMatrix.myNumbers[8] * aMatrix.myNumbers[14] * aMatrix.myNumbers[7]) - (aMatrix.myNumbers[7] * aMatrix.myNumbers[10] * aMatrix.myNumbers[12] + aMatrix.myNumbers[11] * aMatrix.myNumbers[14] * aMatrix.myNumbers[4] + aMatrix.myNumbers[6] * aMatrix.myNumbers[8] * aMatrix.myNumbers[15])) +
			aMatrix.myNumbers[2] * ((aMatrix.myNumbers[4] * aMatrix.myNumbers[9] * aMatrix.myNumbers[15] + aMatrix.myNumbers[5] * aMatrix.myNumbers[11] * aMatrix.myNumbers[12] + aMatrix.myNumbers[8] * aMatrix.myNumbers[13] * aMatrix.myNumbers[7]) - (aMatrix.myNumbers[7] * aMatrix.myNumbers[9] * aMatrix.myNumbers[12] + aMatrix.myNumbers[11] * aMatrix.myNumbers[13] * aMatrix.myNumbers[4] + aMatrix.myNumbers[5] * aMatrix.myNumbers[8] * aMatrix.myNumbers[15])) -
			aMatrix.myNumbers[3] * ((aMatrix.myNumbers[4] * aMatrix.myNumbers[9] * aMatrix.myNumbers[14] + aMatrix.myNumbers[5] * aMatrix.myNumbers[10] * aMatrix.myNumbers[12] + aMatrix.myNumbers[8] * aMatrix.myNumbers[13] * aMatrix.myNumbers[6]) - (aMatrix.myNumbers[6] * aMatrix.myNumbers[9] * aMatrix.myNumbers[12] + aMatrix.myNumbers[10] * aMatrix.myNumbers[13] * aMatrix.myNumbers[4] + aMatrix.myNumbers[5] * aMatrix.myNumbers[8] * aMatrix.myNumbers[14]));
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::Inverse(const Matrix4x4<T>& aMatrixToInvert)
	{
		T det = Determinant(aMatrixToInvert);
		assert(det != 0 && "Non-invertible matrix");
		Matrix4x4<T> result;
		result.myNumbers[0] = aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[10] * aMatrixToInvert.myNumbers[15] -
			aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[11] * aMatrixToInvert.myNumbers[14] -
			aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[15] +
			aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[14] +
			aMatrixToInvert.myNumbers[13] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[11] -
			aMatrixToInvert.myNumbers[13] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[10];

		result.myNumbers[4] = -aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[10] * aMatrixToInvert.myNumbers[15] +
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[11] * aMatrixToInvert.myNumbers[14] +
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[15] -
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[14] -
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[11] +
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[10];

		result.myNumbers[8] = aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[15] -
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[11] * aMatrixToInvert.myNumbers[13] -
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[15] +
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[13] +
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[11] -
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[9];

		result.myNumbers[12] = -aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[14] +
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[10] * aMatrixToInvert.myNumbers[13] +
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[14] -
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[13] -
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[10] +
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[9];

		result.myNumbers[1] = -aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[10] * aMatrixToInvert.myNumbers[15] +
			aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[11] * aMatrixToInvert.myNumbers[14] +
			aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[15] -
			aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[14] -
			aMatrixToInvert.myNumbers[13] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[11] +
			aMatrixToInvert.myNumbers[13] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[10];

		result.myNumbers[5] = aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[10] * aMatrixToInvert.myNumbers[15] -
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[11] * aMatrixToInvert.myNumbers[14] -
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[15] +
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[14] +
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[11] -
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[10];

		result.myNumbers[9] = -aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[15] +
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[11] * aMatrixToInvert.myNumbers[13] +
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[15] -
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[13] -
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[11] +
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[9];

		result.myNumbers[13] = aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[14] -
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[10] * aMatrixToInvert.myNumbers[13] -
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[14] +
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[13] +
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[10] -
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[9];

		result.myNumbers[2] = aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[15] -
			aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[14] -
			aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[15] +
			aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[14] +
			aMatrixToInvert.myNumbers[13] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[7] -
			aMatrixToInvert.myNumbers[13] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[6];

		result.myNumbers[6] = -aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[15] +
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[14] +
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[15] -
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[14] -
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[7] +
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[6];

		result.myNumbers[10] = aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[15] -
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[13] -
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[15] +
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[13] +
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[7] -
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[5];

		result.myNumbers[14] = -aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[14] +
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[13] +
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[14] -
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[13] -
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[6] +
			aMatrixToInvert.myNumbers[12] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[5];

		result.myNumbers[3] = -aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[11] +
			aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[10] +
			aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[11] -
			aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[10] -
			aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[7] +
			aMatrixToInvert.myNumbers[9] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[6];

		result.myNumbers[7] = aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[11] -
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[10] -
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[11] +
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[10] +
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[7] -
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[6];

		result.myNumbers[11] = -aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[11] +
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[7] * aMatrixToInvert.myNumbers[9] +
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[11] -
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[9] -
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[7] +
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[3] * aMatrixToInvert.myNumbers[5];

		result.myNumbers[15] = aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[5] * aMatrixToInvert.myNumbers[10] -
			aMatrixToInvert.myNumbers[0] * aMatrixToInvert.myNumbers[6] * aMatrixToInvert.myNumbers[9] -
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[10] +
			aMatrixToInvert.myNumbers[4] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[9] +
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[1] * aMatrixToInvert.myNumbers[6] -
			aMatrixToInvert.myNumbers[8] * aMatrixToInvert.myNumbers[2] * aMatrixToInvert.myNumbers[5];
		return result * (1 / det);
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::FastInverse(const Matrix4x4<T>& aMatrixToInvert)
	{
		fw::Matrix3x3<T> transposed = aMatrixToInvert;
		transposed = fw::Matrix3x3<T>::Transpose(transposed);
		fw::Vec3f negated(-aMatrixToInvert.myNumbers[12], -aMatrixToInvert.myNumbers[13], -aMatrixToInvert.myNumbers[14]);
		negated *= transposed;
		return fw::Matrix4x4<T>({
			transposed.myNumbers[0],	transposed.myNumbers[1],	transposed.myNumbers[2],	0,
			transposed.myNumbers[3],	transposed.myNumbers[4],	transposed.myNumbers[5],	0,
			transposed.myNumbers[6],	transposed.myNumbers[7],	transposed.myNumbers[8],	0,
			negated.x,					negated.y,					negated.z,					1
			});
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::GetInversed() const
	{
		return Inverse(*this);
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateTransformationMatrix(const Vector3<T>& aPosition, const Vector3<T>& aRotation, const Vector3<T>& aScale)
	{
		return CreateScaleMatrix(aScale) * CreateYawPitchRollMatrix(aRotation) * CreateTranslationMatrix(aPosition);
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateScaleMatrix(const T aX, const T aY, const T aZ)
	{
		Matrix4x4<T> result;
		result.myNumbers[0] = aX;
		result.myNumbers[5] = aY;
		result.myNumbers[10] = aZ;

		return result;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateScaleMatrix(const Vector3<T>& aScale)
	{
		return CreateScaleMatrix(aScale.x, aScale.y, aScale.z);
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateTranslationMatrix(const T aX, const T aY, const T aZ)
	{
		Matrix4x4<T> result;
		result.myPosition.x = aX;
		result.myPosition.y = aY;
		result.myPosition.z = aZ;

		return result;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateTranslationMatrix(const Vector3<T>& aPosition)
	{
		return CreateTranslationMatrix(aPosition.x, aPosition.y, aPosition.z);
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateYawPitchRollMatrix(const T aYawAngleInRadians, const T aPitchAngleInRadians, const T aRollAngleInRadians)
	{
		Matrix4x4<T> m;

		T cosYawAngle = cosf(aYawAngleInRadians);
		T cosRollAngle = cosf(aRollAngleInRadians);
		T cosPitchAngle = cosf(aPitchAngleInRadians);
		T sinYawAngle = sinf(aYawAngleInRadians);
		T sinRollAngle = sinf(aRollAngleInRadians);
		T sinPitchAngle = sinf(aPitchAngleInRadians);

		m.myNumbers[0] = (cosYawAngle * cosRollAngle);
		m.myNumbers[1] = (cosYawAngle * sinRollAngle);
		m.myNumbers[2] = (-sinYawAngle);
		m.myNumbers[3] = T(0);
		m.myNumbers[4] = (sinPitchAngle * sinYawAngle * cosRollAngle + cosPitchAngle * -sinRollAngle);
		m.myNumbers[5] = (sinPitchAngle * sinYawAngle * sinRollAngle + cosPitchAngle * cosRollAngle);
		m.myNumbers[6] = (sinPitchAngle * cosYawAngle);
		m.myNumbers[7] = T(0);
		m.myNumbers[8] = (cosPitchAngle * sinYawAngle * cosRollAngle + -sinPitchAngle * -sinRollAngle);
		m.myNumbers[9] = (cosPitchAngle * sinYawAngle * sinRollAngle + -sinPitchAngle * cosRollAngle);
		m.myNumbers[10] = (cosPitchAngle * cosYawAngle);
		m.myNumbers[11] = T(0);
		m.myNumbers[12] = T(0);
		m.myNumbers[13] = T(0);
		m.myNumbers[14] = T(0);
		m.myNumbers[15] = T(1);

		return m;
	}
	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateYawPitchRollMatrix(const Vector3<T>& aRotation)
	{
		return CreateYawPitchRollMatrix(aRotation.y, aRotation.x, aRotation.z);
	}
	template<class T>
	Matrix4x4<T>& Matrix4x4<T>::operator=(const Matrix4x4<T>& aMatrix)
	{
		for (size_t i = 0; i < 16; ++i)
		{
			myNumbers[i] = aMatrix.myNumbers[i];
		}
		return *this;
	}
	template<class T>
	Matrix4x4<T> Matrix4x4<T>::operator+(const Matrix4x4<T>& aMatrix) const
	{
		Matrix4x4<T> result;
		for (size_t i = 0; i < 16; ++i)
		{
			result.myNumbers[i] = myNumbers[i] + aMatrix.myNumbers[i];
		}
		return result;
	}
	template<class T>
	Matrix4x4<T> Matrix4x4<T>::operator-(const Matrix4x4<T>& aMatrix) const
	{
		Matrix4x4<T> result;
		for (size_t i = 0; i < 16; ++i)
		{
			result.myNumbers[i] = myNumbers[i] - aMatrix.myNumbers[i];
		}
		return result;
	}
	template<class T>
	Matrix4x4<T> Matrix4x4<T>::operator*(const Matrix4x4<T>& aMatrix) const
	{
		Matrix4x4<T> result;
		for (size_t i = 0; i <= 12; i += 4)
		{
			for (size_t j = 0; j < 4; ++j)
			{
				result.myNumbers[i + j] =
					myNumbers[i] * aMatrix.myNumbers[j] +
					myNumbers[1 + i] * aMatrix.myNumbers[4 + j] +
					myNumbers[2 + i] * aMatrix.myNumbers[8 + j] +
					myNumbers[3 + i] * aMatrix.myNumbers[12 + j];
			}
		}
		return result;
	}

	template<class T>
	inline Matrix4x4<T> Matrix4x4<T>::operator*(const T & aScalar) const
	{
		Matrix4x4<T> result;
		for (size_t i = 0; i < 16; ++i)
		{
			result.myNumbers[i] = myNumbers[i] * aScalar;
		}
		return result;
	}
	template<class T>
	bool Matrix4x4<T>::operator==(const Matrix4x4<T>& aMatrix) const
	{
		for (size_t i = 0; i < 16; ++i)
		{
			if (myNumbers[i] != aMatrix.myNumbers[i])
			{
				return false;
			}
		}
		return true;
	}

	template<class T>
	const Vector3<T> Matrix4x4<T>::GetPosition() const
	{
		return fw::Vec3f(myNumbers[12], myNumbers[13], myNumbers[14]);;
	}

	template<class T>
	void Matrix4x4<T>::SetPosition(const Vector3<T>& aVector)
	{
		myNumbers[12] = aVector.x;
		myNumbers[13] = aVector.y;
		myNumbers[14] = aVector.z;
	}

	template<class T>
	inline const Vector3<T> Matrix4x4<T>::GetEulerAngles() const
	{
		return Quaternion<T>(*this).GetEulerAngles();
	}

	template<class T>
	inline const Vector3<T> Matrix4x4<T>::GetScale() const
	{
		return fw::Vec3f(myNumbers[0], myNumbers[5], myNumbers[10]);
	}


	template <class T>
	inline T & Matrix4x4<T>::operator[](const u32& aIndex)
	{
		assert((aIndex < 16) && "Index out of bounds.");
		return myNumbers[aIndex];
	}

	template <class T>
	const inline T& Matrix4x4<T>::operator[](const u32& aIndex) const
	{
		assert((aIndex < 16) && "Index out of bounds.");
		return myNumbers[aIndex];
	}

	typedef Matrix4x4<f32> Mat4f;
}
namespace fw = frostwave;

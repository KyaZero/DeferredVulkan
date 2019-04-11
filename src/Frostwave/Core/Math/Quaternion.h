#pragma once

namespace frostwave
{
	template <class T>
	class Matrix3x3;

	template <class T>
	class Matrix4x4;

	template <class T>
	class Vector3;
}

#include <Frostwave/Core/Common.h>
#include <Frostwave/Core/Math/Matrix4x4.h>
#ifdef _DEBUG
#include <iostream>
#endif // !_RELEASE
namespace frostwave
{
	template <class T>
	class Quaternion
	{
	public:
#pragma warning( disable : 4201 )
		union
		{
			T myValues[4];
			struct { T w; T x; T y; T z; };
		};
#pragma warning( default : 4201 )

		Quaternion<T>();
		Quaternion<T>(const T& aW, const T& aX, const T& aY, const T& aZ);
		Quaternion<T>(const T& aYaw, const T& aPitch, const T& aRoll);
		explicit Quaternion<T>(const Vector3<T>& aYawPitchRoll);
		Quaternion<T>(const Vector3<T>& aVector, const T aAngle);
		Quaternion<T>(const Matrix4x4<T>& aMatrix);

		//Set from Unity values - Flip X and Z values, W and Y remains the same
		void SetFromUnityValues(const T aW, const T aX, const T aY, const T aZ);

		inline void Normalize();
		inline Quaternion<T> GetNormalized() const;
		inline Quaternion<T> GetConjugate() const;

		Quaternion<T>& operator=(const Quaternion<T>& aQuat) = default;

		T Length() const;
		T Length2() const;
		inline Vector3<T> GetEulerAngles() const;
		inline Matrix3x3<T> GetRotationMatrix33() const;
		inline Matrix4x4<T> GetRotationMatrix44() const;
		inline T Dot(const Quaternion<T>& aQuat) const;
		inline Vector3<T> GetForwardVector() const;
		inline Vector3<T> GetUpVector() const;
		inline Vector3<T> GetRightVector() const;

		inline static Quaternion<T> Slerp(const Quaternion<T>& aQuatA, const Quaternion<T>& aQuatB, const T& aDelta);
	};

	template<class T>
	inline Quaternion<T>::Quaternion()
	{
		w = static_cast<T>(1);
		x = static_cast<T>(0);
		y = static_cast<T>(0);
		z = static_cast<T>(0);
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const T & aW, const T & aX, const T & aY, const T & aZ) : w(aW), x(aX), y(aY), z(aZ)
	{
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const T & aYaw, const T & aPitch, const T & aRoll)
	{
		T cy = cos(aYaw * T(0.5));
		T sy = sin(aYaw * T(0.5));
		T cr = cos(aRoll * T(0.5));
		T sr = sin(aRoll * T(0.5));
		T cp = cos(aPitch * T(0.5));
		T sp = sin(aPitch * T(0.5));

		w = cy * cr * cp + sy * sr * sp;
		x = cy * sr * cp - sy * cr * sp;
		y = cy * cr * sp + sy * sr * cp;
		z = sy * cr * cp - cy * sr * sp;
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const Vector3<T>& aYawPitchRoll)
	{
		T cx = cos(aYawPitchRoll.x * T(0.5));
		T cy = cos(aYawPitchRoll.y * T(0.5));
		T cz = cos(aYawPitchRoll.z * T(0.5));
		T sx = sin(aYawPitchRoll.x * T(0.5));
		T sy = sin(aYawPitchRoll.y * T(0.5));
		T sz = sin(aYawPitchRoll.z * T(0.5));

		w = cx * cy * cz + sx * sy * sz;
		x = sx * cy * cz - cx * sy * sz;
		y = cx * sy * cz + sx * cy * sz;
		z = cx * cy * sz - sx * sy * cz;
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const Vector3<T>& aVector, const T aAngle)
	{
		auto norm = aVector.GetNormalized();
		T halfAngle = aAngle / T(2);
		w = cos(halfAngle);
		T halfAngleSin = sin(halfAngle);
		x = norm.x * halfAngleSin;
		y = norm.y * halfAngleSin;
		z = norm.z * halfAngleSin;
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const Matrix4x4<T>& aMatrix)
	{
		w = std::sqrt(fw::Max(T(0), T(1) + aMatrix.myNumbers[0] + aMatrix.myNumbers[5] + aMatrix.myNumbers[10])) * T(0.5);
		x = std::sqrt(fw::Max(T(0), T(1) + aMatrix.myNumbers[0] - aMatrix.myNumbers[5] - aMatrix.myNumbers[10])) * T(0.5);
		y = std::sqrt(fw::Max(T(0), T(1) - aMatrix.myNumbers[0] + aMatrix.myNumbers[5] - aMatrix.myNumbers[10])) * T(0.5);
		z = std::sqrt(fw::Max(T(0), T(1) - aMatrix.myNumbers[0] - aMatrix.myNumbers[5] + aMatrix.myNumbers[10])) * T(0.5);
		x = std::copysign(x, aMatrix.myNumbers[9] - aMatrix.myNumbers[6]);
		y = std::copysign(y, aMatrix.myNumbers[2] - aMatrix.myNumbers[8]);
		z = std::copysign(z, aMatrix.myNumbers[4] - aMatrix.myNumbers[1]);
	}

	template<class T>
	inline void Quaternion<T>::SetFromUnityValues(const T aW, const T aX, const T aY, const T aZ)
	{
		w = aW;
		x = -aX;
		y = aY;
		z = -aZ;
	}

	template<class T>
	inline void Quaternion<T>::Normalize()
	{
		T length = T(1) / Length();
		w *= length;
		x *= length;
		y *= length;
		z *= length;
	}

	template<class T>
	inline Quaternion<T> Quaternion<T>::GetNormalized() const
	{
		T length = T(1) / Length();
		return Quaternion<T>(w * length, x * length, y * length, z * length);
	}

	template<class T>
	inline Quaternion<T> Quaternion<T>::GetConjugate() const
	{
		return Quaternion<T>(w, -x, -y, -z);
	}

	template<class T>
	inline T Quaternion<T>::Length() const
	{
		return sqrt(Length2());
	}

	template<class T>
	inline T Quaternion<T>::Length2() const
	{
		return (x * x) + (y * y) + (z * z) + (w * w);
	}

	template<class T>
	inline Vector3<T> Quaternion<T>::GetEulerAngles() const
	{
		// roll (z-axis rotation)
		T sinr = T(2.0) * (w * x + y * z);
		T cosr = T(1.0) - T(2.0) * (x * x + y * y);
		T roll = atan2(sinr, cosr);

		// pitch (x-axis rotation)
		T sinp = T(2.0) * (w * y - z * x);
		T pitch = T(0.0);
		if (abs(sinp) >= T(1))
		{
			pitch = copysign(PI * T(0.5), sinp); // Default to 90 degrees if out of range.
		}
		else
		{
			pitch = asin(sinp);
		}

		// yaw (y-axis rotation)
		T siny = T(2.0) * (w * z + x * y);
		T cosy = T(1.0) - T(2.0) * (y * y + z * z);
		T yaw = atan2(siny, cosy);

		return Vector3<T>(roll, pitch, yaw);
	}

	template<class T>
	inline Matrix3x3<T> Quaternion<T>::GetRotationMatrix33() const
	{
		Matrix3x3<T> result;
		T qxx(x * x);
		T qyy(y * y);
		T qzz(z * z);
		T qxz(x * z);
		T qxy(x * y);
		T qyz(y * z);
		T qwx(w * x);
		T qwy(w * y);
		T qwz(w * z);

		result.myNumbers[0] = T(1) - T(2) * (qyy + qzz);
		result.myNumbers[3] = T(2) * (qxy + qwz);
		result.myNumbers[6] = T(2) * (qxz - qwy);

		result.myNumbers[1] = T(2) * (qxy - qwz);
		result.myNumbers[4] = T(1) - T(2) * (qxx + qzz);
		result.myNumbers[7] = T(2) * (qyz + qwx);

		result.myNumbers[2] = T(2) * (qxz + qwy);
		result.myNumbers[5] = T(2) * (qyz - qwx);
		result.myNumbers[8] = T(1) - T(2) * (qxx + qyy);

// 		result.m11 = T(1) - T(2) * (qyy + qzz);   Didn't compile so martin commented out
// 		result.m21 = T(2) * (qxy + qwz);
// 		result.m31 = T(2) * (qxz - qwy);
// 
// 		result.m12 = T(2) * (qxy - qwz);
// 		result.m22 = T(1) - T(2) * (qxx + qzz);
// 		result.m32 = T(2) * (qyz + qwx);
// 
// 		result.m13 = T(2) * (qxz + qwy);
// 		result.m23 = T(2) * (qyz - qwx);
// 		result.m33 = T(1) - T(2) * (qxx + qyy);
		return result;
	}
	template<class T>
	inline Matrix4x4<T> Quaternion<T>::GetRotationMatrix44() const
	{
		Matrix4x4<T> result;
		T qxx(x * x);
		T qyy(y * y);
		T qzz(z * z);

		T qxz(x * z);
		T qxy(x * y);
		T qyz(y * z);

		T qwx(w * x);
		T qwy(w * y);
		T qwz(w * z);


		result.myNumbers[0] = T(1) - T(2) * (qyy + qzz);
		result.myNumbers[4] = T(2) * (qxy + qwz);
		result.myNumbers[8] = T(2) * (qxz - qwy);

		result.myNumbers[1] = T(2) * (qxy - qwz);
		result.myNumbers[5] = T(1) - T(2) * (qxx + qzz);
		result.myNumbers[9] = T(2) * (qyz + qwx);

		result.myNumbers[2] = T(2) * (qxz + qwy);
		result.myNumbers[6] = T(2) * (qyz - qwx);
		result.myNumbers[10] = T(1) - T(2) * (qxx + qyy);

		return fw::Mat4f::Transpose(result);
	}

	template<class T>
	inline T Quaternion<T>::Dot(const Quaternion<T>& aQuat) const
	{
		return x * aQuat.x + y * aQuat.y + z * aQuat.z + w * aQuat.w;
	}

	template<class T>
	inline Vector3<T> Quaternion<T>::GetForwardVector() const
	{
		return Vector3<T>(2 * (x * z + w * y),
			2 * (y * z - w * x),
			1 - 2 * (x * x + y * y));
	}
	template<class T>
	inline Vector3<T> Quaternion<T>::GetUpVector() const
	{
		return Vector3<T>(2 * (x * y - w * z),
			1 - 2 * (x* x + z * z),
			2 * (y * z + w * x));
	}
	template<class T>
	inline Vector3<T> Quaternion<T>::GetRightVector() const
	{
		return Vector3<T>(1 - 2 * (y * y + z * z),
			2 * (x * y + w * z),
			2 * (x * z - w * y));
	}

	template<class T>
	inline Quaternion<T> Quaternion<T>::Slerp(const Quaternion<T>& aQuatA, const Quaternion<T>& aQuatB, const T & aDelta)
	{
		Quaternion<T> qz = aQuatB;

		T cosTheta = aQuatA.Dot(aQuatB);

		// If cosTheta < 0, the interpolation will take the long way around the sphere. 
		// To fix this, one quat must be negated.
		if (cosTheta < T(0))
		{
			qz = -qz;
			cosTheta = -cosTheta;
		}

		const T dotThreshold = static_cast<T>(0.9995);
		// Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
		if (cosTheta > T(1) - dotThreshold)
		{
			// Linear interpolation
			return Lerp(aQuatA, qz, aDelta);
		}
		else
		{
			// Essential Mathematics, page 467
			T angle = acos(cosTheta);
			return (sin((T(1) - aDelta) * angle) * aQuatA + sin(aDelta * angle) * qz) / sin(angle);
		}
	}

	typedef Quaternion<f32> Quaternionf;
	typedef Quaternion<f32> Quatf;
}
namespace fw = frostwave;

template <class T> fw::Quaternion<T> operator*(const fw::Quaternion<T>& aQuat, const T& aScalar)
{
	return fw::Quaternion<T>(aQuat.w * aScalar, aQuat.x * aScalar, aQuat.y * aScalar, aQuat.z * aScalar);
}

template <class T> fw::Quaternion<T> operator*(const T& aScalar, const fw::Quaternion<T>& aQuat)
{
	return fw::Quaternion<T>(aQuat.w * aScalar, aQuat.x * aScalar, aQuat.y * aScalar, aQuat.z * aScalar);
}

template <class T> fw::Quaternion<T> operator*(const fw::Quaternion<T>& aQuat0, const fw::Quaternion<T>& aQuat1)
{
	return fw::Quaternion<T>(
		(aQuat1.w * aQuat0.w) - (aQuat1.x * aQuat0.x) - (aQuat1.y * aQuat0.y) - (aQuat1.z * aQuat0.z),
		(aQuat1.w * aQuat0.x) + (aQuat1.x * aQuat0.w) + (aQuat1.y * aQuat0.z) - (aQuat1.z * aQuat0.y),
		(aQuat1.w * aQuat0.y) + (aQuat1.y * aQuat0.w) + (aQuat1.z * aQuat0.x) - (aQuat1.x * aQuat0.z),
		(aQuat1.w * aQuat0.z) + (aQuat1.z * aQuat0.w) + (aQuat1.x * aQuat0.y) - (aQuat1.y * aQuat0.x)
		);
}

template <class T> void operator*=(fw::Quaternion<T>& aQuat, const T& aScalar)
{
	aQuat.w *= aScalar;
	aQuat.x *= aScalar;
	aQuat.y *= aScalar;
	aQuat.z *= aScalar;
}

template <class T> void operator*=(fw::Quaternion<T>& aQuat0, const fw::Quaternion<T>& aQuat1)
{
	T w = aQuat0.w;
	T x = aQuat0.x;
	T y = aQuat0.y;
	T z = aQuat0.z;

	aQuat0.w = (aQuat1.w * w) - (aQuat1.x * x) - (aQuat1.y * y) - (aQuat1.z * z);
	aQuat0.x = (aQuat1.w * x) + (aQuat1.x * w) + (aQuat1.y * z) - (aQuat1.z * y);
	aQuat0.y = (aQuat1.w * y) + (aQuat1.y * w) + (aQuat1.z * x) - (aQuat1.x * z);
	aQuat0.z = (aQuat1.w * z) + (aQuat1.z * w) + (aQuat1.x * y) - (aQuat1.y * x);

}

template <class T> fw::Quaternion<T> operator/(const fw::Quaternion<T>& aQuat, const T& aScalar)
{
	return fw::Quaternion<T>(aQuat.w / aScalar, aQuat.x / aScalar, aQuat.y / aScalar, aQuat.z / aScalar);
}

template <class T> fw::Quaternion<T> operator-(const fw::Quaternion<T>& aQuatA, const fw::Quaternion<T>& aQuatB)
{
	return fw::Quaternion<T>(aQuatA.w - aQuatB.w, aQuatA.x - aQuatB.x, aQuatA.y - aQuatB.y, aQuatA.z - aQuatB.z);
}

template <class T> fw::Quaternion<T> operator-(const fw::Quaternion<T>& aQuat)
{
	return fw::Quaternion<T>(-aQuat.w, -aQuat.x, -aQuat.y, -aQuat.z);
}

template <class T> fw::Quaternion<T> operator+(const fw::Quaternion<T>& aQuatA, const fw::Quaternion<T>& aQuatB)
{
	return fw::Quaternion<T>(aQuatA.w + aQuatB.w, aQuatA.x + aQuatB.x, aQuatA.y + aQuatB.y, aQuatA.z + aQuatB.z);
}

template <class T> void operator+=(fw::Quaternion<T>& aQuatA, const fw::Quaternion<T>& aQuatB)
{
	aQuatA.w += aQuatB.w;
	aQuatA.x += aQuatB.x;
	aQuatA.y += aQuatB.y;
	aQuatA.z += aQuatB.z;
}
#pragma once
#include <cmath>
#include <Frostwave/Core/Types.h>
#include <Frostwave/Core/Math/Vector2.h>
#include <Frostwave/Core/Math/Vector3.h>

namespace frostwave
{
	template <class T>
	class Vector4
	{
	public:
		Vector4();
		Vector4(const T& aX, const T& aY, const T& aZ, const T& aW);
		Vector4(const Vector4& aVector);
		Vector4(const Vector3<T>& aVector, T aW = 1.0);
		~Vector4() = default;

		inline void Set(T aX, T aY, T aZ, T aW);

		inline Vector4& operator=(const Vector4& aVector);
		inline Vector4& operator=(const T aValue);

		inline bool operator==(const Vector4& aVector) const;
		inline bool operator!=(const Vector4& aVector) const;

		T Distance(const Vector4& aVector) const;

		T DistanceSqr(const Vector4& aVector) const;

		T LengthSqr() const;
		T Length() const;

		i32 Hash();

		Vector4 GetNormalized() const;
		void Normalize();

		T Dot(const Vector4& aVector) const;
#pragma warning(disable : 4201)
		union
		{
			T myValues[4];
			struct { T x; T y; T z; T w; };
			struct { Vector3<T> xyz; T w; };
			struct { Vector2<T> xy, zw; };
			struct { T r; T g; T b; T a; };
			struct { Vector3<T> rgb; T a; };
			struct { Vector2<T> rg, ba; };
		};
#pragma warning(default : 4201)

		inline T & operator[](const u32 & aIndex);
		inline const T & operator[](const u32 & aIndex) const;
	};

	template<class T>
	inline Vector4<T>::Vector4() : x(0), y(0), z(0), w(0)
	{
	}

	template<class T>
	inline Vector4<T>::Vector4(const T & aX, const T & aY, const T & aZ, const T & aW) : x(aX), y(aY), z(aZ), w(aW)
	{
	}

	template<class T>
	inline Vector4<T>::Vector4(const Vector4& aVector) : x(aVector.x), y(aVector.y), z(aVector.z), w(aVector.w)
	{
	}

	template<class T>
	inline Vector4<T>::Vector4(const Vector3<T>& aVector, T aW)
	{
		x = aVector.x;
		y = aVector.y;
		z = aVector.z;
		w = aW;
	}

	template<class T>
	inline void Vector4<T>::Set(T aX, T aY, T aZ, T aW)
	{
		x = aX;
		y = aY;
		z = aZ;
		w = aW;
	}

	template<class T>
	inline Vector4<T>& Vector4<T>::operator=(const Vector4& aVector)
	{
		x = aVector.x;
		y = aVector.y;
		z = aVector.z;
		w = aVector.w;
		return *this;
	}

	template<class T>
	inline Vector4<T>& Vector4<T>::operator=(const T aValue)
	{
		x = aValue;
		y = aValue;
		z = aValue;
		w = aValue;
		return *this;
	}

	template<class T>
	inline bool Vector4<T>::operator==(const Vector4& aVector) const
	{
		return (x == aVector.x) && (y == aVector.y) && (w == aVector.w);
	}

	template<class T>
	inline bool Vector4<T>::operator!=(const Vector4& aVector) const
	{
		return (x != aVector.x) || (y != aVector.y) || (z != aVector.z) || (w != aVector.w);
	}

	template<class T>
	inline T Vector4<T>::Distance(const Vector4& aVector) const
	{
		return sqrt(((x - aVector.x) * (x - aVector.x)) + ((y - aVector.y) * (y - aVector.y)) + ((z - aVector.z) * (z - aVector.z)) + ((w - aVector.w)*(w - aVector.w)));
	}

	template<class T>
	inline T Vector4<T>::DistanceSqr(const Vector4& aVector) const
	{
		return (((x - aVector.x) * (x - aVector.x)) + ((y - aVector.y) * (y - aVector.y)) + ((z - aVector.z) * (z - aVector.z)) + ((w - aVector.w)*(w - aVector.w)));
	}

	template<class T>
	inline T Vector4<T>::LengthSqr() const
	{
		return (x * x) + (y * y) + (z * z) + (w * w);
	}

	template<class T>
	inline T Vector4<T>::Length() const
	{
		return sqrt((x * x) + (y * y) + (z * z) + (w * w));
	}

	template<class T>
	inline Vector4<T> Vector4<T>::GetNormalized() const
	{
		if (Length() != 0)
		{
			T len = Length();
			T invlen = 1 / len;
			return Vector4(x * invlen, y * invlen, z * invlen, w * invlen);
		}
		else
		{
			return Vector4(0, 0, 0, 0);
		}
	}

	template<class T>
	inline void Vector4<T>::Normalize()
	{
		*this = GetNormalized();
	}
	template<class T>
	inline T Vector4<T>::Dot(const Vector4& aVector) const
	{
		return (x * aVector.x) + (y * aVector.y) + (z * aVector.z) + (w * aVector.w);
	}

	template<class T>
	inline T & Vector4<T>::operator[](const u32 & aIndex)
	{
		return myValues[aIndex];
	}

	template<class T>
	inline const T & Vector4<T>::operator[](const u32 & aIndex) const
	{
		return myValues[aIndex];
	}

	template<class T>
	inline i32 Vector4<T>::Hash()
	{
		return (i32)((i32)(x * (T)73856093) ^ (i32)(y * (i32)19349663) ^ (i32)(z * (T)83492791) ^ (i32)(w * (T)1300153));
	}

	using Vec4f = Vector4<f32>;
	using Vector4i = Vector4<i32>;
	using Color = Vec4f;
}
namespace fw = frostwave;

template <class T> fw::Vector4<T> operator-(fw::Vector4<T>& aVector) { return fw::Vector4<T>(-aVector.x, -aVector.y, -aVector.z, -aVector.w); }
template <class T> fw::Vector4<T> operator+(const fw::Vector4<T>& aVector0, const fw::Vector4<T>& aVector1) { return fw::Vector4<T>((aVector0.x + aVector1.x), (aVector0.y + aVector1.y), (aVector0.z + aVector1.z), (aVector0.w + aVector1.w)); }
template <class T> fw::Vector4<T> operator-(const fw::Vector4<T>& aVector0, const fw::Vector4<T>& aVector1) { return fw::Vector4<T>((aVector0.x - aVector1.x), (aVector0.y - aVector1.y), (aVector0.z - aVector1.z), (aVector0.w - aVector1.w)); }
template <class T> fw::Vector4<T> operator*(const fw::Vector4<T>& aVector, const T& aScalar) { return fw::Vector4<T>((aVector.x * aScalar), (aVector.y * aScalar), (aVector.z * aScalar), (aVector.w * aScalar)); }
template <class T> fw::Vector4<T> operator*(const T& aScalar, const fw::Vector4<T>& aVector) { return fw::Vector4<T>((aScalar * aVector.x), (aScalar * aVector.y), (aScalar * aVector.z), (aScalar * aVector.w)); }
template <class T> fw::Vector4<T> operator/(const fw::Vector4<T>& aVector, const T& aScalar) { return fw::Vector4<T>((aVector.x / aScalar), (aVector.y / aScalar), (aVector.z / aScalar), (aVector.w / aScalar)); }
template <class T> void operator+=(fw::Vector4<T>& aVector0, const fw::Vector4<T>& aVector1) { aVector0 = aVector0 + aVector1; }
template <class T> void operator-=(fw::Vector4<T>& aVector0, const fw::Vector4<T>& aVector1) { aVector0 = aVector0 - aVector1; }
template <class T> void operator*=(fw::Vector4<T>& aVector, const T& aScalar) { aVector = aVector * aScalar; }
template <class T> void operator/=(fw::Vector4<T>& aVector, const T& aScalar) { aVector = aVector / aScalar; }

template <class T> fw::Vector4<T> operator*(const fw::Vector4<T>& aVector0, const fw::Vector4<T>& aVector1) { return fw::Vector4<T>((aVector0.x * aVector1.x), (aVector0.y * aVector1.y), (aVector0.z * aVector1.z), (aVector0.w * aVector1.w)); }
template <class T> void operator*=(fw::Vector4<T>& aVector0, const fw::Vector4<T>& aVector1) { aVector0 = aVector0 * aVector1; }

template <class T> fw::Vector4<T> operator*(const fw::Vector4<T>& aVector0, const fw::Vector3<T>& aVector1) { return fw::Vector4<T>((aVector0.x * aVector1.x), (aVector0.y * aVector1.y), (aVector0.z * aVector1.z), aVector0.w ); }
template <class T> fw::Vector4<T> operator*(const fw::Vector3<T>& aVector0, const fw::Vector4<T>& aVector1) { return aVector1 * aVector0; }
template <class T> void operator*=(fw::Vector4<T>& aVector0, const fw::Vector3<T>& aVector1) { aVector0 = aVector0 * aVector1; }



template <class T> fw::Vector4<T> operator+(const fw::Vector4<T>& aVector0, const fw::Vector3<T>& aVector1)
{
	return fw::Vector4<T>(aVector0.x + aVector1.x, aVector0.y + aVector1.y, aVector0.z + aVector1.z, aVector0.w);
}

template <class T> void operator+=(fw::Vector4<T>& aVector0, const fw::Vector3<T>& aVector1) { aVector0 = aVector0 + aVector1; }


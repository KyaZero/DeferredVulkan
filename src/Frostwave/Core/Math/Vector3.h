#pragma once
#include <cmath>
#include "Vector2.h"

#include <functional>

namespace frostwave
{
	template <class T>
	class Vector4;

	template <class T>
	class Vector3
	{
	public:
		Vector3();
		Vector3(const T& aX, const T& aY, const T& aZ);
		Vector3(const T& aValue);
		Vector3(const Vector3& aVector);
		//Vector3(const Vector2<T>& aVector, const T& aZ = 0);
		Vector3(const Vector4<T>& aVector);
		~Vector3() = default;

		inline void Set(T aX, T aY, T aZ);

		inline Vector3& operator=(const Vector3& aVector);
		inline Vector3& operator=(const T aValue);

		inline bool operator==(const Vector3& aVector) const;
		inline bool operator!=(const Vector3& aVector) const;
		//inline Vector3 operator*(const Vector3& aVector, const T& aScalar);
		T Distance(const Vector3& aVector) const;

		T DistanceSqr(const Vector3& aVector) const;

		T LengthSqr() const;
		T Length() const;

		Vector3 GetNormalized() const;
		void Normalize();

		T Dot(const Vector3& aVector) const;

		Vector3 Cross(const Vector3& aVector) const;

		i32 Hash() const;

		T Max() const;

		Vector2<T> xz() const { return Vector2<T>(x, z); }

#pragma warning( push )
#pragma warning(disable : 4201)
		union
		{
			T myValues[3];
			struct { T r; T g; T b; };
			struct { T x; T y; T z; };
			struct { Vector2<T> xy; T z; };
			struct { T x; Vector2<T> yz; };
		};
#pragma warning(default : 4201)
#pragma warning( pop )
	};

	template<class T>
	inline Vector3<T>::Vector3() : x(0), y(0), z(0)
	{
	}
	template<class T> inline Vector3<T>::Vector3(const T & aValue) : x(aValue), y(aValue), z(aValue)
	{
	}
	template<class T>
	inline Vector3<T>::Vector3(const T & aX, const T & aY, const T & aZ) : x(aX), y(aY), z(aZ)
	{
	}

	template<class T>
	inline Vector3<T>::Vector3(const Vector3& aVector) : x(aVector.x), y(aVector.y), z(aVector.z)
	{
	}

	//template<class T>
	//inline Vector3<T>::Vector3(const Vector2<T>& aVector, const T& aZ) : x(aVector.x), y(aVector.y), z(aZ)
	//{
	//}

	template<class T>
	inline Vector3<T>::Vector3(const Vector4<T>& aVector) : x(aVector.x), y(aVector.y), z(aVector.z)
	{
	}

	template<class T>
	inline void Vector3<T>::Set(T aX, T aY, T aZ)
	{
		x = aX;
		y = aY;
		z = aZ;
	}

	template<class T>
	inline Vector3<T>& Vector3<T>::operator=(const Vector3& aVector)
	{
		x = aVector.x;
		y = aVector.y;
		z = aVector.z;
		return *this;
	}

	template<class T>
	inline Vector3<T>& Vector3<T>::operator=(const T aValue)
	{
		x = aValue;
		y = aValue;
		z = aValue;
		return *this;
	}

	template<class T>
	inline bool Vector3<T>::operator==(const Vector3& aVector) const
	{
		return (x == aVector.x) && (y == aVector.y) && (z == aVector.z);
	}

	template<class T>
	inline bool Vector3<T>::operator!=(const Vector3& aVector) const
	{
		return (x != aVector.x) || (y != aVector.y) || (z != aVector.z);
	}

	//template <class T> Vector3<T> operator*(const Vector3<T>& aVector, const T& aScalar)
	//{
	//	Vector3<T> vectorProd;

	//	vectorProd.x = aVector.x * aScalar;
	//	vectorProd.y = aVector.y * aScalar;
	//	vectorProd.z = aVector.z * aScalar;

	//	return vectorProd;
	//}

	template<class T>
	inline T Vector3<T>::Distance(const Vector3& aVector) const
	{
		return sqrt(((x - aVector.x) * (x - aVector.x)) + ((y - aVector.y) * (y - aVector.y)) + ((z - aVector.z) * (z - aVector.z)));
	}

	template<class T>
	inline T Vector3<T>::DistanceSqr(const Vector3& aVector) const
	{
		return (((x - aVector.x) * (x - aVector.x)) + ((y - aVector.y) * (y - aVector.y)) + ((z - aVector.z) * (z - aVector.z)));
	}

	template<class T>
	inline T Vector3<T>::LengthSqr() const
	{
		return (x * x) + (y * y) + (z * z);
	}

	template<class T>
	inline T Vector3<T>::Length() const
	{
		return sqrt((x * x) + (y * y) + (z * z));
	}

	template<class T>
	inline Vector3<T> Vector3<T>::GetNormalized() const
	{
		if (Length() != 0)
		{
			T len = Length();
			return Vector3(x / len, y / len, z / len);
		}
		else
		{
			return Vector3(0, 0, 0);
		}
	}

	template<class T>
	inline void Vector3<T>::Normalize()
	{
		*this = GetNormalized();
	}
	template<class T>
	inline T Vector3<T>::Dot(const Vector3& aVector) const
	{
		return (x * aVector.x) + (y * aVector.y) + (z * aVector.z);
	}
	template<class T>
	inline Vector3<T> Vector3<T>::Cross(const Vector3 & aVector) const
	{
		return Vector3((y * aVector.z) - (z * aVector.y),
			(z * aVector.x) - (x * aVector.z),
			(x * aVector.y) - (y * aVector.x));
	}

	template<class T>
	inline i32 Vector3<T>::Hash() const
	{
		return (i32)((i32)(x * (T)73856093) ^ (i32)(y * (i32)19349663) ^ (i32)(z * (T)83492791));
	}

	template<class T>
	inline T Vector3<T>::Max() const
	{
		return fw::Max(x, fw::Max(y, z));
	}

	using Vec3f = Vector3<f32>;
	using Vector3i = Vector3<i32>;
}

namespace fw = frostwave;

namespace std
{
	template<>
	struct hash<fw::Vector3i>
	{
		typedef fw::Vector3i argument_type;
		typedef std::size_t result_type;

		result_type operator()(argument_type const& s) const noexcept
		{
			return (result_type)s.Hash();
		}
	};
}

template <class T> fw::Vector3<T> operator-(fw::Vector3<T>& aVector) { return fw::Vector3<T>(-aVector.x, -aVector.y, -aVector.z); }
template <class T> fw::Vector3<T> operator+(const fw::Vector3<T>& aVector0, const fw::Vector3<T>& aVector1) { return fw::Vector3<T>((aVector0.x + aVector1.x), (aVector0.y + aVector1.y), (aVector0.z + aVector1.z)); }
template <class T> fw::Vector3<T> operator+(const fw::Vector3<T>& aVector0, const T& aAddition) { return fw::Vector3<T>((aVector0.x + aAddition), (aVector0.y + aAddition), (aVector0.z + aAddition)); }
template <class T> fw::Vector3<T> operator-(const fw::Vector3<T>& aVector0, const T& aSubtraction) { return fw::Vector3<T>((aVector0.x - aSubtraction), (aVector0.y - aSubtraction), (aVector0.z - aSubtraction)); }
template <class T> fw::Vector3<T> operator-(const fw::Vector3<T>& aVector0, const fw::Vector3<T>& aVector1) { return fw::Vector3<T>((aVector0.x - aVector1.x), (aVector0.y - aVector1.y), (aVector0.z - aVector1.z)); }
template <class T> fw::Vector3<T> operator*(const fw::Vector3<T>& aVector, const T& aScalar) { return fw::Vector3<T>((aVector.x * aScalar), (aVector.y * aScalar), (aVector.z * aScalar)); }
template <class T> fw::Vector3<T> operator*(const T& aScalar, const fw::Vector3<T>& aVector) { return fw::Vector3<T>((aScalar * aVector.x), (aScalar * aVector.y), (aScalar * aVector.z)); }
template <class T> fw::Vector3<T> operator/(const fw::Vector3<T>& aVector, const T& aScalar) { return fw::Vector3<T>((aVector.x / aScalar), (aVector.y / aScalar), (aVector.z / aScalar)); }
template <class T> fw::Vector3<T> operator/(const fw::Vector3<T>& aVector0, const fw::Vector3<T>& aVector1){ return fw::Vector3<T>(aVector0.x / aVector1.x, aVector0.y / aVector1.y, aVector0.z / aVector1.z);}
template <class T> void operator+=(fw::Vector3<T>& aVector0, const fw::Vector3<T>& aVector1) { aVector0 = aVector0 + aVector1; }
template <class T> void operator-=(fw::Vector3<T>& aVector0, const fw::Vector3<T>& aVector1) { aVector0 = aVector0 - aVector1; }
template <class T> void operator*=(fw::Vector3<T>& aVector, const T& aScalar) { aVector = aVector * aScalar; }
template <class T> void operator/=(fw::Vector3<T>& aVector, const T& aScalar) { aVector = aVector / aScalar; }

template <class T> fw::Vector3<T> operator*(const fw::Vector3<T>& aVector0, const fw::Vector3<T>& aVector1) { return fw::Vector3<T>((aVector0.x * aVector1.x), (aVector0.y * aVector1.y), (aVector0.z * aVector1.z)); }
template <class T> void operator*=(fw::Vector3<T>& aVector0, const fw::Vector3<T>& aVector1) { aVector0 = aVector0 * aVector1; }

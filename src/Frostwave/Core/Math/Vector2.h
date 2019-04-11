#pragma once
#include <Frostwave/Core/Types.h>

namespace frostwave
{
	template <class T>
	class Vector3;

	template <class T>
	class Vector2
	{
	public:
#pragma warning( push )
#pragma warning( disable : 4201 )
		union
		{
			T myValues[2];
			struct { T x; T y; };
			struct { T r; T g; };
			struct { T u; T v; };
		};
#pragma warning( pop )

		Vector2();
		Vector2(T aX, T aY);
		Vector2<T>(const T& aValue);
		template<typename U>
		Vector2(const Vector3<U>& aVector);
		~Vector2() = default;
		
		template<class R>
		Vector2(const Vector2<R>& aVector);

		inline void Set(T aX, T aY);

		inline Vector2& operator=(const Vector2& aVector);
		inline Vector2& operator=(const T aValue);

		inline bool operator==(const Vector2& aVector) const;
		inline bool operator!=(const Vector2& aVector) const;

		T LengthSqr() const;
		T Length() const;
		T Distance(const Vector2& aVector) const;
		T DistanceSqr(const Vector2& aVector) const;
		T Dot(const Vector2& aVector) const;

		T Max() const { return fw::Max(x, y); }
		T Min() const { return fw::Min(x, y); }

		Vector2 Normal() { return{ y,- x }; }

		Vector2 GetNormalized() const;
		Vector2 Normalize();

		Vector2 Rotate(const f32 aAngle);

		i32 Hash() const;
	};

	template<class T>
	inline Vector2<T>::Vector2() : x(0), y(0)
	{
	}

	template<class T>
	inline Vector2<T>::Vector2(T aX, T aY) : x(aX), y(aY)
	{
	}

	template<class T>
	inline Vector2<T>::Vector2(const T & aValue)
	{
		x = aValue;
		y = aValue;
	}

	template<typename T>
	template<typename U>
	inline Vector2<T>::Vector2(const Vector3<U>& aVector) : x((T)aVector.x), y((T)aVector.y)
	{
	}

	template<class T>
	template<class R>
	inline Vector2<T>::Vector2(const Vector2<R>& aVector) : x((T)aVector.x), y((T)aVector.y)
	{
	}

	template<class T>
	inline void Vector2<T>::Set(T aX, T aY)
	{
		x = aX;
		y = aY;
	}

	template<class T>
	Vector2<T>& Vector2<T>::operator=(const Vector2& aVector)
	{
		x = aVector.x;
		y = aVector.y;
		return *this;
	}

	template<class T>
	inline Vector2<T>& Vector2<T>::operator=(const T aValue)
	{
		x = aValue;
		y = aValue;
		return *this;
	}

	template<class T>
	inline bool Vector2<T>::operator==(const Vector2& aVector) const
	{
		return x == aVector.x && y == aVector.y;
	}

	template<class T>
	inline bool Vector2<T>::operator!=(const Vector2 & aVector) const
	{
		return (x != aVector.x) || (y != aVector.y);
	}

	template<class T>
	inline T Vector2<T>::LengthSqr() const
	{
		return (x * x) + (y * y);
	}

	template<class T>
	inline T Vector2<T>::Length() const
	{
		return sqrt((x * x) + (y * y));
	}

	template<class T>
	inline T Vector2<T>::Distance(const Vector2 & aVector) const
	{
		return sqrt(((x - aVector.x) * (x - aVector.x)) + ((y - aVector.y) * (y - aVector.y)));
	}

	template<class T>
	inline T Vector2<T>::DistanceSqr(const Vector2 & aVector) const
	{
		return (((x - aVector.x) * (x - aVector.x)) + ((y - aVector.y) * (y - aVector.y)));
	}

	template<class T>
	inline T Vector2<T>::Dot(const Vector2 & aVector) const
	{
		return (x * aVector.x) + (y * aVector.y);
	}

	template<class T>
	inline Vector2<T> Vector2<T>::GetNormalized() const
	{
		if (Length() != 0)
		{
			T len = Length();
			return Vector2(x / len, y / len);
		}
		else
		{
			return Vector2(0, 0);
		}
	}

	template<class T>
	inline Vector2<T> Vector2<T>::Normalize()
	{
		*this = GetNormalized();
		return *this;
	}

	template<class T>
	inline Vector2<T> Vector2<T>::Rotate(const f32 aAngle)
	{
		f32 s = std::sin(aAngle);
		f32 c = std::cos(aAngle);

		return {
			(c * x) - (s * y),
			(s * x) + (s * y)
		};
	}

	template<class T>
	inline i32 Vector2<T>::Hash() const
	{
		return (i32)((i32)(x * (T)73856093) ^ (i32)(y * (i32)19349663));
	}

	using Vec2f = Vector2<f32>;
	using Vector2i = Vector2<i32>;
	using Vector2ui = Vector2<u32>;
}
namespace fw = frostwave;

template <class T> fw::Vector2<T> operator-(fw::Vector2<T>& aVector) { return fw::Vector2<T>(-aVector.x, -aVector.y); }
template <class T> fw::Vector2<T> operator+(const fw::Vector2<T>& aVector0, const fw::Vector2<T>& aVector1) { return fw::Vector2<T>((aVector0.x + aVector1.x), (aVector0.y + aVector1.y)); }
template <class T> fw::Vector2<T> operator-(const fw::Vector2<T>& aVector0, const fw::Vector2<T>& aVector1) { return fw::Vector2<T>((aVector0.x - aVector1.x), (aVector0.y - aVector1.y)); }
template <class T> fw::Vector2<T> operator*(const fw::Vector2<T>& aVector, const T& aScalar) { return fw::Vector2<T>((aVector.x * aScalar), (aVector.y * aScalar)); }
template <class T> fw::Vector2<T> operator*(const T& aScalar, const fw::Vector2<T>& aVector) { return fw::Vector2<T>((aScalar * aVector.x), (aScalar * aVector.y)); }
template <class T> fw::Vector2<T> operator/(const fw::Vector2<T>& aVector, const T& aScalar) { return fw::Vector2<T>((aVector.x / aScalar), (aVector.y / aScalar)); }
template <class T> fw::Vector2<T> operator/(const fw::Vector2<T>& aVector, const fw::Vector2<T>& aVector2) { return fw::Vector2<T>((aVector.x / aVector2.x), (aVector.y / aVector2.y)); }
template <class T> void operator+=(fw::Vector2<T>& aVector0, const fw::Vector2<T>& aVector1) { aVector0 = aVector0 + aVector1; }
template <class T> void operator-=(fw::Vector2<T>& aVector0, const fw::Vector2<T>& aVector1) { aVector0 = aVector0 - aVector1; }
template <class T> void operator*=(fw::Vector2<T>& aVector, const T& aScalar) { aVector = aVector * aScalar; }
template <class T> void operator/=(fw::Vector2<T>& aVector, const T& aScalar) { aVector = aVector / aScalar; }

template<class T> fw::Vector2<T> operator/(fw::Vector2<T>& aVec0, fw::Vector2<T>& aVec1) { return{ aVec0.x / aVec1.x, aVec0.y / aVec1.y }; }

template <class T> fw::Vector2<T> operator*(const fw::Vector2<T>& aVector0, const fw::Vector2<T>& aVector1) { return fw::Vector2<T>((aVector0.x * aVector1.x), (aVector0.y * aVector1.y)); }
template <class T> void operator*=(fw::Vector2<T>& aVector0, const fw::Vector2<T>& aVector1) { aVector0 = aVector0 * aVector1; }

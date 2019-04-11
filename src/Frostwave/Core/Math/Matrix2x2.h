#pragma once
#include "Vector2.h"

namespace frostwave
{
	template <class T>
	class Matrix2x2
	{
	public:
		Matrix2x2();
		Matrix2x2(T a11, T a12, T a21, T a22);
		Matrix2x2(const Matrix2x2<T>& aMatrix) = default;
		~Matrix2x2() = default;

		Matrix2x2<T> Transposed() const;
		Matrix2x2<T> Inversed() const;
		T Determinant() const;

		T m11, m12, m21, m22;
	};

	template <class T> inline Matrix2x2<T> operator+(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1);
	template <class T> inline Matrix2x2<T> operator-(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1);
	template <class T> inline Matrix2x2<T> operator*(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1);
	template <class T> inline Matrix2x2<T> operator*(const Matrix2x2<T>& aMatrix0, const T& aScalar);
	template <class T> inline Vector2<T> operator*(const Vector2<T>& aVector, const Matrix2x2<T>& aMatrix);

	template <class T> inline void operator+=(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1) { aMatrix0 = aMatrix0 + aMatrix1; }
	template <class T> inline void operator-=(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1) { aMatrix0 = aMatrix0 - aMatrix1; }
	template <class T> inline void operator*=(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1) { aMatrix0 = aMatrix0 * aMatrix1; }
	template <class T> inline void operator*=(const Matrix2x2<T>& aMatrix0, const T& aScalar) { aMatrix0 = aMatrix0 * aScalar; };
	template <class T> inline void operator*=(const Vector2<T>& aVector, const Matrix2x2<T>& aMatrix) { aVector = aVector * aMatrix; };

	template<class T>
	inline Matrix2x2<T>::Matrix2x2()
	{
		m11 = 1;
		m12 = 0;
		m21 = 0;
		m22 = 1;
	}
	template<class T>
	inline Matrix2x2<T>::Matrix2x2(T a11, T a12, T a21, T a22)
	{
		m11 = a11;
		m12 = a12;
		m21 = a21;
		m22 = a22;
	}
	template<class T>
	inline Matrix2x2<T> Matrix2x2<T>::Transposed() const
	{
		return Matrix2x2<T>(m.m11, m.m21, m.m12, m.m22);
	}
	template<class T>
	inline Matrix2x2<T> Matrix2x2<T>::Inversed() const
	{
		T det = Determinant();
		if (det == 0)
		{
			assert("Matrix cannot be inversed");
			return *this;
		}
		Matrix2x2<T> inverse(m22, -m12, m21, m11);
		T invDet = 1 / det;
		inverse *= invDet;
		return inverse;
	}
	template<class T>
	inline T Matrix2x2<T>::Determinant() const
	{
		return m11 * m22 - m12 * m21;
	}
	template <class T>
	inline Matrix2x2<T> operator+(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1)
	{
		return Matrix2x2<T>(
			aMatrix0.m11 + aMatrix1.m11,
			aMatrix0.m12 + aMatrix1.m12,
			aMatrix0.m21 + aMatrix1.m21,
			aMatrix0.m22 + aMatrix1.m22
			);
	}
	template <class T>
	inline Matrix2x2<T> operator-(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1)
	{
		return Matrix2x2<T>(
			aMatrix0.m11 - aMatrix1.m11,
			aMatrix0.m12 - aMatrix1.m12,
			aMatrix0.m21 - aMatrix1.m21,
			aMatrix0.m22 - aMatrix1.m22
			);
	}
	template <class T>
	inline Matrix2x2<T> operator*(const Matrix2x2<T>& aMatrix0, const Matrix2x2<T>& aMatrix1)
	{
		return Matrix2x2<T>(
			aMatrix0.m11 * aMatrix1.m11 + aMatrix0.m12 * aMatrix1.m21,
			aMatrix0.m11 * aMatrix1.m12 + aMatrix0.m12 * aMatrix1.m22,
			aMatrix0.m21 * aMatrix1.m11 + aMatrix0.m22 * aMatrix1.m21,
			aMatrix0.m21 * aMatrix1.m12 + aMatrix0.m22 * aMatrix1.m22
			);
	}
	template<class T>
	Matrix2x2<T> operator*(const Matrix2x2<T>& aMatrix0, const T & aScalar)
	{
		return Matrix2x2<T>(
			aMatrix0.m11 * aScalar,
			aMatrix0.m12 * aScalar,
			aMatrix0.m21 * aScalar,
			aMatrix0.m22 * aScalar
			);
	}
	template <class T>
	inline Vector2<T> operator*(const Vector2<T>& aVector, const Matrix2x2<T>& aMatrix)
	{
		return Vector2<T> v(
			aVector.x * aMatrix.m11 + aVector.y * aMatrix.m21,
			aVector.x * aMatrix.m12 + aVector.y * aMatrix.m22
		);
	}
}
namespace fw = frostwave;

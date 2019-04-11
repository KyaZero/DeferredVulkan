#pragma once
#include <Frostwave/stdafx.h>
#include <Frostwave/Core/Types.h>
#include <Frostwave/Core/Math/Vector.h>
#include <Frostwave/Core/Math/Matrix4x4.h>
#include <Frostwave/Core/Random.h>
#include <Frostwave/Core/Easings.h>
#include <Frostwave/Core/Containers/GrowingArray.h>

namespace frostwave
{
	template <typename T, typename U>
	inline T Max(const T& x, const U& y)
	{
		return (T)((x < y) ? y : x);
	}

	template <typename T, typename U>
	inline T Min(const T& x, const U& y)
	{
		return (T)(x < y) ? x : y;
	}

	template<typename T, typename U>
	inline T Big(const T& a, const U& b)
	{
		return std::abs(a) > std::abs(b) ? a : b;
	}

	template <typename T, typename U, typename R>
	inline T Clamp(const T& x, const U& min, const R& max)
	{
		return (T)fw::Max(min, fw::Min(x, max));
	}

	template <typename T>
	inline f32 Cerp(T a1, T a2, f32 t)
	{
		T t2 = (1 - cos(t * PI)) / 2;
		return (a1 * (1 - t2) + a2 * t2);
	}

	template <typename T, typename T2>
	T Lerp(const T& aMin, const T& aMax, const T2& aCurrentValue)
	{
		return aMin + (aMax - aMin) * aCurrentValue;
	}

	inline static fw::Color MakeColorFromHex(const u32 aHexColor)
	{
		return fw::Color(
			(aHexColor & 0xFF000000) / static_cast<f32>(0xFF000000),
			(aHexColor & 0x00FF0000) / static_cast<f32>(0x00FF0000),
			(aHexColor & 0x0000FF00) / static_cast<f32>(0x0000FF00),
			(aHexColor & 0x000000FF) / static_cast<f32>(0x000000FF)
		);
	}

	template <typename T>
	inline T ToRadians(T aAngleInDegrees)
	{
		return aAngleInDegrees * fw::PI / 180.f;
	}

	template <typename T>
	inline T ToDegrees(T aAngleInRadians)
	{
		return aAngleInRadians * 180.f / fw::PI;
	}

	template <typename T>
	inline string ToString(const T aValue, const i32 aPrecision = 6)
	{
		std::ostringstream out;
		out << std::setprecision(aPrecision) << aValue;
		return out.str();
	}

	inline std::wstring StringToWString(const string& aString)
	{
		////setup converter
		//typedef std::codecvt_utf8<wchar_t> convert_type;
		//std::wstring_convert<convert_type, wchar_t> converter;

		////use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
		//return converter.from_bytes(aString);

		return std::wstring(aString.begin(), aString.end());
	}

	inline string WStringToString(const std::wstring& aString)
	{
		string str(aString.length(), '\0');
		i32 i;
		for (i32 j = 0; j < aString.length(); j++)
		{
			auto& wc = aString[j];
			wctomb_s(&i, &str[j], sizeof(wc), wc);
		}
		return str;
	}

	inline string GetDateTime(const string& aFormat)
	{
		time_t rawtime;
		struct tm timeinfo;
		char buffer[80];

		time(&rawtime);
		localtime_s(&timeinfo, &rawtime);

		strftime(buffer, sizeof(buffer), aFormat.c_str(), &timeinfo);
		string str(buffer);

		return str;
	}
}
namespace fw = frostwave;

#pragma once
#include <Frostwave/stdafx.h>
#include <Frostwave/Core/CommonMathDefinitions.h>

namespace frostwave
{
	namespace Easing
	{
		inline double EaseInSine(double t)
		{
			return sin(1.5707963 * t);
		}

		inline double EaseOutSine(double t)
		{
			return 1 + sin(1.5707963 * (--t));
		}

		inline double EaseInOutSine(double t)
		{
			return 0.5 * (1 + sin(3.1415926 * (t - 0.5)));
		}

		inline double EaseInQuad(double t)
		{
			return t * t;
		}

		inline double EaseOutQuad(double t)
		{
			return t * (2 - t);
		}

		inline double EaseInOutQuad(double t)
		{
			return t < 0.5 ? 2 * t * t : t * (4 - 2 * t) - 1;
		}

		inline double EaseInCubic(double t)
		{
			return t * t * t;
		}

		inline double EaseOutCubic(double t)
		{
			return 1 + (--t) * t * t;
		}

		inline double EaseInOutCubic(double t)
		{
			return t < 0.5 ? 4 * t * t * t : 1 + (--t) * (2 * (--t)) * (2 * t);
		}

		inline double EaseInQuart(double t)
		{
			t *= t;
			return t * t;
		}

		inline double EaseOutQuart(double t)
		{
			t = (--t) * t;
			return 1 - t * t;
		}

		inline double EaseInOutQuart(double t)
		{
			if (t < 0.5) {
				t *= t;
				return 8 * t * t;
			}
			else {
				t = (--t) * t;
				return 1 - 8 * t * t;
			}
		}

		inline double EaseInQuint(double t)
		{
			double t2 = t * t;
			return t * t2 * t2;
		}

		inline double EaseOutQuint(double t)
		{
			double t2 = (--t) * t;
			return 1 + t * t2 * t2;
		}

		inline double EaseInOutQuint(double t)
		{
			double t2;
			if (t < 0.5) {
				t2 = t * t;
				return 16 * t * t2 * t2;
			}
			else {
				t2 = (--t) * t;
				return 1 + 16 * t * t2 * t2;
			}
		}

		inline double EaseInExpo(double t)
		{
			return (pow(2, 8 * t) - 1) / 255;
		}

		inline double EaseOutExpo(double t)
		{
			return 1 - pow(2, -8 * t);
		}

		inline double EaseInOutExpo(double t)
		{
			if (t < 0.5) {
				return (pow(2, 16 * t) - 1) / 510;
			}
			else {
				return 1 - 0.5 * pow(2, -16 * (t - 0.5));
			}
		}

		inline double EaseInCirc(double t)
		{
			return 1 - sqrt(1 - t);
		}

		inline double EaseOutCirc(double t)
		{
			return sqrt(t);
		}

		inline double EaseInOutCirc(double t)
		{
			if (t < 0.5) {
				return (1 - sqrt(1 - 2 * t)) * 0.5;
			}
			else {
				return (1 + sqrt(2 * t - 1)) * 0.5;
			}
		}

		inline double EaseInBack(double t)
		{
			return t * t * (2.70158 * t - 1.70158);
		}

		inline double EaseOutBack(double t)
		{
			return 1 + (--t) * t * (2.70158 * t + 1.70158);
		}

		inline double EaseInOutBack(double t)
		{
			if (t < 0.5) {
				return t * t * (7 * t - 2.5) * 2;
			}
			else {
				return 1 + (--t) * t * 2 * (7 * t + 2.5);
			}
		}

		inline double EaseInElastic(double t)
		{
			double t2 = t * t;
			return t2 * t2 * sin(t * PI * 4.5);
		}

		inline double EaseOutElastic(double t)
		{
			double t2 = (t - 1) * (t - 1);
			return 1 - t2 * t2 * cos(t * PI * 4.5);
		}

		inline double EaseInOutElastic(double t)
		{
			double t2;
			if (t < 0.45) {
				t2 = t * t;
				return 8 * t2 * t2 * sin(t * PI * 9);
			}
			else if (t < 0.55) {
				return 0.5 + 0.75 * sin(t * PI * 4);
			}
			else {
				t2 = (t - 1) * (t - 1);
				return 1 - 8 * t2 * t2 * sin(t * PI * 9);
			}
		}

		inline double EaseInBounce(double t)
		{
			return pow(2, 6 * (t - 1)) * abs(sin(t * PI * 3.5));
		}

		inline double EaseOutBounce(double t)
		{
			return 1 - pow(2, -6 * t) * abs(cos(t * PI * 3.5));
		}

		inline double EaseInOutBounce(double t)
		{
			if (t < 0.5) {
				return 8 * pow(2, 8 * (t - 1)) * abs(sin(t * PI * 7));
			}
			else {
				return 1 - 8 * pow(2, -8 * t) * abs(sin(t * PI * 7));
			}
		}
	}
}

namespace fw = frostwave;

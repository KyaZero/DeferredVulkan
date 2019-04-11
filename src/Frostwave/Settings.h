#pragma once

#include <Frostwave/Core/Types.h>

namespace frostwave
{
	struct WindowSettings
	{
		enum 
		{ 
			Windowed				= (1 << 1), 
			Fullscreen				= (1 << 2),
			Borderless				= (1 << 3)
		};

		string title = "Default Title";
		i32 mode = Windowed;
		u32 width = 800;
		u32 height = 600;
		bool focusFreeze = true;
	};

	struct GraphicsSettings
	{
		enum
		{
			Off		= (1 << 1),
			Verbose = (1 << 2),
			Info	= (1 << 3),
			Warning = (1 << 4),
			Error	= (1 << 5)
		};

		enum
		{
			None		= (1 << 1),
			General		= (1 << 2),
			Validation	= (1 << 3),
			Performance = (1 << 4)
		};

#ifdef _RETAIL
		i32 validation = Off;
		i32 validationType = None;
#else
		i32 validationType = General | Validation | Performance;
		i32 validation = Verbose | Info | Warning | Error;
#endif
		bool vsync = false;
	};

	struct Settings
	{
		WindowSettings window;
		GraphicsSettings graphics;
	};
}
namespace fw = frostwave;
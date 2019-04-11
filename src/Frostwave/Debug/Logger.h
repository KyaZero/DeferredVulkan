#pragma once
#include <fstream>
#include <array>
//#include <frostwave/Core/Containers/StaticArray.h>
#include <string>
#include <chrono>
//#include <frostwave/Core/StackWalker/StackWalker.h>
#include <mutex>
//#include <Engine/Debug/imgui/imgui.h>
//#include <Engine/Core/Math/Vector4.h>

#define VERBOSE_LOG(...) frostwave::Logger::Get()->Log(frostwave::Logger::Level::Verbose, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define INFO_LOG(...) frostwave::Logger::Get()->Log(frostwave::Logger::Level::Info, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define WARNING_LOG(...) frostwave::Logger::Get()->Log(frostwave::Logger::Level::Warning, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define ERROR_LOG(...) frostwave::Logger::Get()->Log(frostwave::Logger::Level::Error, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);
#define FATAL_LOG(...) frostwave::Logger::Get()->Log(frostwave::Logger::Level::Fatal, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);

namespace frostwave
{
	class Logger
	{
	public:
		enum class Level : char
		{
			All,
			Verbose,
			Info,
			Warning,
			Error,
			Fatal,
			Count
		};

		static constexpr i32 WhiteTextColorIndex	= 1;
		static constexpr i32 InfoColorIndex			= 2;
		static constexpr i32 WarningColorIndex		= 3;
		static constexpr i32 ErrorColorIndex		= 4;
		static constexpr i32 FatalColorIndex		= 5;
		static constexpr i32 DarkTextColorIndex		= 7;
		static constexpr i32 ConsoleAlpha			= 245;

	public:
		static void Create(const string& aFilename = "Frostwave.log");
		static void Destroy();
		static void SetLogLevel(Level aLogLevel);

		static Logger* Get();
		void Log(Level aLogLevel, const char* aFile, long aLine, const char* aFunctionName, const char* aFormatString, ...);
	private:
		Logger();
		~Logger();
		void InitConsole();
		static Logger* ourInstance;
		static string LogPath;
		std::array<string, (char)Level::Count + 1> myLevelStrings;
		Level myLogLevel;
		std::fstream myFile;
		std::mutex myMutex;

		static constexpr u32 BufferSize = 2048;
	};
}
namespace fw = frostwave;
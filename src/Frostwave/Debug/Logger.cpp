#include "stdafx.h"
#include "Logger.h"
#include <filesystem>
#include <Frostwave/Core/FileManip.h>
#include <Frostwave/Core/Common.h>
#include <iostream>
#include <cstdarg>
#include <Windows.h>

//#include <Engine/Threading/ThreadScheduler.h>

namespace fs = std::filesystem;

string frostwave::Logger::LogPath = "logs/";

frostwave::Logger* frostwave::Logger::ourInstance = nullptr;

void frostwave::Logger::Create(const string& aFilename)
{
	aFilename;
	ourInstance = new Logger();
#ifndef _RETAIL

	if (!fs::is_directory(LogPath) || !fs::exists(LogPath))
	{
		fs::create_directories(LogPath);
	}

	File::ForEachFileInDir(LogPath, [&](string s)
	{
		if (remove(s.c_str()) != 0)
		{
			INFO_LOG("Error deleting %s", s.c_str());
		}
	});

	string filename = LogPath + aFilename;

	ourInstance->myFile.open(filename, std::fstream::out | std::fstream::app);

	//Setup enum names
	ourInstance->myLevelStrings[(char)Logger::Level::All] = "";
	ourInstance->myLevelStrings[(char)Logger::Level::Verbose] = "Verbose";
	ourInstance->myLevelStrings[(char)Logger::Level::Info] = "Info";
	ourInstance->myLevelStrings[(char)Logger::Level::Warning] = "Warning";
	ourInstance->myLevelStrings[(char)Logger::Level::Error] = "Error";
	ourInstance->myLevelStrings[(char)Logger::Level::Fatal] = "FATAL";
	ourInstance->myLevelStrings[(char)Logger::Level::Count] = "";

	ourInstance->myLogLevel = Logger::Level::All;

	ourInstance->InitConsole();
#endif
}

void frostwave::Logger::Destroy()
{
#ifndef _RETAIL
	ourInstance->myFile.close();
	delete ourInstance;
	ourInstance = nullptr;
#endif
}

frostwave::Logger* frostwave::Logger::Get()
{
	return ourInstance;
}

void frostwave::Logger::SetLogLevel(Level aLogLevel)
{
	Get()->myLogLevel = aLogLevel;
}

void frostwave::Logger::Log(Level aLogLevel, const char* aFile, long aLine, const char* aFunctionName, const char* aFormatString, ...)
{
	if ((char)aLogLevel < (char)myLogLevel) return;
	if (aLogLevel == Logger::Level::All || aLogLevel == Logger::Level::Count) return;
	aLogLevel; aFile; aLine; aFunctionName; aFormatString;
#ifndef _RETAIL
	static const constexpr i32 MaxBufferSize = 4096;

	char buffer[MaxBufferSize];
	va_list args;
	va_start(args, aFormatString);
	vsprintf_s(buffer, aFormatString, args);
	va_end(args);

	//ThreadScheduler::Get()->QueueIOWork(new IOWorkItem("Logging", [=](IOWorkItem* item)
	//{
	//	item;

		string shortFile = fw::File::GetFileName(aFile, { ".h", ".cpp" }, false);
		string functionName(aFunctionName);

		if (functionName.find("lambda") != string::npos)
		{
			functionName = functionName.substr(0, functionName.find("lambda") + 6) + ">";
		}

		string shortFuncName;

		size_t shorteningPos = functionName.find_last_of(":");
		if (shorteningPos != string::npos)
		{
			shortFuncName = functionName.substr(shorteningPos + 1);
		}
		else
		{
			shortFuncName = functionName;
		}

		char buffer2[MaxBufferSize];
		sprintf_s(buffer2, "[%s][%s] %s:%li:%s: %s\n", fw::GetDateTime("%H:%M:%S").c_str(), myLevelStrings[(char)aLogLevel].c_str(), shortFile.c_str(), aLine, functionName.c_str(), buffer);
		std::unique_lock<std::mutex> lk(myMutex);
		myFile << buffer2;
		myFile.flush();

		HWND hwnd = GetConsoleWindow();
		if (aLogLevel == Level::Fatal || aLogLevel == Level::Error)
		{
			//Flash to get attention
			FlashWindow(hwnd, false);
		}

		//rewriting the same code but with color :(
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, DarkTextColorIndex);
		printf("[");
		SetConsoleTextAttribute(hConsole, (char)aLogLevel);
		printf("%s", myLevelStrings[(char)aLogLevel].c_str());
		SetConsoleTextAttribute(hConsole, DarkTextColorIndex);
		printf("] %s:%li:%s: ", shortFile.c_str(), aLine, shortFuncName.c_str());
		SetConsoleTextAttribute(hConsole, WhiteTextColorIndex);
		printf("%s\n", buffer);

		lk.unlock();

#ifdef _WIN32
		if (aLogLevel == frostwave::Logger::Level::Fatal)
		{
			MessageBoxA(NULL, buffer, "Fatal Error", MB_OK);
			DebugBreak();
			std::exit(0);
		};
#else
		if (aLogLevel == frostwave::Logger::Level::Fatal) abort();
#endif
	//}, [](IOWorkItem* item) { delete item; }));
#endif
}

frostwave::Logger::Logger()
{
}

frostwave::Logger::~Logger()
{
}

void frostwave::Logger::InitConsole()
{
#ifndef _RETAIL
#pragma warning( push )
#pragma warning( disable : 4996 )
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
#pragma warning( pop )

	HWND hwnd = GetConsoleWindow();

	SetConsoleTitle(L"Frostwave Console");
	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, 0, ConsoleAlpha, LWA_ALPHA);

	auto console = GetStdHandle(STD_OUTPUT_HANDLE);
	_CONSOLE_SCREEN_BUFFER_INFOEX csbi;
	csbi.cbSize = sizeof(_CONSOLE_SCREEN_BUFFER_INFOEX);
	GetConsoleScreenBufferInfoEx(console, &csbi);

	csbi.ColorTable[0] =				    (25 << 0) | (25 << 8) | (30 << 16);
	csbi.ColorTable[WhiteTextColorIndex] =	(219 << 0) | (221 << 8) | (231 << 16);
	csbi.ColorTable[DarkTextColorIndex] =   (150 << 0) | (150 << 8) | (150 << 16);
	csbi.ColorTable[InfoColorIndex] =		(0 << 0) | (150 << 8) | (75 << 16);
	csbi.ColorTable[WarningColorIndex] =	(200 << 0) | (200 << 8) | (50 << 16);
	csbi.ColorTable[ErrorColorIndex] =		(255 << 0) | (50 << 8) | (50 << 16);
	csbi.cbSize = sizeof(csbi);
	SetConsoleScreenBufferInfoEx(console, &csbi);

	MoveWindow(hwnd, 0, 0, 1250, 600, true);
#endif
}
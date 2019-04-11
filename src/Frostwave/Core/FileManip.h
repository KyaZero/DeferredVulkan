#pragma once
#include <Frostwave/stdafx.h>
#include <Frostwave/Core/Types.h>

namespace fs = std::filesystem;

namespace frostwave
{
	namespace File
	{
		inline i64 GetFileTime(const string& aFilePath)
		{
			try
			{
				auto file = fs::current_path() / aFilePath;
				bool exists = fs::exists(file);
				if (!exists) return 0;
				auto lastTime = fs::last_write_time(file);
				auto sinceEpoch = lastTime.time_since_epoch();
				auto count = sinceEpoch.count();
				return static_cast<i64>(count);
			}
			catch (std::exception e)
			{
				e;
			}
			return 0;
		}

		inline void ForEachFileInDir(const string& aDirectory, std::function<void(string)> aCallback)
		{
			for (auto& p : fs::directory_iterator(aDirectory))
			{
				string path = p.path().string();
				if (path.find(".") != string::npos)
				{
					aCallback(path);
				}
				else
				{
					ForEachFileInDir(path, aCallback);
				}
			}
		}

		inline void ForEachFileInDir(const wstring& aDirectory, std::function<void(wstring)> aCallback)
		{
			for (auto& p : fs::directory_iterator(aDirectory))
			{
				wstring path = p.path().wstring();
				if (path.find(L".") != wstring::npos)
				{
					aCallback(path);
				}
				else
				{
					ForEachFileInDir(path, aCallback);
				}
			}
		}

		inline string GetFileName(const string& aFilePath, const string& aFileExtension = "", bool aShouldTrim = false)
		{
			std::size_t found = aFilePath.find_last_of("/\\");

			string fileName = aFilePath.substr(found + 1);

			if (aFileExtension != "")
			{
				if (fileName.find(aFileExtension) != string::npos)
				{
					fileName = fileName.substr(0, fileName.length() - (aShouldTrim ? aFileExtension.length() : 0));
				}
			}

			return fileName;
		}

		inline string GetFileName(const string& aFilePath, const std::initializer_list<const char*> aInitList, bool aShouldTrim = false)
		{
			string finalstr = aFilePath;
			for (auto& str : aInitList)
			{
				finalstr = GetFileName(aFilePath, str, aShouldTrim).size() < finalstr.size() ? GetFileName(aFilePath, str, aShouldTrim) : finalstr;
			}
			return finalstr;
		}

		inline wstring GetFileName(const wstring& aFilePath, const wstring& aFileExtension = L"", bool aShouldTrim = false)
		{
			std::size_t found = aFilePath.find_last_of(L"/\\");

			wstring fileName = aFilePath.substr(found + 1);

			if (aFileExtension != L"")
			{
				if (fileName.find(aFileExtension) != wstring::npos)
				{
					fileName = fileName.substr(0, fileName.length() - (aShouldTrim ? aFileExtension.length() : 0));
				}
			}

			return fileName;
		}

		inline wstring GetFileName(const wstring& aFilePath, const std::initializer_list<const wchar_t*> aInitList, bool aShouldTrim = false)
		{
			wstring finalstr = aFilePath;
			for (auto& str : aInitList)
			{
				finalstr = GetFileName(aFilePath, str, aShouldTrim).size() < finalstr.size() ? GetFileName(aFilePath, str, aShouldTrim) : finalstr;
			}
			return finalstr;
		}
	}
}
namespace fw = frostwave;

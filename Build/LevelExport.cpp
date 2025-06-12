#include "LevelExport.h"
#include <fstream>
#include <codecvt>
#include <string>
#include <locale>

// create material "MaxMaterial" has priority is Max;

std::wstring ToWStringEx(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.to_bytes(wstr);
}



GS::ErrCode ExportLevelCsv(void)
{
	API_StoryInfo storyInfo;
	ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
	std::string path = "C:/Tobic/Level.csv";
	std::fstream fout;
	fout.open(path, std::ofstream::out | std::ofstream::trunc);
	API_StoryType* storyInterator = &(*storyInfo.data)[0];
	for (int i = storyInfo.firstStory; i <= storyInfo.lastStory; i++)
	{
		std::wstring wname = ToWStringEx(storyInterator->uName);
		fout << ws2s(wname) << ",";
		fout << storyInterator->index << ",";
		fout << storyInterator->level << "\n";
		++storyInterator;
	}
	fout.close();
	return NoError;
}

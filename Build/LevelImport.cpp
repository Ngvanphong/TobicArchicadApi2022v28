#include "LevelImport.h"
#include <fstream>
#include <sstream>

using namespace std;

std::wstring ToWStringImportLevel(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

void SortEleIndex(GS::Array<EleIndex>& arraySort)
{
	auto count = arraySort.GetSize();
	for (int i = 0; i < count; i++)
	{
		auto sItem = arraySort[i];
		for (int j = i + 1; j < count; j++)
		{
			auto sNextItem = arraySort[j];
			if (sItem.Elevation > sNextItem.Elevation)
			{
				auto tem = arraySort[j];
				arraySort[j] = arraySort[i];
				arraySort[i] = tem;
			}
		}
	}
}

void SortDecesdingEleIndex(GS::Array<EleIndex>& arraySort)
{
	auto count = arraySort.GetSize();
	for (int i = 0; i < count; i++)
	{
		auto sItem = arraySort[i];
		for (int j = i + 1; j < count; j++)
		{
			auto sNextItem = arraySort[j];
			if (sItem.Elevation < sNextItem.Elevation)
			{
				auto tem = arraySort[j];
				arraySort[j] = arraySort[i];
				arraySort[i] = tem;
			}
		}
	}
}

GS::ErrCode ImportLevelFile(void)
{
	std::string path = "C:/Tobic/Level.csv";
	std::fstream fin;
	fin.open(path, ios::in);
	string line, word, temp;
	bool isFirst = true;
	GS::Array<EleIndex> negativeArray = GS::Array<EleIndex>();
	GS::Array<EleIndex> positiveArray = GS::Array<EleIndex>();
	while (fin >> temp)
	{
		std::stringstream s(temp);
		int i = 0;
		EleIndex eleIndex = EleIndex();
		while (std::getline(s, word, ','))
		{
			if (i == 0)
			{
				GS::UniString	uName = word.c_str();
				eleIndex.Name = uName;
			}
			else if (i == 1) eleIndex.Index = std::stod(word);
			else if (i == 2) eleIndex.Elevation = std::stod(word);
			i++;
		}
		if (eleIndex.Index >= 0) positiveArray.Push(eleIndex);
		else negativeArray.Push(eleIndex);
	}
	fin.close();

	SortEleIndex(positiveArray);
	SortDecesdingEleIndex(negativeArray);
	double prev = 0;
	for (auto item : positiveArray)
	{
		API_StoryCmdType* storyType = new API_StoryCmdType();
		if (item.Index == 0) {
			storyType->action = APIStory_Rename;
		}
		else {
			storyType->index = prev;
			storyType->action = APIStory_InsAbove;
			storyType->height = 100;
			prev++;
		}
		GS::ucscpy(storyType->uName, item.Name.ToUStr());
		GS::ErrCode err= ACAPI_ProjectSetting_ChangeStorySettings(storyType);
	}
	double prevNegative = 0;
	for (auto item : negativeArray)
	{
		API_StoryCmdType* storyType = new API_StoryCmdType();
		storyType->index = prevNegative;
		storyType->action = APIStory_InsBelow;
		storyType->height = 100;
		prevNegative--;
		GS::ucscpy(storyType->uName, item.Name.ToUStr());
		GS::ErrCode err = ACAPI_ProjectSetting_ChangeStorySettings(storyType);
	}

	for (auto item : positiveArray)
	{
		API_StoryCmdType* storyType = new API_StoryCmdType();
		storyType->action = APIStory_SetElevation;
		storyType->index = item.Index;
		storyType->elevation = item.Elevation;
		GS::ErrCode err = ACAPI_ProjectSetting_ChangeStorySettings(storyType);
	}

	for (auto item : negativeArray)
	{
		API_StoryCmdType* storyType = new API_StoryCmdType();
		storyType->action = APIStory_SetElevation;
		storyType->index = item.Index;
		storyType->elevation = item.Elevation;
		GS::ErrCode err = ACAPI_ProjectSetting_ChangeStorySettings(storyType);
	}

	return NoError;

}

#include "ProfileRecSteelAtribute.h"
#include	"VectorImageIterator.hpp"
#include	"ProfileVectorImageOperations.hpp"
#include	"OverriddenAttributes.hpp"
#include	"ProfileAdditionalInfo.hpp"
#include "ReadExcel.h"
#include <locale>
#include <codecvt>
#include <string>
#include "ModifyHotSpotRec.h"


GS::ErrCode CreateProfileRecSteel()
{
	auto data = GetDataFromExcelForRecSteelColumn();
	GSErrCode              err = NoError;
	for (DataProfileRecSteel item : data) {
		API_Attribute attrib;
		API_AttributeDefExt defs;
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
		attrib.header.typeID = API_ProfileID;
		std::wstring nameProfile = L"_toda_SíåÅ†-ëœâŒîÌï¢";
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
		CHCopyC(convert.to_bytes(nameProfile).c_str(), attrib.header.name);
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError)
			err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);
		if (err == NoError) {
			ProfileVectorImage* profileDescription = defs.profile_vectorImageItems;
			ConstProfileVectorImageIterator profileDescriptionIt(*profileDescription);
			Int32 indexHatchBreak = 1;
			GS::Array<Sy_HatchType> arrayHatchType;
			GS::Array<HatchObject> arrayHatchObject;
			
			double thickOrigin = 0.022;
			double widthOrigin = 0.6;
			double heightOrigin = 0.6;
			while (indexHatchBreak<=4)
			{
				switch (profileDescriptionIt->item_Typ)
				{
					case SyHatch:
					{
						HatchObject syHatch = profileDescriptionIt;
						Sy_HatchType hatchType;
						profileDescription->AddHatch(hatchType, syHatch);
						UInt32 index = 0;
						for (Coord coord : syHatch.GetCoords()) 
						{
							int x = std::round(coord.x * 10000);
							int y = std::round(coord.y * 10000);

							if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000)) {
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = item.Height / 2;
							}
							else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000)) {
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = item.Height / 2;
							}
							else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.Height / 2;
							}
							else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.Height / 2;
							}
							else if (x == std::round(-(widthOrigin / 2 - thickOrigin) * 10000) && y == std::round((heightOrigin / 2 - thickOrigin) * 10000)) {
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -(item.Width / 2 - item.Thickness);
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = (item.Height / 2 - item.Thickness);
							}
							else if (x == std::round((widthOrigin / 2 - thickOrigin) * 10000) && y == std::round((heightOrigin / 2 - thickOrigin) * 10000)) {
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = (item.Width / 2 - item.Thickness);
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = (item.Height / 2 - item.Thickness);
							}
							else if (x == std::round((widthOrigin / 2 - thickOrigin) * 10000) && y == std::round(-(heightOrigin / 2 - thickOrigin) * 10000)) {
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = (item.Width / 2 - item.Thickness);
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height / 2 - item.Thickness);
							}
							else if (x == std::round(-(widthOrigin / 2 - thickOrigin) * 10000) && y == std::round(-(heightOrigin / 2 - thickOrigin) * 10000)) {
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -(item.Width / 2 - item.Thickness);
								static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height / 2 - item.Thickness);
							}

							index++;
						}
						arrayHatchType.Push(hatchType);
						arrayHatchObject.Push(syHatch);
						break;
					}
				}
				indexHatchBreak++;
				++profileDescriptionIt;
			}
			err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);
			for (Int32 i = 0; i < arrayHatchType.GetSize(); i++) {
				(*defs.profile_vectorImageItems).GetHatchObject(arrayHatchType[i]) = arrayHatchObject[i];
			}
		}
		strcpy(attrib.header.name, item.Name.c_str());
		err = ACAPI_Attribute_CreateExt(&attrib, &defs);
		ACAPI_DisposeAttrDefsHdlsExt(&defs);
		if (err == NoError) {
			//Mofify HotSpot
			ModifyHotSpotRecSteel(item);
		}

	}

	return NoError;
}

#include "CreateProfilesH.h"
#include "ReadExcel.h"
#include <locale>
#include <codecvt>
#include <string>
#include	"VectorImageIterator.hpp"
#include	"ProfileVectorImageOperations.hpp"
#include	"OverriddenAttributes.hpp"
#include	"ProfileAdditionalInfo.hpp"
#include "ModifyHotSpotH.h"

// bat dim lai kich thuoc cua chieu rong, cao tat ca
// dim lai lop chong chay cua cot thep
// dim lai kich thuoc chieu rong cao, cua steel rectangle

GS::ErrCode CreateProfilesH()
{
	GS::ErrCode err = NoError;
	auto data = GetDataFromExcelForH();
	for (DataProfileH item : data) {
		if (item.IsColumn) {
			API_Attribute          attrib;
			API_AttributeDefExt   defs;
			GSErrCode              err;
			BNZeroMemory(&attrib, sizeof(API_Attribute));
			BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
			attrib.header.typeID = API_ProfileID;
			std::wstring nameProfile = L"_toda_S’ŒH-‘Ï‰Î”í•¢";
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
			CHCopyC(convert.to_bytes(nameProfile).c_str(), attrib.header.name);
			err = ACAPI_Attribute_Get(&attrib);
			if (err == NoError)
				err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);

			if (err == NoError) {
				ProfileVectorImage* profileDescription = defs.profile_vectorImageItems;
				ConstProfileVectorImageIterator profileDescriptionIt(*profileDescription);
				Int32 indexHatchBreak = 1;
				GS::Array<Sy_HatchType> arratHatchType;
				GS::Array<HatchObject> arratHatchObject;

				double twOrigin = 0.007;
				double tfOrigin = 0.011;
				double widthOrigin = 0.199;
				double heightOrigin = 0.396;
				while (indexHatchBreak <= 4)
				{
					switch (profileDescriptionIt->item_Typ)
					{
						case SyHatch:
						{
							HatchObject syHatch = profileDescriptionIt;
							ProfileItem profileItemInfo = syHatch.GetProfileItemNonConst(); 
							Sy_HatchType hatchType;
							profileDescription->AddHatch(hatchType, syHatch);
							UInt32 index = 0;
							for (Coord coord : syHatch.GetCoords()) {
								int x = std::round(coord.x * 10000);
								int y = std::round(coord.y * 10000);

								if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000))
								{
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = item.Height / 2;
								}
								else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000))
								{
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = item.Height / 2;
								}
								else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000))
								{
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = item.Height / 2 - item.tf;
								}
								else if (x == std::round(twOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.tw / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = item.Height / 2 - item.tf;
								}
								else if (x == std::round(twOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.tw / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height / 2 - item.tf);
								}
								else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height / 2 - item.tf);
								}
								else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.Height / 2;
								}
								else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.Height / 2;
								}
								else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height / 2 - item.tf);
								}
								else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.tw / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height / 2 - item.tf);
								}
								else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.tw / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = item.Height / 2 - item.tf;
								}
								else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = item.Height / 2 - item.tf;
								}
								else if (x == 0 && y == std::round(heightOrigin / 2 * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = 0;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = item.Height / 2;
								}

								index++;
							}
							arratHatchType.Push(hatchType);
							arratHatchObject.Push(syHatch);
							break;
						}
					}
					indexHatchBreak++;
					++profileDescriptionIt;
				}

				err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);

				for (Int32 i = 0; i < arratHatchType.GetSize(); i++) {
					(*defs.profile_vectorImageItems).GetHatchObject(arratHatchType[i]) = arratHatchObject[i];
				}
			}

			strcpy(attrib.header.name, item.Name.c_str());
			err = ACAPI_Attribute_CreateExt(&attrib, &defs);

			ACAPI_DisposeAttrDefsHdlsExt(&defs);

			if (err == NoError) {
				err = ModifyHotSpotH(item);
			}
		}
		else {
			API_Attribute          attrib;
			API_AttributeDefExt   defs;
			GSErrCode              err;
			BNZeroMemory(&attrib, sizeof(API_Attribute));
			BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
			attrib.header.typeID = API_ProfileID;
			std::wstring nameProfile = L"_toda_S—À-‘Ï‰Î”í•¢";
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
			CHCopyC(convert.to_bytes(nameProfile).c_str(), attrib.header.name);
			err = ACAPI_Attribute_Get(&attrib);
			if (err == NoError)
				err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);

			if (err == NoError) {
				ProfileVectorImage* profileDescription = defs.profile_vectorImageItems;
				ConstProfileVectorImageIterator profileDescriptionIt(*profileDescription);
				Int32 indexHatchBreak = 1;
				GS::Array<Sy_HatchType> arratHatchType;
				GS::Array<HatchObject> arratHatchObject;

				double twOrigin = 0.007;
				double tfOrigin = 0.011;
				double widthOrigin = 0.199;
				double heightOrigin = 0.396;
				while (indexHatchBreak <= 4)
				{
					switch (profileDescriptionIt->item_Typ)
					{
						case SyHatch:
						{
							HatchObject syHatch = profileDescriptionIt;
							ProfileItem profileItemInfo = syHatch.GetProfileItemNonConst();
							Sy_HatchType hatchType;
							profileDescription->AddHatch(hatchType, syHatch);
							UInt32 index = 0;
							for (Coord coord : syHatch.GetCoords()) {
								int x = std::round(coord.x * 10000);
								int y = std::round(coord.y * 10000);

								if (x == std::round(-widthOrigin / 2 * 10000) && y == 0)
								{
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = 0;
								}
								else if (x == std::round(widthOrigin / 2 * 10000) && y == 0)
								{
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = 0;
								}
								else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000))
								{
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.tf;
								}
								else if (x == std::round(twOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.tw / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.tf;
								}
								else if (x == std::round(twOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.tw / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height - item.tf);
								}
								else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height - item.tf);
								}
								else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-heightOrigin * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.Height;
								}
								else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-heightOrigin * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.Height;
								}
								else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height - item.tf);
								}
								else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.tw / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -(item.Height - item.tf);
								}
								else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.tw / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.tf;
								}
								else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = -item.Width / 2;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.tf;
								}
								else if (x == 0 && y == std::round(-heightOrigin / 2 * 10000)) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = 0;
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = -item.Height / 2;
								}

								index++;
							}
							arratHatchType.Push(hatchType);
							arratHatchObject.Push(syHatch);
							break;
						}
						
					}
					
					indexHatchBreak++;
					++profileDescriptionIt;
				}

				err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);

				for (Int32 i = 0; i < arratHatchType.GetSize(); i++) {
					(*defs.profile_vectorImageItems).GetHatchObject(arratHatchType[i]) = arratHatchObject[i];
				}
			}

			strcpy(attrib.header.name, item.Name.c_str());
			err = ACAPI_Attribute_CreateExt(&attrib, &defs);

			ACAPI_DisposeAttrDefsHdlsExt(&defs);
			if (err == NoError) {
				err = ModifyHotSpotH(item);
			}

		}
	}



	return NoError;
}
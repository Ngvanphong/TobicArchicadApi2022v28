#include "CreateProfiles.h"
#include	"VectorImageIterator.hpp"
#include	"ProfileVectorImageOperations.hpp"
#include	"OverriddenAttributes.hpp"
#include	"ProfileAdditionalInfo.hpp"
#include "ReadExcel.h"
#include <locale>
#include <codecvt>
#include <string>
#include "ModifyHotSpot.h"


// bat dim lai kich thuoc cua chieu rong, cao tat ca
// dim lai lop chong chay cua cot thep


GS::ErrCode CreateProfiles()
{
	auto data = GetDataFromExcel();
	GSErrCode              err= NoError;
	for (DataProfile item : data) {
		if (item.IsColumn) {
			API_Attribute          attrib;
			API_AttributeDefExt   defs;
			BNZeroMemory(&attrib, sizeof(API_Attribute));
			BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
			attrib.header.typeID = API_ProfileID;
			std::wstring nameProfile = L"_toda_RCíå-ëùÇµë≈Çø";
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

				double deltaX = item.Width - 0.5;
				double deltaY = item.Height - 0.5;

				while (indexHatchBreak <= 2)
				{
					switch (profileDescriptionIt->item_Typ)
					{
						case SyHatch: {
							HatchObject syHatch = profileDescriptionIt;
							ProfileItem profileItemInfo = syHatch.GetProfileItemNonConst();
							Sy_HatchType hatchType;
							profileDescription->AddHatch(hatchType, syHatch);
							UInt32 index = 0;
							for (Coord coord : syHatch.GetCoords()) {
								if (coord.x < 0) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x= coord.x - deltaX / 2;
								}
								else if (coord.x > 0)
								{
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = coord.x + deltaX / 2;
								}

								if (coord.y < 0) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = coord.y - deltaY / 2;
								}
								else if (coord.y > 0) {
									static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = coord.y + deltaY / 2;
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
				err = ModifyHotSpot(item);
			}
		}
		else {
			API_Attribute          attrib;
			API_AttributeDefExt   defs;
			GSErrCode              err;
			BNZeroMemory(&attrib, sizeof(API_Attribute));
			BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
			attrib.header.typeID = API_ProfileID;
			std::wstring nameProfile = L"_toda_RCó¿-ëùÇµë≈Çø";
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

				double deltaX = item.Width - 0.4;
				double deltaY = item.Height - 0.6;

				while (indexHatchBreak <= 2)
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
									if (coord.x < 0) {
										static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = coord.x - deltaX / 2;
									}
									else if (coord.x > 0)
									{
										static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].x = coord.x + deltaX / 2;
									}

									if (coord.y < -0.0001) {
										static_cast<GS::Array<Point2D>>(syHatch.GetCoords())[index].y = coord.y - deltaY;
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
				err = ModifyHotSpot(item);
			}
		}
	}


	return NoError;
}
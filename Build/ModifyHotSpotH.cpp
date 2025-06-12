#include "ModifyHotSpotH.h"
#include	"VectorImageIterator.hpp"
#include	"ProfileVectorImageOperations.hpp"
#include	"OverriddenAttributes.hpp"
#include	"ProfileAdditionalInfo.hpp"
#include "ReadExcel.h"

//GS::ErrCode ModifyHotSpotH()
//{
//	GS::ErrCode err = NoError;
//	auto data = GetDataFromExcelForH();
//	for (auto item : data)
//	{
//		if (item.IsColumn)
//		{
//			API_Attribute          attrib;
//			API_AttributeDefExt   defs;
//			GSErrCode              err;
//
//			BNZeroMemory(&attrib, sizeof(API_Attribute));
//			BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
//			attrib.header.typeID = API_ProfileID;
//			CHCopyC(item.Name.c_str(), attrib.header.name);
//			err = ACAPI_Attribute_Get(&attrib);
//			if (err == NoError)
//				err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);
//
//			if (err == NoError) {
//				ProfileVectorImage* profileDescriptionHot = defs.profile_vectorImageItems;
//				ProfileVectorImageIterator profileDescriptionIt(*profileDescriptionHot);
//
//				double twOrigin = 0.007;
//				double tfOrigin = 0.011;
//				double widthOrigin = 0.199;
//				double heightOrigin = 0.396;
//				ProfileVectorImageIterator profileDescriptionItHot(*profileDescriptionHot);
//				while (!profileDescriptionItHot.IsEOI()) {
//					switch (profileDescriptionItHot->item_Typ)
//					{
//					case SyHots:
//					{
//						Sy_HotType* pSyHot = ((Sy_HotType*)profileDescriptionItHot);
//						int x = std::round(pSyHot->c.x * 10000);
//						int y = std::round(pSyHot->c.y * 10000);
//						if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000))
//						{
//							pSyHot->c.x = -item.Width / 2;
//							pSyHot->c.y = item.Height / 2;
//						}
//						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000))
//						{
//							pSyHot->c.x = item.Width / 2;
//							pSyHot->c.y = item.Height / 2;
//						}
//						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000))
//						{
//							pSyHot->c.x = item.Width / 2;
//							pSyHot->c.y = item.Height / 2 - item.tf;
//						}
//						else if (x == std::round(twOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000)) {
//							pSyHot->c.x = item.tw / 2;
//							pSyHot->c.y = item.Height / 2 - item.tf;
//						}
//						else if (x == std::round(twOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
//							pSyHot->c.x = item.tw / 2;
//							pSyHot->c.y = -(item.Height / 2 - item.tf);
//						}
//						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
//							pSyHot->c.x = item.Width / 2;
//							pSyHot->c.y = -(item.Height / 2 - item.tf);
//						}
//						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
//							pSyHot->c.x = item.Width / 2;
//							pSyHot->c.y = -item.Height / 2;
//						}
//						else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
//							pSyHot->c.x = -item.Width / 2;
//							pSyHot->c.y = -item.Height / 2;
//						}
//						else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
//							pSyHot->c.x = -item.Width / 2;
//							pSyHot->c.y = -(item.Height / 2 - item.tf);
//						}
//						else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
//							pSyHot->c.x = -item.tw / 2;
//							pSyHot->c.y = -(item.Height / 2 - item.tf);
//						}
//						else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000)) {
//							pSyHot->c.x = -item.tw / 2;
//							pSyHot->c.y = item.Height / 2 - item.tf;
//						}
//						else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000)) {
//							pSyHot->c.x = -item.Width / 2;
//							pSyHot->c.y = item.Height / 2 - item.tf;
//						}
//						else if (x == 0 && y == std::round(heightOrigin / 2 * 10000)) {
//							pSyHot->c.x = 0;
//							pSyHot->c.y = item.Height / 2;
//						}
//						break;
//					}
//					/*case SyLine:
//					{
//						Sy_LinType* pSyLine = ((Sy_LinType*)profileDescriptionItHot);
//						pSyLine->begC.x = -item.Width / 2 - item.Width / 4;
//						pSyLine->begC.y = item.Height / 2;
//						pSyLine->endC.x = item.Width / 2 + item.Width / 4;
//						pSyLine->endC.y = item.Height / 2;
//						break;
//					}*/
//					}
//					++profileDescriptionItHot;
//				}
//			}
//			err = ACAPI_Attribute_ModifyExt(&attrib, &defs);
//			ACAPI_DisposeAttrDefsHdlsExt(&defs);
//		}
//		else
//		{
//			API_Attribute          attrib;
//			API_AttributeDefExt   defs;
//			GSErrCode              err;
//
//			BNZeroMemory(&attrib, sizeof(API_Attribute));
//			BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
//			attrib.header.typeID = API_ProfileID;
//			CHCopyC(item.Name.c_str(), attrib.header.name);
//			err = ACAPI_Attribute_Get(&attrib);
//			if (err == NoError)
//				err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);
//
//			if (err == NoError) {
//				ProfileVectorImage* profileDescriptionHot = defs.profile_vectorImageItems;
//				ProfileVectorImageIterator profileDescriptionIt(*profileDescriptionHot);
//
//				double twOrigin = 0.007;
//				double tfOrigin = 0.011;
//				double widthOrigin = 0.199;
//				double heightOrigin = 0.396;
//				ProfileVectorImageIterator profileDescriptionItHot(*profileDescriptionHot);
//				while (!profileDescriptionItHot.IsEOI()) {
//					switch (profileDescriptionItHot->item_Typ) {
//					case SyHots:
//					{
//						Sy_HotType* pSyHot = ((Sy_HotType*)profileDescriptionItHot);
//						int x = std::round(pSyHot->c.x * 10000);
//						int y = std::round(pSyHot->c.y * 10000);
//						if (x == std::round(-widthOrigin / 2 * 10000) && y == 0)
//						{
//							pSyHot->c.x = -item.Width / 2;
//							pSyHot->c.y = 0;
//						}
//						else if (x == std::round(widthOrigin / 2 * 10000) && y == 0)
//						{
//							pSyHot->c.x = item.Width / 2;
//							pSyHot->c.y = 0;
//						}
//						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000))
//						{
//							pSyHot->c.x = item.Width / 2;
//							pSyHot->c.y = -item.tf;
//						}
//						else if (x == std::round(twOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000)) {
//							pSyHot->c.x = item.tw / 2;
//							pSyHot->c.y = -item.tf;
//						}
//						else if (x == std::round(twOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
//							pSyHot->c.x = item.tw / 2;
//							pSyHot->c.y = -(item.Height - item.tf);
//						}
//						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
//							pSyHot->c.x = item.Width / 2;
//							pSyHot->c.y = -(item.Height - item.tf);
//						}
//						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-heightOrigin * 10000)) {
//							pSyHot->c.x = item.Width / 2;
//							pSyHot->c.y = -item.Height;
//						}
//						else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-heightOrigin * 10000)) {
//							pSyHot->c.x = -item.Width / 2;
//							pSyHot->c.y = -item.Height;
//						}
//						else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
//							pSyHot->c.x = -item.Width / 2;
//							pSyHot->c.y = -(item.Height - item.tf);
//						}
//						else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
//							pSyHot->c.x = -item.tw / 2;
//							pSyHot->c.y = -(item.Height - item.tf);
//						}
//						else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000)) {
//							pSyHot->c.x = -item.tw / 2;
//							pSyHot->c.y = -item.tf;
//						}
//						else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000)) {
//							pSyHot->c.x = -item.Width / 2;
//							pSyHot->c.y = -item.tf;
//						}
//						else if (x == 0 && y == std::round(-heightOrigin / 2 * 10000)) {
//							pSyHot->c.x = 0;
//							pSyHot->c.y = -item.Height / 2;
//						}
//						break;
//					}
//					}
//					++profileDescriptionItHot;
//				}
//			}
//
//			err = ACAPI_Attribute_ModifyExt(&attrib, &defs);
//			ACAPI_DisposeAttrDefsHdlsExt(&defs);
//		}
//	}
//
//	return err;
//}

GS::ErrCode ModifyHotSpotH(DataProfileH& item)
{
	GS::ErrCode err = NoError;
	if (item.IsColumn)
	{
		API_Attribute          attrib;
		API_AttributeDefExt   defs;
		GSErrCode              err;

		BNZeroMemory(&attrib, sizeof(API_Attribute));
		BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
		attrib.header.typeID = API_ProfileID;
		CHCopyC(item.Name.c_str(), attrib.header.name);
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError)
			err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);

		if (err == NoError) {
			ProfileVectorImage* profileDescriptionHot = defs.profile_vectorImageItems;
			ProfileVectorImageIterator profileDescriptionIt(*profileDescriptionHot);

			double twOrigin = 0.007;
			double tfOrigin = 0.011;
			double widthOrigin = 0.199;
			double heightOrigin = 0.396;
			ProfileVectorImageIterator profileDescriptionItHot(*profileDescriptionHot);
			while (!profileDescriptionItHot.IsEOI()) {
				switch (profileDescriptionItHot->item_Typ)
				{
					case SyHots:
					{
						Sy_HotType* pSyHot = ((Sy_HotType*)profileDescriptionItHot);
						int x = std::round(pSyHot->c.x * 10000);
						int y = std::round(pSyHot->c.y * 10000);
						if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000))
						{
							pSyHot->c.x = -item.Width / 2;
							pSyHot->c.y = item.Height / 2;
						}
						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000))
						{
							pSyHot->c.x = item.Width / 2;
							pSyHot->c.y = item.Height / 2;
						}
						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000))
						{
							pSyHot->c.x = item.Width / 2;
							pSyHot->c.y = item.Height / 2 - item.tf;
						}
						else if (x == std::round(twOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000)) {
							pSyHot->c.x = item.tw / 2;
							pSyHot->c.y = item.Height / 2 - item.tf;
						}
						else if (x == std::round(twOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
							pSyHot->c.x = item.tw / 2;
							pSyHot->c.y = -(item.Height / 2 - item.tf);
						}
						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
							pSyHot->c.x = item.Width / 2;
							pSyHot->c.y = -(item.Height / 2 - item.tf);
						}
						else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
							pSyHot->c.x = item.Width / 2;
							pSyHot->c.y = -item.Height / 2;
						}
						else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
							pSyHot->c.x = -item.Width / 2;
							pSyHot->c.y = -item.Height / 2;
						}
						else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
							pSyHot->c.x = -item.Width / 2;
							pSyHot->c.y = -(item.Height / 2 - item.tf);
						}
						else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round(-(heightOrigin / 2 - tfOrigin) * 10000)) {
							pSyHot->c.x = -item.tw / 2;
							pSyHot->c.y = -(item.Height / 2 - item.tf);
						}
						else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000)) {
							pSyHot->c.x = -item.tw / 2;
							pSyHot->c.y = item.Height / 2 - item.tf;
						}
						else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round((heightOrigin / 2 - tfOrigin) * 10000)) {
							pSyHot->c.x = -item.Width / 2;
							pSyHot->c.y = item.Height / 2 - item.tf;
						}
						else if (x == 0 && y == std::round(heightOrigin / 2 * 10000)) {
							pSyHot->c.x = 0;
							pSyHot->c.y = item.Height / 2;
						}
						break;
					}
					case SyLine:
					{
						Sy_LinType* pSyLine = ((Sy_LinType*)profileDescriptionItHot);
						pSyLine->begC.x = -item.Width / 2 - item.Width / 4;
						pSyLine->begC.y = item.Height / 2;
						pSyLine->endC.x = item.Width / 2 + item.Width / 4;
						pSyLine->endC.y = item.Height / 2;
						break;
					}
				}
				++profileDescriptionItHot;
			}
		}
		err = ACAPI_Attribute_ModifyExt(&attrib, &defs);
		ACAPI_DisposeAttrDefsHdlsExt(&defs);
	}
	else
	{
		API_Attribute          attrib;
		API_AttributeDefExt   defs;
		GSErrCode              err;

		BNZeroMemory(&attrib, sizeof(API_Attribute));
		BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
		attrib.header.typeID = API_ProfileID;
		CHCopyC(item.Name.c_str(), attrib.header.name);
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError)
			err = ACAPI_Attribute_GetDefExt(attrib.header.typeID, attrib.header.index, &defs);

		if (err == NoError) {
			ProfileVectorImage* profileDescriptionHot = defs.profile_vectorImageItems;
			ProfileVectorImageIterator profileDescriptionIt(*profileDescriptionHot);

			double twOrigin = 0.007;
			double tfOrigin = 0.011;
			double widthOrigin = 0.199;
			double heightOrigin = 0.396;
			ProfileVectorImageIterator profileDescriptionItHot(*profileDescriptionHot);
			while (!profileDescriptionItHot.IsEOI()) {
				switch (profileDescriptionItHot->item_Typ) {
				case SyHots:
				{
					Sy_HotType* pSyHot = ((Sy_HotType*)profileDescriptionItHot);
					int x = std::round(pSyHot->c.x * 10000);
					int y = std::round(pSyHot->c.y * 10000);
					if (x == std::round(-widthOrigin / 2 * 10000) && y == 0)
					{
						pSyHot->c.x = -item.Width / 2;
						pSyHot->c.y = 0;
					}
					else if (x == std::round(widthOrigin / 2 * 10000) && y == 0)
					{
						pSyHot->c.x = item.Width / 2;
						pSyHot->c.y = 0;
					}
					else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000))
					{
						pSyHot->c.x = item.Width / 2;
						pSyHot->c.y = -item.tf;
					}
					else if (x == std::round(twOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000)) {
						pSyHot->c.x = item.tw / 2;
						pSyHot->c.y = -item.tf;
					}
					else if (x == std::round(twOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
						pSyHot->c.x = item.tw / 2;
						pSyHot->c.y = -(item.Height - item.tf);
					}
					else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
						pSyHot->c.x = item.Width / 2;
						pSyHot->c.y = -(item.Height - item.tf);
					}
					else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-heightOrigin * 10000)) {
						pSyHot->c.x = item.Width / 2;
						pSyHot->c.y = -item.Height;
					}
					else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-heightOrigin * 10000)) {
						pSyHot->c.x = -item.Width / 2;
						pSyHot->c.y = -item.Height;
					}
					else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
						pSyHot->c.x = -item.Width / 2;
						pSyHot->c.y = -(item.Height - item.tf);
					}
					else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round(-(heightOrigin - tfOrigin) * 10000)) {
						pSyHot->c.x = -item.tw / 2;
						pSyHot->c.y = -(item.Height - item.tf);
					}
					else if (x == std::round(-twOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000)) {
						pSyHot->c.x = -item.tw / 2;
						pSyHot->c.y = -item.tf;
					}
					else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-tfOrigin * 10000)) {
						pSyHot->c.x = -item.Width / 2;
						pSyHot->c.y = -item.tf;
					}
					else if (x == 0 && y == std::round(-heightOrigin / 2 * 10000)) {
						pSyHot->c.x = 0;
						pSyHot->c.y = -item.Height / 2;
					}
					break;
				}
				}
				++profileDescriptionItHot;
			}
		}

		err = ACAPI_Attribute_ModifyExt(&attrib, &defs);
		ACAPI_DisposeAttrDefsHdlsExt(&defs);
	}

	return err;
}
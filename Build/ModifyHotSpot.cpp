#include "ModifyHotSpot.h"
#include	"VectorImageIterator.hpp"
#include	"ProfileVectorImageOperations.hpp"
#include	"OverriddenAttributes.hpp"
#include	"ProfileAdditionalInfo.hpp"
#include "ReadExcel.h"

//GS::ErrCode ModifyHotSpot()
//{
//	auto data = GetDataFromExcel();
//
//	for (auto item : data) {
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
//				double width = item.Width;
//				double height = item.Height;
//				double widthHot = 0.5;
//				double heightHot = 0.5;
//				double deltaX = (width - widthHot) / 2;
//				double deltaY = (height - heightHot) / 2;
//				ProfileVectorImageIterator profileDescriptionItHot(*profileDescriptionHot);
//				while (!profileDescriptionItHot.IsEOI()) {
//					switch (profileDescriptionItHot->item_Typ) {
//					case SyHots:
//					{
//						Sy_HotType* pSyHot = ((Sy_HotType*)profileDescriptionItHot);
//						if (pSyHot->c.x < 0)
//							pSyHot->c.x = pSyHot->c.x - deltaX;
//						else if (pSyHot->c.x > 0)
//							pSyHot->c.x = pSyHot->c.x + deltaX;
//
//						if (pSyHot->c.y < 0)
//							pSyHot->c.y = pSyHot->c.y - deltaY;
//						else if (pSyHot->c.y > 0)
//							pSyHot->c.y = pSyHot->c.y + deltaY;
//					}
//					break;
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
//				double width = item.Width;
//				double height = item.Height;
//				double widthHot = 0.4;
//				double heightHot = 0.6;
//				double deltaX = (width - widthHot) / 2;
//				double deltaY = (height - heightHot) / 2;
//				ProfileVectorImageIterator profileDescriptionItHot(*profileDescriptionHot);
//				while (!profileDescriptionItHot.IsEOI()) {
//					switch (profileDescriptionItHot->item_Typ) {
//					case SyHots:
//					{
//						Sy_HotType* pSyHot = ((Sy_HotType*)profileDescriptionItHot);
//						if (pSyHot->c.x < -0.0001)
//							pSyHot->c.x = pSyHot->c.x - deltaX;
//						else if (pSyHot->c.x > 0.0001)
//							pSyHot->c.x = pSyHot->c.x + deltaX;
//
//						if (pSyHot->c.y < -0.5)
//							pSyHot->c.y = pSyHot->c.y - deltaY * 2;
//						else if (pSyHot->c.y < -0.2)
//							pSyHot->c.y = pSyHot->c.y - deltaY;
//					}
//					break;
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
//	return GS::ErrCode();
//}

GS::ErrCode ModifyHotSpot(DataProfile& item)
{
	auto data = GetDataFromExcel();
	GS::ErrCode err = NoError;

	if (item.IsColumn)
	{
		API_Attribute          attrib;
		API_AttributeDefExt   defs;

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

			double width = item.Width;
			double height = item.Height;
			double widthHot = 0.5;
			double heightHot = 0.5;
			double deltaX = (width - widthHot) / 2;
			double deltaY = (height - heightHot) / 2;
			ProfileVectorImageIterator profileDescriptionItHot(*profileDescriptionHot);
			while (!profileDescriptionItHot.IsEOI()) {
				switch (profileDescriptionItHot->item_Typ) {
				case SyHots:
				{
					Sy_HotType* pSyHot = ((Sy_HotType*)profileDescriptionItHot);
					if (pSyHot->c.x < 0)
						pSyHot->c.x = pSyHot->c.x - deltaX;
					else if (pSyHot->c.x > 0)
						pSyHot->c.x = pSyHot->c.x + deltaX;

					if (pSyHot->c.y < 0)
						pSyHot->c.y = pSyHot->c.y - deltaY;
					else if (pSyHot->c.y > 0)
						pSyHot->c.y = pSyHot->c.y + deltaY;
				}
				break;
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

			double width = item.Width;
			double height = item.Height;
			double widthHot = 0.4;
			double heightHot = 0.6;
			double deltaX = (width - widthHot) / 2;
			double deltaY = (height - heightHot) / 2;
			ProfileVectorImageIterator profileDescriptionItHot(*profileDescriptionHot);
			while (!profileDescriptionItHot.IsEOI()) {
				switch (profileDescriptionItHot->item_Typ) {
				case SyHots:
				{
					Sy_HotType* pSyHot = ((Sy_HotType*)profileDescriptionItHot);
					if (pSyHot->c.x < -0.0001)
						pSyHot->c.x = pSyHot->c.x - deltaX;
					else if (pSyHot->c.x > 0.0001)
						pSyHot->c.x = pSyHot->c.x + deltaX;

					if (pSyHot->c.y < -0.5)
						pSyHot->c.y = pSyHot->c.y - deltaY * 2;
					else if (pSyHot->c.y < -0.2)
						pSyHot->c.y = pSyHot->c.y - deltaY;
				}
				break;
				}
				++profileDescriptionItHot;
			}
		}

		err = ACAPI_Attribute_ModifyExt(&attrib, &defs);
		ACAPI_DisposeAttrDefsHdlsExt(&defs);
	}

	return err;
}
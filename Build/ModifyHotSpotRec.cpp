#include "ModifyHotSpotRec.h"
#include	"VectorImageIterator.hpp"
#include	"ProfileVectorImageOperations.hpp"
#include	"OverriddenAttributes.hpp"
#include	"ProfileAdditionalInfo.hpp"
#include "ReadExcel.h"


GS::ErrCode ModifyHotSpotRecSteel(DataProfileRecSteel& item)
{
	GS::ErrCode err = NoError;
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
		double thickOrigin = 0.022;
		double widthOrigin = 0.6;
		double heightOrigin = 0.6;
		ProfileVectorImageIterator profileDescriptionItHot(*profileDescriptionHot);
		while (!profileDescriptionItHot.IsEOI()) {
			switch (profileDescriptionItHot->item_Typ) {
				case SyHots: 
				{
					Sy_HotType* pSyHot = ((Sy_HotType*)profileDescriptionItHot);
					int x = std::round(pSyHot->c.x * 10000);
					int y = std::round(pSyHot->c.y * 10000);

					if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000)) {
						pSyHot->c.x = -item.Width / 2;
						pSyHot->c.y = item.Height / 2;
					}
					else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(heightOrigin / 2 * 10000)) {
						pSyHot->c.x = item.Width / 2;
						pSyHot->c.y = item.Height / 2;
					}
					else if (x == std::round(widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
						pSyHot->c.x = item.Width / 2;
						pSyHot->c.y = -item.Height / 2;
					}
					else if (x == std::round(-widthOrigin / 2 * 10000) && y == std::round(-heightOrigin / 2 * 10000)) {
						pSyHot->c.x = -item.Width / 2;
						pSyHot->c.y = -item.Height / 2;
					}
					else if (x == std::round(-(widthOrigin / 2 - thickOrigin) * 10000) && y == std::round((heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyHot->c.x = -(item.Width / 2 - item.Thickness);
						pSyHot->c.y = (item.Height / 2 - item.Thickness);
					}
					else if (x == std::round((widthOrigin / 2 - thickOrigin) * 10000) && y == std::round((heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyHot->c.x = (item.Width / 2 - item.Thickness);
						pSyHot->c.y = (item.Height / 2 - item.Thickness);
					}
					else if (x == std::round((widthOrigin / 2 - thickOrigin) * 10000) && y == std::round(-(heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyHot->c.x = (item.Width / 2 - item.Thickness);
						pSyHot->c.y = -(item.Height / 2 - item.Thickness);
					}
					else if (x == std::round(-(widthOrigin / 2 - thickOrigin) * 10000) && y == std::round(-(heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyHot->c.x = -(item.Width / 2 - item.Thickness);
						pSyHot->c.y = -(item.Height / 2 - item.Thickness);
					}
					else if (x == 0 && y == std::round(heightOrigin / 2 * 10000)) {
						pSyHot->c.x = 0;
						pSyHot->c.y = item.Height / 2;
					}
					else if (x == std::round(widthOrigin / 2 * 10000) && y == 0) {
						pSyHot->c.x = item.Width/2;
						pSyHot->c.y = 0;
					}
					else if (x == 0 && y == std::round(-heightOrigin / 2 * 10000)) {
						pSyHot->c.x = 0;
						pSyHot->c.y = -item.Height / 2;
					}
					else if (x == std::round(-widthOrigin / 2 * 10000) && y == 0) {
						pSyHot->c.x = -item.Width / 2;
						pSyHot->c.y = 0;
					}
					break;
				}
				case SyLine: 
				{
					Sy_LinType* pSyLine = ((Sy_LinType*)profileDescriptionItHot);
					int xb = std::round(pSyLine->begC.x * 10000);
					int yb = std::round(pSyLine->begC.y * 10000);
					
					if (xb == std::round(-widthOrigin / 2 * 10000) && yb == std::round(heightOrigin / 2 * 10000)) {
						pSyLine->begC.x = -item.Width / 2;
						pSyLine->begC.y = item.Height / 2;
					}
					else if (xb == std::round(widthOrigin / 2 * 10000) && yb == std::round(heightOrigin / 2 * 10000)) {
						pSyLine->begC.x = item.Width / 2;
						pSyLine->begC.y = item.Height / 2;
					}
					else if (xb == std::round(widthOrigin / 2 * 10000) && yb == std::round(-heightOrigin / 2 * 10000)) {
						pSyLine->begC.x = item.Width / 2;
						pSyLine->begC.y = -item.Height / 2;
					}
					else if (xb == std::round(-widthOrigin / 2 * 10000) && yb == std::round(-heightOrigin / 2 * 10000)) {
						pSyLine->begC.x = -item.Width / 2;
						pSyLine->begC.y = -item.Height / 2;
					}
					else if (xb == std::round(-(widthOrigin / 2 - thickOrigin) * 10000) && yb == std::round((heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyLine->begC.x = -(item.Width / 2 - item.Thickness);
						pSyLine->begC.y = (item.Height / 2 - item.Thickness);
					}
					else if (xb == std::round((widthOrigin / 2 - thickOrigin) * 10000) && yb == std::round((heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyLine->begC.x = (item.Width / 2 - item.Thickness);
						pSyLine->begC.y = (item.Height / 2 - item.Thickness);
					}
					else if (xb == std::round((widthOrigin / 2 - thickOrigin) * 10000) && yb == std::round(-(heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyLine->begC.x = (item.Width / 2 - item.Thickness);
						pSyLine->begC.y = -(item.Height / 2 - item.Thickness);
					}
					else if (xb == std::round(-(widthOrigin / 2 - thickOrigin) * 10000) && yb == std::round(-(heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyLine->begC.x = -(item.Width / 2 - item.Thickness);
						pSyLine->begC.y = -(item.Height / 2 - item.Thickness);
					}


					int xe = std::round(pSyLine->endC.x * 10000);
					int ye = std::round(pSyLine->endC.y * 10000);
					if (xe == std::round(-widthOrigin / 2 * 10000) && ye == std::round(heightOrigin / 2 * 10000)) {
						pSyLine->endC.x = -item.Width / 2;
						pSyLine->endC.y = item.Height / 2;
					}
					else if (xe == std::round(widthOrigin / 2 * 10000) && ye == std::round(heightOrigin / 2 * 10000)) {
						pSyLine->endC.x = item.Width / 2;
						pSyLine->endC.y = item.Height / 2;
					}
					else if (xe == std::round(widthOrigin / 2 * 10000) && ye == std::round(-heightOrigin / 2 * 10000)) {
						pSyLine->endC.x = item.Width / 2;
						pSyLine->endC.y = -item.Height / 2;
					}
					else if (xe == std::round(-widthOrigin / 2 * 10000) && ye == std::round(-heightOrigin / 2 * 10000)) {
						pSyLine->endC.x = -item.Width / 2;
						pSyLine->endC.y = -item.Height / 2;
					}
					else if (xe == std::round(-(widthOrigin / 2 - thickOrigin) * 10000) && ye == std::round((heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyLine->endC.x = -(item.Width / 2 - item.Thickness);
						pSyLine->endC.y = (item.Height / 2 - item.Thickness);
					}
					else if (xe == std::round((widthOrigin / 2 - thickOrigin) * 10000) && ye == std::round((heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyLine->endC.x = (item.Width / 2 - item.Thickness);
						pSyLine->endC.y = (item.Height / 2 - item.Thickness);
					}
					else if (xe == std::round((widthOrigin / 2 - thickOrigin) * 10000) && ye == std::round(-(heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyLine->endC.x = (item.Width / 2 - item.Thickness);
						pSyLine->endC.y = -(item.Height / 2 - item.Thickness);
					}
					else if (xe == std::round(-(widthOrigin / 2 - thickOrigin) * 10000) && ye == std::round(-(heightOrigin / 2 - thickOrigin) * 10000)) {
						pSyLine->endC.x = -(item.Width / 2 - item.Thickness);
						pSyLine->endC.y = -(item.Height / 2 - item.Thickness);
					}

					break;
				}
			}
			++profileDescriptionItHot;
		}
		
		err = ACAPI_Attribute_ModifyExt(&attrib, &defs);
		ACAPI_DisposeAttrDefsHdlsExt(&defs);
	}

	return NoError;
}

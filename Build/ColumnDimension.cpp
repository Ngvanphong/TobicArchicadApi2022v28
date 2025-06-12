#include "ColumnDimension.h"
#include <corecrt_math_defines.h>
#define  ExtendFromSideColumn 0.6
std::vector<API_Element> SelectionColumnWithGrid(GS::Array<API_Neig> selNeigs);

GS::Array<API_Coord> GetPointCornerColumn(const GS::Array<API_Coord> arrayCoord, bool isX) {
	GS::Array<API_Coord> result;
	API_Coord pointMin;
	API_Coord pointMax;
	double Min = 10e9;
	double Max = -10e9;
	if (isX) {
		for (API_Coord item : arrayCoord) {
			if (Min > item.x) {
				Min = item.x;
				pointMin = item;
			}
			if (Max < item.x) {
				Max = item.x;
				pointMax = item;
			}
		}
	}
	else {
		for (API_Coord item : arrayCoord) {
			if (Min > item.y) {
				Min = item.y;
				pointMin = item;
			}
			if (Max < item.y) {
				Max = item.y;
				pointMax = item;
			}
		}
	}
	result.Push(pointMin);
	result.Push(pointMax);
	return result;
}

void ChangeMemoWitnessTypeOfDimension(API_Element& dimension) {
	API_ElementMemo memoDim;
	BNZeroMemory(&memoDim, sizeof(API_ElementMemo));
	GS::ErrCode err = NoError;
	err = ACAPI_Element_GetMemo(dimension.header.guid, &memoDim, APIMemoMask_AdditionalPolygon);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return;
	}
	(*memoDim.dimElems)[0].witnessForm = APIWtn_None;
	err = ACAPI_Element_ChangeMemo(dimension.header.guid, APIMemoMask_AdditionalPolygon, &memoDim);
	ACAPI_DisposeElemMemoHdls(&memoDim);
}

GS::ErrCode NewDimensionByLayerIndex(API_Element& newDim, API_ElementMemo& memoDim, const API_Element column,
	bool isX, const API_Coord pointPutDim, int indexLayer, const GS::Array<API_Coord> arrayCoordDim, API_Element* lineGrid)
{
	GS::ErrCode err = NoError;
	BNZeroMemory(&newDim, sizeof(API_Element));
	BNZeroMemory(&memoDim, sizeof(API_ElementMemo));
	newDim.header.type.typeID = API_DimensionID;

	err = ACAPI_Element_GetDefaults(&newDim, &memoDim);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}
	if (isX) {
		newDim.dimension.direction = { 1,0 };
	}
	else {
		newDim.dimension.direction = { 0,1 };
	}
	newDim.dimension.defStaticDim = false;
	SetTypeForDimensionColumn(newDim);
	newDim.dimension.refC = pointPutDim;

	if (indexLayer == 1) {
		newDim.dimension.nDimElem = 2;
		memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
		if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
			ACAPI_DisposeElemMemoHdls(&memoDim);
			return err;
		}
		GS::Array<API_Coord> arrayCorner = GetPointCornerColumn(arrayCoordDim, isX);
		for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
			API_DimElem& dimElem = (*memoDim.dimElems)[i];
			dimElem.base.loc = arrayCorner.Get(i);
			dimElem.base.base.special = false;
			dimElem.base.base.node_typ = 0;
			dimElem.base.base.node_status = 0;
			dimElem.base.base.node_id = 0;
			dimElem.note = newDim.dimension.defNote;
			dimElem.witnessVal = newDim.dimension.defWitnessVal;
			dimElem.witnessForm = newDim.dimension.defWitnessForm;
			dimElem.fixedPos = false;
		}

		err = ACAPI_Element_Create(&newDim, &memoDim);
		if (err != NoError)
			ACAPI_DisposeElemMemoHdls(&memoDim);
		ACAPI_DisposeElemMemoHdls(&memoDim);
	}
	else if (indexLayer == 2) {
		if (lineGrid != nullptr) {
			newDim.dimension.nDimElem = 3;
			memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
			if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
				ACAPI_DisposeElemMemoHdls(&memoDim);
				return err;
			}
			GS::Array<API_Coord> arrayCorner = GetPointCornerColumn(arrayCoordDim, isX);
			for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
				API_DimElem& dimElem = (*memoDim.dimElems)[i];
				if (i == 0 || i == 2) {
					dimElem.base.loc = i == 0 ? arrayCoordDim.Get(0) : arrayCoordDim.Get(1);
					dimElem.base.base.special = false;
				}
				else if ((*lineGrid).line.head.type.typeID == API_LineID)
				{
					dimElem.base.base.type.typeID = API_LineID;
					dimElem.base.base.line = false;
					dimElem.base.base.guid = (*lineGrid).header.guid;
					dimElem.base.base.special = false;
					dimElem.base.base.inIndex = 1;
				}
				else {
					dimElem.base.base.type.typeID = API_ObjectID;
					dimElem.base.base.line = false;
					dimElem.base.base.guid = (*lineGrid).header.guid;
					dimElem.base.base.special = false;
					dimElem.base.base.inIndex = 1;
				}

				dimElem.witnessVal = newDim.dimension.defWitnessVal;
				dimElem.witnessForm = newDim.dimension.defWitnessForm;
				dimElem.base.base.node_typ = 0;
				dimElem.base.base.node_status = 0;
				dimElem.base.base.node_id = 0;
				dimElem.note = newDim.dimension.defNote;
				dimElem.fixedPos = false;
			}
			err = ACAPI_Element_Create(&newDim, &memoDim);
			if (err != NoError)
				ACAPI_DisposeElemMemoHdls(&memoDim);
			ACAPI_DisposeElemMemoHdls(&memoDim);
			ChangeMemoWitnessTypeOfDimension(newDim);
		}
		else {
			newDim.dimension.nDimElem = arrayCoordDim.GetSize();
			memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
			if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
				ACAPI_DisposeElemMemoHdls(&memoDim);
				return err;
			}
			for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
				API_DimElem& dimElem = (*memoDim.dimElems)[i];
				dimElem.base.loc = arrayCoordDim.Get(i);
				dimElem.base.base.special = false;
				dimElem.base.base.node_typ = 0;
				dimElem.base.base.node_status = 0;
				dimElem.base.base.node_id = 0;
				dimElem.note = newDim.dimension.defNote;
				dimElem.witnessVal = newDim.dimension.defWitnessVal;
				dimElem.witnessForm = newDim.dimension.defWitnessForm;

				dimElem.fixedPos = false;
			}
			err = ACAPI_Element_Create(&newDim, &memoDim);
			if (err != NoError)
				ACAPI_DisposeElemMemoHdls(&memoDim);
			ACAPI_DisposeElemMemoHdls(&memoDim);
		}
	}
	else if (indexLayer == 3) {
		newDim.dimension.nDimElem = arrayCoordDim.GetSize();
		memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
		if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
			ACAPI_DisposeElemMemoHdls(&memoDim);
			return err;
		}
		for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
			API_DimElem& dimElem = (*memoDim.dimElems)[i];
			dimElem.base.loc = arrayCoordDim.Get(i);
			dimElem.base.base.special = false;
			dimElem.base.base.node_typ = 0;
			dimElem.base.base.node_status = 0;
			dimElem.base.base.node_id = 0;
			dimElem.note = newDim.dimension.defNote;
			dimElem.witnessVal = newDim.dimension.defWitnessVal;
			dimElem.witnessForm = newDim.dimension.defWitnessForm;
			dimElem.fixedPos = false;
		}
		err = ACAPI_Element_Create(&newDim, &memoDim);
		if (err != NoError)
			ACAPI_DisposeElemMemoHdls(&memoDim);
		ACAPI_DisposeElemMemoHdls(&memoDim);
	}

	return err;
}

GSErrCode CreateDimensionWithGrid(double distance) {
	GSErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> arrayElements = SelectionColumnWithGrid(selNeigs);
	API_Element column = arrayElements[0];
	API_Coord locationColumn = column.column.origoPos;
	GS::Array<API_Coord> arrayHotspotY;
	GS::Array<API_Coord> arrayHotspotX = GetHotspotColumnToDimX(column, locationColumn, &arrayHotspotY);

	///Dim for x
	int totalLayerDimX = 1;
	API_Element* lineGridX = nullptr;
	if (arrayElements.size() > 1) {
		for (int i = 1; i < arrayElements.size(); i++) {
			API_Element lineGrid = arrayElements[i];
			if (lineGrid.header.type.typeID == API_ObjectID) {
				if (std::abs(std::abs(lineGrid.object.angle) - M_PI / 2) < 0.001 ||
					std::abs(std::abs(lineGrid.object.angle) - 3 * M_PI / 2) < 0.001) {
					lineGridX = &arrayElements[i];
					totalLayerDimX = 2;
				}
			}
			else if (lineGrid.header.type.typeID == API_LineID) {
				if (std::abs(lineGrid.line.begC.y - lineGrid.line.endC.y) < 0.0001) {
					lineGridX = &arrayElements[i];
					totalLayerDimX = 2;
				}
			}
		}
	}
	if (arrayHotspotX.GetSize() > 2) {
		totalLayerDimX += 1;
	}

	API_Element newDim;
	API_ElementMemo memoDim;

	double scale = 60;
	err = ACAPI_Drawing_GetDrawingScale(&scale);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	GS::Array<API_Coord> arrayCornerX = GetPointCornerColumn(arrayHotspotX, true);
	GS::Array<API_Coord> arrayCornerY = GetPointCornerColumn(arrayHotspotY, false);
	std::pair<double, double> sx = { arrayCornerX.Get(0).x,arrayCornerX.Get(0).y };
	std::pair<double, double> ex = { arrayCornerX.Get(1).x,arrayCornerX.Get(1).y };
	std::pair<double, double> sy = { arrayCornerY.Get(0).x,arrayCornerY.Get(0).y };
	std::pair<double, double> ey = { arrayCornerY.Get(1).x,arrayCornerY.Get(1).y };
	double width = calculateDistance(sx, ex);
	double depth = calculateDistance(sy, ey);

	double multiFactorX = ExtendFromSideColumn * scale / 60;
	API_Coord pointPutDim = { locationColumn.x ,locationColumn.y - depth / 2 - multiFactorX };

	if (totalLayerDimX == 3) {
		err = NewDimensionByLayerIndex(newDim, memoDim, column, true, pointPutDim, 3, arrayHotspotX, lineGridX);

		double multiFactorX2 = (ExtendFromSideColumn + distance) * scale / 60;
		API_Coord pointPutDim2 = { locationColumn.x ,locationColumn.y - depth / 2 - multiFactorX2 };
		err = NewDimensionByLayerIndex(newDim, memoDim, column, true, pointPutDim2, 2, arrayHotspotX, lineGridX);

		double multiFactorX3 = (ExtendFromSideColumn + distance + distance) * scale / 60;
		API_Coord pointPutDim3 = { locationColumn.x ,locationColumn.y - depth / 2 - multiFactorX3 };
		err = NewDimensionByLayerIndex(newDim, memoDim, column, true, pointPutDim3, 1, arrayHotspotX, lineGridX);
	}
	else if (totalLayerDimX == 2) {
		err = NewDimensionByLayerIndex(newDim, memoDim, column, true, pointPutDim, 2, arrayHotspotX, lineGridX);

		double multiFactorX2 = (ExtendFromSideColumn + distance) * scale / 60;
		API_Coord pointPutDim2 = { locationColumn.x ,locationColumn.y - depth / 2 - multiFactorX2 };
		err = NewDimensionByLayerIndex(newDim, memoDim, column, true, pointPutDim2, 1, arrayHotspotX, lineGridX);
	}
	else {
		err = NewDimensionByLayerIndex(newDim, memoDim, column, true, pointPutDim, 1, arrayHotspotX, lineGridX);
	}

	///Dimension for y
	int totalLayerDimY = 1;
	API_Element* lineGridY = nullptr;
	if (arrayElements.size() > 1) {
		for (int i = 1; i < arrayElements.size(); i++) {
			API_Element lineGridItemY = arrayElements[i];
			if (lineGridItemY.header.type.typeID == API_ObjectID) {
				if (std::abs(lineGridItemY.object.angle) < 0.001 ||
					std::abs(std::abs(lineGridItemY.object.angle) - M_PI) < 0.001) {
					lineGridY = &arrayElements[i];
					totalLayerDimY = 2;
				}
			}
			else if (lineGridItemY.header.type.typeID == API_LineID) {
				if (std::abs(lineGridItemY.line.begC.x - lineGridItemY.line.endC.x) < 0.0001) {
					lineGridY = &arrayElements[i];
					totalLayerDimY = 2;
				}
			}
		}
	}
	if (arrayHotspotY.GetSize() > 2) {
		totalLayerDimY += 1;
	}
	double multiFactorY = ExtendFromSideColumn * scale / 60;
	API_Coord pointPutDimY = { locationColumn.x + width / 2 + multiFactorY,locationColumn.y };
	if (totalLayerDimY == 3) {
		err = NewDimensionByLayerIndex(newDim, memoDim, column, false, pointPutDimY, 3, arrayHotspotY, lineGridY);

		double multiFactorY2 = (ExtendFromSideColumn + distance) * scale / 60;
		API_Coord pointPutDim2 = { locationColumn.x + width / 2 + multiFactorY2 ,locationColumn.y };
		err = NewDimensionByLayerIndex(newDim, memoDim, column, false, pointPutDim2, 2, arrayHotspotY, lineGridY);

		double multiFactorY3 = (ExtendFromSideColumn + distance + distance) * scale / 60;
		API_Coord pointPutDim3 = { locationColumn.x + width / 2 + multiFactorY3  ,locationColumn.y };
		err = NewDimensionByLayerIndex(newDim, memoDim, column, false, pointPutDim3, 1, arrayHotspotY, lineGridY);
	}
	else if (totalLayerDimY == 2) {
		err = NewDimensionByLayerIndex(newDim, memoDim, column, false, pointPutDimY, 2, arrayHotspotY, lineGridY);

		double multiFactorY2 = (ExtendFromSideColumn + distance) * scale / 60;
		API_Coord pointPutDim2 = { locationColumn.x + width / 2 + multiFactorY2,locationColumn.y };
		err = NewDimensionByLayerIndex(newDim, memoDim, column, false, pointPutDim2, 1, arrayHotspotY, lineGridY);
	}
	else {
		err = NewDimensionByLayerIndex(newDim, memoDim, column, false, pointPutDimY, 1, arrayHotspotY, lineGridY);
	}

	return NoError;
}

/// <summary>
/// selection column with gird and line
/// </summary>
/// <param name="selNeigs"></param>
/// <returns></returns>
std::vector<API_Element> SelectionColumnWithGrid(GS::Array<API_Neig> selNeigs) {
	std::vector<API_Element> vectorElements;
	API_SelectionInfo selectionInfo;
	GS::ErrCode err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (selectionInfo.typeID == API_SelEmpty) {
		return vectorElements;
	}
	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
		API_Element element;
		BNClear(element);
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (element.header.type.typeID == API_ColumnID) {
			vectorElements.insert(vectorElements.begin(), element);
		}
		else if (element.header.type.typeID == API_LineID) {
			vectorElements.insert(vectorElements.end(), element);
		}
		else {
			vectorElements.insert(vectorElements.end(), element);
		}
	}
	return vectorElements;
}
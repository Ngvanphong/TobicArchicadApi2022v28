#include "WalllWithLineDimenison.h"

static API_Coord pointPutDimension;

API_Element* SelectAllElement(GS::Array<API_Neig>);

GSErrCode CreateDimensionWallWithLine(double distance) {
	
	GS::Array<API_Neig> selNeigs;
	API_Element* arrayElement = SelectAllElement(selNeigs);
	API_Element wall = *arrayElement;
	API_Element line = *(arrayElement + 1);

	API_Coord begin = wall.wall.begC;
	API_Coord end = wall.wall.endC;
	API_Coord locationDim = pointPutDimension;

	API_Coord* pointBeginEnd = SetDirectionElementToDim(begin, end);
	API_Coord directionWall = { (*(pointBeginEnd + 1)).x - (*pointBeginEnd).x ,(*(pointBeginEnd + 1)).y - (*pointBeginEnd).y };

	API_Element			newDim;
	API_ElementMemo		memoDim;
	GSErrCode			err = NoError;

	BNZeroMemory(&newDim, sizeof(API_Element));
	BNZeroMemory(&memoDim, sizeof(API_ElementMemo));

	newDim.header.type.typeID = API_DimensionID;

	err = ACAPI_Element_GetDefaults(&newDim, &memoDim);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	SetTypeForDimension(newDim);
	double scale = 60;
	err = ACAPI_Drawing_GetDrawingScale(&scale);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	std::pair<double, double> s = { begin.x,begin.y };
	std::pair<double, double> e = { end.x,end.y };
	double lengthWall = calculateDistance(s, e);
	double multiFactor = distance * scale / 60 / lengthWall;
	API_Coord pointPutDim = { locationDim.x + multiFactor * directionWall.x,locationDim.y + multiFactor * directionWall.y };

	newDim.dimension.defStaticDim = false;
	newDim.dimension.nDimElem = 3;
	newDim.dimension.refC = pointPutDim;
	newDim.dimension.direction = { -directionWall.y,directionWall.x };

	memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
	if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	// Set location for dim
	for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
		API_DimElem& dimElem = (*memoDim.dimElems)[i];
		if (i == 0 || i == 2) {
			dimElem.base.base.type.typeID = API_WallID;
			dimElem.base.base.line = false;
			dimElem.base.base.guid = wall.header.guid;
			dimElem.base.base.special = 1;
			dimElem.base.base.inIndex = (i == 0) ? 2 : 3;
		}
		else if(line.line.head.type.typeID == API_LineID&&i==1)
		{
			dimElem.base.base.type.typeID = API_LineID;
			dimElem.base.base.line = false;
			dimElem.base.base.guid = line.header.guid;
			dimElem.base.base.special = false;
			dimElem.base.base.inIndex = 1;
		}
		else if (i == 1) {
			dimElem.base.base.type.typeID = API_ObjectID;
			dimElem.base.base.line = false;
			dimElem.base.base.guid = line.header.guid;
			dimElem.base.base.special = false;
			dimElem.base.base.inIndex = 1;
		}
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
	delete[] arrayElement;
	return NoError;
}

GSErrCode CreateDimensionTotalWallWithLine()
{
	bool isClickPoint = PickPoint("You must click a point to put dimension", &pointPutDimension);
	if (!isClickPoint) return NoError;
	GS::Array<API_Neig> selNeigs;
	API_Element* arrayElement = SelectAllElement(selNeigs);
	API_Element wall = *arrayElement;
	API_Coord begin = wall.wall.begC;
	API_Coord end = wall.wall.endC;
	API_Coord locationDim = pointPutDimension;

	API_Coord* pointBeginEnd = SetDirectionElementToDim(begin, end);
	API_Coord directionWall = { (*(pointBeginEnd + 1)).x - (*pointBeginEnd).x ,(*(pointBeginEnd + 1)).y - (*pointBeginEnd).y };

	API_Element			newDim;
	API_ElementMemo		memoDim;
	GSErrCode			err = NoError;

	BNZeroMemory(&newDim, sizeof(API_Element));
	BNZeroMemory(&memoDim, sizeof(API_ElementMemo));

	newDim.header.type.typeID = API_DimensionID;

	err = ACAPI_Element_GetDefaults(&newDim, &memoDim);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	SetTypeForDimension(newDim);

	newDim.dimension.defStaticDim = false;
	newDim.dimension.nDimElem = 2;
	newDim.dimension.refC = locationDim;
	newDim.dimension.direction = { -directionWall.y,directionWall.x };

	memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
	if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
		API_DimElem& dimElem = (*memoDim.dimElems)[i];

		dimElem.base.base.type.typeID = API_WallID;
		dimElem.base.base.line = false;
		dimElem.base.base.guid = wall.header.guid;
		dimElem.base.base.special = 1;
		dimElem.base.base.inIndex = i + 2;

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
	delete[] arrayElement;
	return NoError;
}


API_Element* SelectAllElement(GS::Array<API_Neig> selNeigs) {
	API_SelectionInfo selectionInfo;
	
	GS::ErrCode err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (selectionInfo.typeID == API_SelEmpty) {
		return NULL;
	}
	std::vector<API_Element> arrayElements;
	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++)
	{
		API_Element element;
		BNClear(element);
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (element.header.type.typeID == API_WallID) {
			arrayElements.insert(arrayElements.begin(), element);
		}
		else if (element.header.type.typeID == API_LineID) {
			arrayElements.insert(arrayElements.end(), element);
		}
		else {
			arrayElements.insert(arrayElements.end(), element);
		}
	};

	const int count = (int)arrayElements.size();
	API_Element* pointArray = new API_Element[count];
	for (int i = 0; i < count; i++) {
		*(pointArray + i) = arrayElements[i];
	}
	return pointArray;
}
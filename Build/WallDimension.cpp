#include "WallDimension.h"
static API_Coord pointPutDimension;
void Dim_Selection(GS::Array<API_Neig>selNeigs, API_Element& element) {
	API_SelectionInfo selectionInfo;
	GS::ErrCode err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (selectionInfo.typeID == API_SelEmpty) {
		return;
	}

	for (const API_Neig& selNeig : selNeigs) {
		BNClear(element);
		element.header.guid = selNeig.guid;
		element.header.type.typeID = API_DimensionID;
		err = ACAPI_Element_Get(&element);
		if (err != NoError) return;
	}
}

void Wall_Selection(GS::Array<API_Neig>selNeigs, API_Element& element) {
	API_SelectionInfo selectionInfo;
	GS::ErrCode err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);

	if (selectionInfo.typeID == API_SelEmpty) {
		return;
	}
	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
		BNClear(element);
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == NoError){
			if(element.header.type.typeID==API_WallID)return;
		}	
	}
}




GSErrCode CreateDimensionTotalWall()
{
	bool isClickPoint = PickPoint("You must click a point to put dimension", &pointPutDimension);
	if (!isClickPoint) return NoError;
	GS::Array<API_Neig>selNeigs;

	//API_Element dimTest;
	//API_ElementMemo memoDimTest;
	//Dim_Selection(selNeigs, dimTest);
	//API_DimensionType typeDim = dimTest.dimension;
	//ACAPI_Element_GetMemo(dimTest.header.guid, &memoDimTest);


	API_Element wall;
	Wall_Selection(selNeigs, wall);

	API_Coord begin = wall.wall.begC;
	API_Coord end = wall.wall.endC;
	API_Coord locationDim =pointPutDimension;

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

	return NoError;
}

void ChangeReferenceWallToCoreCenter(API_Element&);
void RobackReferenceWall(API_Element&, const API_WallReferenceLineLocationID);

int* IndexCoreFace(const API_Element wall) {
	int result[2];
	API_Neig neig;
	BNZeroMemory(&neig, sizeof(API_Neig));
	neig.guid = wall.header.guid;
	neig.neigID = APINeig_WallPl;
	neig.inIndex = 5;
	API_Coord3D coord3D;
	API_Coord3D coord3DDocument;
	bool isAdd5 = false;
	ACAPI_Element_NeigToCoord(&neig, &coord3D, &coord3DDocument);
	if (abs(coord3D.x) < pow(10, 9) && abs(coord3D.y) < pow(10, 9)) {
		result[0] = 5;
		isAdd5 = true;
	}

	API_Neig neig2;
	BNZeroMemory(&neig2, sizeof(API_Neig));
	neig2.guid = wall.header.guid;
	neig2.neigID = APINeig_WallPl;
	neig2.inIndex = 7;
	API_Coord3D coord3D2;
	API_Coord3D coord3DDocument2;
	ACAPI_Element_NeigToCoord(&neig2, &coord3D2, &coord3DDocument2);

	bool isAdd7 = false;
	if (abs(coord3D2.x) < pow(10, 9) && abs(coord3D2.y) < pow(10, 9)) {
		if (isAdd5) {
			result[1] = 7;
			return result;
		}
		else {
			result[0] = 7;
			isAdd7 = true;
		}
	};

	if (isAdd7 == false && isAdd5 == true) {
		API_Neig neig3;
		BNZeroMemory(&neig3, sizeof(API_Neig));
		neig3.guid = wall.header.guid;
		neig3.neigID = APINeig_WallPl;
		neig3.inIndex = 4;
		API_Coord3D coord3D3;
		API_Coord3D coord3DDocument3;
		ACAPI_Element_NeigToCoord(&neig3, &coord3D3, &coord3DDocument3);

		if (abs(coord3D3.x) < pow(10, 9) && abs(coord3D3.y) < pow(10, 9)) {
			result[1] = 4;
			return result;
		};
	}

	if (isAdd7 == true && isAdd5 == false) {
		API_Neig neig4;
		BNZeroMemory(&neig4, sizeof(API_Neig));
		neig4.guid = wall.header.guid;
		neig4.neigID = APINeig_Wall;
		neig4.inIndex = 1;
		API_Coord3D coord3D4;
		API_Coord3D coord3DDocument4;
		ACAPI_Element_NeigToCoord(&neig4, &coord3D4, &coord3DDocument4);

		if (abs(coord3D4.x) < pow(10, 9) && abs(coord3D4.y) < pow(10, 9)) {
			result[1] = 1;
			return result;
		};
	}

	if (isAdd7 == false && isAdd5 == false) {
		result[0] = 1;
		result[1] = 4;
	}
	return result;
}

GSErrCode CreateDimensionCenterWall(double distance)
{
	GS::Array<API_Neig>selNeigs;
	API_Element wall;
	Wall_Selection(selNeigs, wall);
	API_Coord begin = wall.wall.begC;
	API_Coord end = wall.wall.endC;
	API_Coord locationDim = pointPutDimension;

	API_Coord* pointBeginEnd = SetDirectionElementToDim(begin, end);
	API_Coord directionWall = { (*(pointBeginEnd + 1)).x - (*pointBeginEnd).x ,(*(pointBeginEnd + 1)).y - (*pointBeginEnd).y };

	std::pair<double, double> s = { begin.x,begin.y };
	std::pair<double, double> e = { end.x,end.y };
	double lengthWall = calculateDistance(s, e);

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

	double scale = 60;
	err = ACAPI_Drawing_GetDrawingScale(&scale);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	SetTypeForDimension(newDim);

	newDim.dimension.defStaticDim = true;
	newDim.dimension.nDimElem = 3;

	API_Vector directionDim = { -directionWall.y,directionWall.x };

	double multiFactor = distance * scale / 60 / lengthWall;
	API_Coord pointPutDim = { locationDim.x + multiFactor * directionWall.x,locationDim.y + multiFactor * directionWall.y };

	newDim.dimension.refC = pointPutDim;
	newDim.dimension.direction = directionDim;

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
		else
		{
			int* indexCoreFace = IndexCoreFace(wall);
			int indexs[2] = { *indexCoreFace,*(indexCoreFace + 1) };
			std::list<API_Coord3D> listPoint = {};
			for (int index : indexs)
			{
				API_Neig neig;
				BNZeroMemory(&neig, sizeof(API_Neig));
				neig.guid = wall.header.guid;
				neig.neigID = index == 1 ? APINeig_Wall : APINeig_WallPl;
				neig.inIndex = index;
				API_Coord3D coord3D;
				API_Coord3D coord3DDocument;
				ACAPI_Element_NeigToCoord(&neig, &coord3D, &coord3DDocument);
				listPoint.push_back(coord3D);
			}
			API_Coord coreMiddel = { (listPoint.front().x + listPoint.back().x) / 2 ,(listPoint.front().y + listPoint.back().y) / 2 };
			dimElem.base.loc = coreMiddel;
			dimElem.base.base.special = false;
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
	return NoError;
}

GSErrCode CreateDimensionLayerWall(double distance)
{
	GS::Array<API_Neig>selNeigs;
	API_Element wall;

	Wall_Selection(selNeigs, wall);

	API_Coord begin = wall.wall.begC;
	API_Coord end = wall.wall.endC;
	API_Coord locationDim = pointPutDimension;
	API_Coord* pointBeginEnd = SetDirectionElementToDim(begin, end);
	API_Coord directionWall = { (*(pointBeginEnd + 1)).x - (*pointBeginEnd).x ,(*(pointBeginEnd + 1)).y - (*pointBeginEnd).y };
	std::pair<double, double> s = { begin.x,begin.y };
	std::pair<double, double> e = { end.x,end.y };
	double lengthWall = calculateDistance(s, e);

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

	double scale = 60;
	err = ACAPI_Drawing_GetDrawingScale(&scale);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	SetTypeForDimension(newDim);

	newDim.dimension.defStaticDim = false;
	newDim.dimension.nDimElem = 4;

	double multiFactor = 2 * distance * scale / 60 / lengthWall;
	API_Vector directionDim = { -directionWall.y,directionWall.x };

	API_Coord pointPutDim = { locationDim.x + multiFactor * directionWall.x,locationDim.y + multiFactor * directionWall.y };

	newDim.dimension.refC = pointPutDim;
	newDim.dimension.direction = directionDim;

	memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
	if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}
	int inIndexArray[] = { 3,8,6,2 };
	for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
		API_DimElem& dimElem = (*memoDim.dimElems)[i];

		dimElem.base.base.type.typeID = API_WallID;
		dimElem.base.base.line = false;
		dimElem.base.base.guid = wall.header.guid;
		dimElem.base.base.special = 1;
		dimElem.base.base.inIndex = inIndexArray[i];

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
	return NoError;
}


void ChangeReferenceWallToCoreCenter(API_Element& element) {
	API_Element elementMask;
	BNZeroMemory(&elementMask, sizeof(API_Element));
	ACAPI_ELEMENT_MASK_SET(elementMask, API_WallType, referenceLineLocation);
	element.wall.referenceLineLocation = APIWallRefLine_CoreCenter;
	ACAPI_Element_Change(&element, &elementMask, nullptr, 0UL, true);
}

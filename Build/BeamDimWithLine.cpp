#include "BeamDimWithLine.h"
static API_Coord pointPutDimension;

std::vector<API_Element> SelectionBeamWithGrid(GS::Array<API_Neig> selNeigs);
GSErrCode CreateTotalDimBeam() {
	bool isClickPoint = PickPoint("You must click a point to put dimension", &pointPutDimension);
	if (!isClickPoint) return NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> arrayElements = SelectionBeamWithGrid(selNeigs);
	API_Element beam = arrayElements[0];
	API_Coord begin = beam.beam.begC;
	API_Coord end = beam.beam.endC;
	API_Coord locationDim = pointPutDimension;
	
	API_Coord* pointBeginEnd = SetDirectionElementToDim(begin, end);
	API_Coord directionBeam= { (*(pointBeginEnd + 1)).x - (*pointBeginEnd).x ,(*(pointBeginEnd + 1)).y - (*pointBeginEnd).y };

	API_Element newDim;
	API_ElementMemo memoDim;
	GSErrCode err = NoError;

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
	newDim.dimension.direction= { -directionBeam.y,directionBeam.x };

	memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
	if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
		API_DimElem& dimElem = (*memoDim.dimElems)[i];

		dimElem.base.base.type.typeID = API_BeamID;
		dimElem.base.base.line = false;
		dimElem.base.base.guid = beam.header.guid;
		dimElem.base.base.special = 0;
		dimElem.base.base.inIndex = i==0?2:4;

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

GSErrCode CreateDimBeamToLine(double distance) {
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> arrayElements = SelectionBeamWithGrid(selNeigs);
	if (arrayElements.size() < 2) {
		return NoError;
	}
	API_Element beam = arrayElements[0];
	API_Element line= arrayElements[1];
	API_Coord begin = beam.beam.begC;
	API_Coord end = beam.beam.endC;
	API_Coord locationDim = pointPutDimension;

	API_Coord* pointBeginEnd = SetDirectionElementToDim(begin, end);
	API_Coord directionBeam = { (*(pointBeginEnd + 1)).x - (*pointBeginEnd).x ,(*(pointBeginEnd + 1)).y - (*pointBeginEnd).y };

	API_Element newDim;
	API_ElementMemo memoDim;
	GSErrCode err = NoError;

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

	std::pair<double, double> s = { begin.x,begin.y };
	std::pair<double, double> e = { end.x,end.y };
	double lengthBeam = calculateDistance(s, e);
	double multiFactor = distance * scale / 60 / lengthBeam;
	API_Coord pointPutDim = { locationDim.x + multiFactor * directionBeam.x,locationDim.y + multiFactor * directionBeam.y };

	SetTypeForDimension(newDim);
	newDim.dimension.defStaticDim = false;
	newDim.dimension.nDimElem = 3;
	newDim.dimension.refC = pointPutDim;
	newDim.dimension.direction = { -directionBeam.y,directionBeam.x };

	memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
	if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
		API_DimElem& dimElem = (*memoDim.dimElems)[i];


		if (i == 0 || i == 2) {
			dimElem.base.base.type.typeID = API_BeamID;
			dimElem.base.base.line = false;
			dimElem.base.base.guid = beam.header.guid;
			dimElem.base.base.special = 0;
			dimElem.base.base.inIndex = i == 0 ? 2 : 4;
		}
		else if (line.line.head.type.typeID == API_LineID && i == 1)
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
	return NoError;
}


GS::Array<API_Coord> GetAllHotspotOfBeamIncludeSampe(API_Element beam, API_Coord begin,API_Coord end);
GSErrCode CreateDimLayerBeam(double distance) {
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> arrayElements = SelectionBeamWithGrid(selNeigs);
	API_Element beam = arrayElements[0];
	API_Coord begin = beam.beam.begC;
	API_Coord end = beam.beam.endC;
	API_Coord locationDim = pointPutDimension;

	API_Coord* pointBeginEnd = SetDirectionElementToDim(begin, end);
	API_Coord directionBeam = { (*(pointBeginEnd + 1)).x - (*pointBeginEnd).x ,(*(pointBeginEnd + 1)).y - (*pointBeginEnd).y };

	API_Element newDim;
	API_ElementMemo memoDim;
	GSErrCode err = NoError;

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
	

	double scale = 60;
	err = ACAPI_Drawing_GetDrawingScale(&scale);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}

	std::pair<double, double> s = { begin.x,begin.y };
	std::pair<double, double> e = { end.x,end.y };
	double lengthBeam = calculateDistance(s, e);
	double multiFactor = (arrayElements.size()<2? 1: 2) * distance * scale / 60 / lengthBeam;
	API_Vector directionDim = { -directionBeam.y,directionBeam.x };

	API_Coord pointPutDim = { locationDim.x + multiFactor * directionBeam.x,locationDim.y + multiFactor * directionBeam.y };

	newDim.dimension.refC = pointPutDim;
	newDim.dimension.direction = directionDim;
	GS::Array<API_Coord> hotspotPoints = GetAllHotspotOfBeamIncludeSampe(beam,begin,end);
	newDim.dimension.nDimElem = hotspotPoints.GetSize();

	memoDim.dimElems = reinterpret_cast<API_DimElem**> (BMhAllClear(newDim.dimension.nDimElem * sizeof(API_DimElem)));
	if (memoDim.dimElems == nullptr || *memoDim.dimElems == nullptr) {
		ACAPI_DisposeElemMemoHdls(&memoDim);
		return err;
	}
	
	
	for (Int32 i = 0; i < newDim.dimension.nDimElem; ++i) {
		API_DimElem& dimElem = (*memoDim.dimElems)[i];

		dimElem.base.loc = hotspotPoints[i];
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
	return NoError;
}

GS::Array<API_Coord> GetAllHotspotOfBeamIncludeSampe(API_Element beam, API_Coord begin,API_Coord end) {
	GS::Array<API_Coord> result;
	GS::Array <API_ElementHotspot> hotspotArray;
	GS::ErrCode err = ACAPI_Element_GetHotspots(beam.header.guid, &hotspotArray);
	for (Int32 i = 0; i < hotspotArray.GetSize(); i++) {
		API_ElementHotspot hotspot = hotspotArray.Get(i);
		bool isExist = false;
		API_Coord hotspotPoint = { hotspot.second.x,hotspot.second.y };
		for (int j = 0; j < result.GetSize(); j++) {
			API_Coord item = result.Get(j);
			if (std::abs(item.x - hotspotPoint.x) < 0.0001 && std::abs(item.y - hotspotPoint.y) < 0.0001) {
				isExist = true;
				break;
			}
		}
		if (std::abs(begin.x - hotspotPoint.x) < 0.0001 && std::abs(begin.y - hotspotPoint.y) < 0.0001) {
			isExist = true;
		}
		if (std::abs(end.x - hotspotPoint.x) < 0.0001 && std::abs(end.y - hotspotPoint.y) < 0.0001) {
			isExist = true;
		}
		if (!isExist) {
			result.Push(hotspotPoint);
		}
	}
	return result;
}





/// <summary>
/// selection grid and beam for dimensions
/// </summary>
/// <param name="selNeigs"></param>
/// <returns></returns>
std::vector<API_Element> SelectionBeamWithGrid(GS::Array<API_Neig> selNeigs) {
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
		if (element.header.type.typeID == API_BeamID) {
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
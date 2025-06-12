#include "BeamBottomTopBinding.h"
#include "basicgeometry.h"

API_Vector3D	VectProdEle(const API_Vector3D& v1, const API_Vector3D& v2)

{
	API_Vector3D v;

	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;

	return v;
}

API_Coord		NormalizeEle2D(const API_Coord& v)

{
	API_Coord ret = v;
	double len = v.x * v.x + v.y * v.y;
	if (len > 1e-10) {
		double lambda = 1.0 / sqrt(len);
		ret.x = ret.x * lambda;
		ret.y = ret.y * lambda;
	}
	return ret;
}

API_Coord3D		NormalizeEle3D(const API_Coord3D& v)

{
	API_Coord3D ret = v;
	double len = v.x * v.x + v.y * v.y + v.z * v.z;
	if (len > 1e-10) {
		double lambda = 1.0 / sqrt(len);
		ret.x = ret.x * lambda;
		ret.y = ret.y * lambda;
		ret.z = ret.z * lambda;
	}
	return ret;
}
double DotProductEle(const Geometry::Vector3D& v1, const Geometry::Vector3D& v2) 
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Geometry::Point3D IntsecitonPointByVector(Geometry::Plane plane, Geometry::Point3D point, Geometry::Vector3D vectorNormalize) 
{
	double dot1 = DotProductEle(plane.GetZAxis(), plane.GetBasePoint());
	double dot2 = DotProductEle(plane.GetZAxis(), point);
	
	double dot3 = DotProductEle(plane.GetZAxis(), vectorNormalize);
	double lineParameter = (dot1 - dot2) / dot3;
	return Geometry::Point3D{ point.x + lineParameter * vectorNormalize.x,
		point.y + lineParameter * vectorNormalize.y,
		point.z + lineParameter * vectorNormalize.z };
	
}

GS::Array<API_Element> SelectBeamElevation(GS::Array<API_Neig> selNeigs) {
	GS::Array<API_Element> result = GS::Array<API_Element>();
	API_SelectionInfo* selectionInfo = new API_SelectionInfo();
	GS::ErrCode err = ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
	if (selectionInfo->typeID == API_SelEmpty) {
		delete selectionInfo;
		return result;
	}
	for (int i = 0; i < selectionInfo->sel_nElem; i++) {
		API_Element element;
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			if (element.header.type.typeID == API_BeamID) {
				result.Push(element);
			}
		}
	}
	delete selectionInfo;
	return result;
}

bool ClickAnElemTopAndBottomBeam(const char* prompt,
	API_ElemTypeID		needTypeID,
	API_Neig* neig /*= nullptr*/,
	API_ElemTypeID* typeID /*= nullptr*/,
	API_Guid* guid /*= nullptr*/,
	API_Coord3D* c /*= nullptr*/,
	bool				ignorePartialSelection /*= true*/)
{
	API_GetPointType pointInfo = {};
	API_ElemTypeID clickedID;
	GSErrCode			err;
	CHTruncate(prompt, pointInfo.prompt, sizeof(pointInfo.prompt));
	pointInfo.changeFilter = false;
	pointInfo.changePlane = false;
	err = ACAPI_UserInput_GetPoint(&pointInfo, nullptr);
	if (err != NoError) return false;
	if (pointInfo.neig.neigID == APINeig_None) {
		API_Elem_Head elemHead;
		BNZeroMemory(&elemHead, sizeof(API_Elem_Head));
		API_ElemSearchPars	pars;
		BNZeroMemory(&pars, sizeof(API_ElemSearchPars));
		pars.type.typeID = needTypeID;
		pars.loc.x = pointInfo.pos.x;
		pars.loc.y = pointInfo.pos.y;
		pars.z = pointInfo.pos.z;
		err = ACAPI_Element_SearchElementByCoord(&pars, &elemHead.guid);
	}
	if (pointInfo.neig.elemPartType != APINeigElemPart_None && ignorePartialSelection) {
		pointInfo.neig.elemPartType = APINeigElemPart_None;
		pointInfo.neig.elemPartIndex = 0;
	}

	API_Element element;
	BNClear(element);
	element.header.guid = pointInfo.neig.guid;
	err = ACAPI_Element_Get(&element);
	if (err != NoError) return false;
	clickedID = element.header.type.typeID;
	if (clickedID == API_ZombieElemID)
		return false;

	if (neig != nullptr)
		*neig = pointInfo.neig;
	if (typeID != nullptr)
		*typeID = clickedID;
	if (guid != nullptr)
		*guid = pointInfo.neig.guid;
	if (c != nullptr)
		*c = pointInfo.pos;

	bool good = (needTypeID == API_ZombieElemID || needTypeID == clickedID);
	if (!good && clickedID == API_SectElemID) {
		API_Element element;
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = pointInfo.neig.guid;
		if (ACAPI_Element_Get(&element) == NoError)
			good = (needTypeID == element.sectElem.parentType);
	}

	return good;
}


Geometry::Plane GetPlaneToChangeElevation(const API_Element& beam, API_Coord3D* pointClick)
{
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	GS::ErrCode err = NoError;
	Geometry::Plane result;
	std::vector<Geometry::Plane> bottomPlanes = std::vector<Geometry::Plane>();
	std::vector<Geometry::Plane> topPlanes = std::vector<Geometry::Plane>();
	if (beam.header.type.typeID == API_BeamID)
	{
		ACAPI_Element_GetMemo(beam.header.guid, &memo);
		GS::Array<API_Elem_Head> arrayHeads = GS::Array<API_Elem_Head>();
		GSSize nSegments = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.beamSegments)) / sizeof(API_BeamSegmentType);
		Int32 idx;
		for (idx = 0; idx < nSegments; ++idx) 
		{
			arrayHeads.Push(memo.beamSegments[idx].head);
		}

		for (API_Elem_Head elementHead : arrayHeads)
		{
			API_ElemInfo3D elementInfo3d;
			err = ACAPI_ModelAccess_Get3DInfo(elementHead, &elementInfo3d);
			if (err != NoError) return result;
			Int32 startIndexBody = elementInfo3d.fbody;
			Int32 endIndexBody = elementInfo3d.lbody;

			for (Int32 i = startIndexBody; i <= endIndexBody; i++)
			{
				API_Component3D component = {};
				component.header.typeID = API_BodyID;
				component.header.index = i;
				err = ACAPI_ModelAccess_GetComponent(&component);
				if (err != NoError) continue;
				Int32 nPgon = component.body.nPgon;
				API_Tranmat tm = component.body.tranmat;
				for (Int32 j = 1; j <= nPgon; j++) {
					component.header.typeID = API_PgonID;
					component.header.index = j;
					err = ACAPI_ModelAccess_GetComponent(&component);
					if (err != NoError) continue;
					API_Coord3D normVect;
					Int32 ivect = component.pgon.ivect;

					Int32 startIndexEdge = component.pgon.fpedg;
					Int32 endIndexEdge = component.pgon.lpedg;

					component.header.typeID = API_VectID;
					component.header.index = std::abs(ivect);
					err = ACAPI_ModelAccess_GetComponent(&component);
					if (err != NoError) continue;
					if (ivect < 0) {
						component.vect.x = -component.vect.x;
						component.vect.y = -component.vect.y;
						component.vect.z = -component.vect.z;
					}
					normVect = { component.vect.x,component.vect.y, component.vect.z };
					API_Coord3D	trVect;	// world coordinates
					trVect.x = tm.tmx[0] * component.vect.x + tm.tmx[1] * component.vect.y + tm.tmx[2] * component.vect.z;
					trVect.y = tm.tmx[4] * component.vect.x + tm.tmx[5] * component.vect.y + tm.tmx[6] * component.vect.z;
					trVect.z = tm.tmx[8] * component.vect.x + tm.tmx[9] * component.vect.y + tm.tmx[10] * component.vect.z;
					normVect = trVect;

					API_Coord3D pointOnPlane;
					for (Int32 kk = startIndexEdge; kk <= endIndexEdge; ++kk) {
						component.header.typeID = API_PedgID;
						component.header.index = kk;
						err = ACAPI_ModelAccess_GetComponent(&component);
						if (err != NoError) continue;
						component.header.typeID = API_EdgeID;
						component.header.index = std::abs(component.pedg.pedg);
						Int32 indexEdge = std::abs(component.pedg.pedg);
						Int32 indexEdgeWithSign = component.pedg.pedg;
						bool isPositive = component.pedg.pedg > 0;
						if (err != NoError) continue;
						if (component.pedg.pedg != 0)
						{
							err = ACAPI_ModelAccess_GetComponent(&component);
							if (err == NoError) {
								component.header.typeID = API_VertID;
								component.header.index = isPositive ? component.edge.vert1 : component.edge.vert2;
								err = ACAPI_ModelAccess_GetComponent(&component);
								if (err == NoError)
								{
									API_Coord3D pointComponentWorld;
									pointComponentWorld.x = tm.tmx[0] * component.vert.x + tm.tmx[1] * component.vert.y + tm.tmx[2] * component.vert.z;
									pointComponentWorld.y = tm.tmx[4] * component.vert.x + tm.tmx[5] * component.vert.y + tm.tmx[6] * component.vert.z;
									pointComponentWorld.z = tm.tmx[8] * component.vert.x + tm.tmx[9] * component.vert.y + tm.tmx[10] * component.vert.z;
									pointOnPlane = pointComponentWorld;
									break;
								}
							}
						}
					}

					Geometry::Point3D pointPl = Geometry::Point3D{ pointOnPlane.x,pointOnPlane.y,pointOnPlane.z };
					Geometry::Vector3D normalGeo = Geometry::Vector3D{ normVect.x,normVect.y,normVect.z };
					Geometry::Plane plane = Geometry::Plane(pointPl, normalGeo);
					Geometry::Point3D pointClickGeo = { pointClick->x,pointClick->y,pointClick->z };
					bool isPointOnPlane = plane.IsPlaneCoord3D(pointClickGeo);
					if (isPointOnPlane) {
						result = plane;
						return result;
					}
				}

			}
		}

		
	}

	return result;
}



std::wstring ToWStringElevation(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

GS::ErrCode ChangeElevationBeam(double offset)
{
	GS::ErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	GS::Array<API_Element> beamMains = SelectBeamElevation(selNeigs);

	API_Neig			theNeigBeam1;
	API_Coord3D* pointTarget1 = new API_Coord3D();
	bool isHas = ClickAnElemTopAndBottomBeam("Click Beam Element", API_ZombieElemID, &theNeigBeam1, nullptr, nullptr, pointTarget1, false);
	if (!isHas) return NoError;
	API_Element beam1 = {};
	beam1.header.guid = theNeigBeam1.guid;
	err = ACAPI_Element_Get(&beam1);
	if (err != NoError) return NoError;
	API_Coord directionBeam1 = NormalizeEle2D(API_Coord{ beam1.beam.endC.x - beam1.beam.begC.x,beam1.beam.endC.y - beam1.beam.begC.y });
	API_Coord3D normalBeam1 = NormalizeEle3D( VectProdEle(API_Vector3D{ 0,0,1 }, API_Vector3D{ directionBeam1.x,directionBeam1.y,0 }));
	Geometry::Plane plane1 = Geometry::Plane(Geometry::Point3D{ beam1.beam.begC.x,beam1.beam.begC.y,0 },
		Geometry::Vector3D {normalBeam1.x,normalBeam1.y,normalBeam1.z });

	API_Neig			theNeigBeam2;
	API_Coord3D* pointTarget2 = new API_Coord3D();
	bool isHas2 = ClickAnElemTopAndBottomBeam("Click Beam Element", API_ZombieElemID, &theNeigBeam2, nullptr, nullptr, pointTarget2, false);
	if (!isHas2) return NoError;
	API_Element beam2 = {};
	beam2.header.guid = theNeigBeam2.guid;
	err = ACAPI_Element_Get(&beam2);
	if (err != NoError) return NoError;

	API_Coord directionBeam2 = NormalizeEle2D(API_Coord{ beam2.beam.endC.x - beam2.beam.begC.x,beam2.beam.endC.y - beam2.beam.begC.y });
	API_Coord3D normalBeam2 = NormalizeEle3D(VectProdEle(API_Vector3D{ 0,0,1 }, API_Vector3D{ directionBeam2.x,directionBeam2.y,0 }));
	Geometry::Plane plane2 = Geometry::Plane(Geometry::Point3D{ beam2.beam.begC.x,beam2.beam.begC.y,0 },
		Geometry::Vector3D{ normalBeam2.x,normalBeam2.y,normalBeam2.z });

	API_StoryInfo storyInfo;
	BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
	ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
	
	for (API_Element beam : beamMains) 
	{
		GS::Array<API_PropertyDefinition> propertyDefinitions;
		API_Element markBeam;
		ACAPI_ELEMENT_MASK_SET(markBeam, API_BeamType, level);
		ACAPI_ELEMENT_MASK_SET(markBeam, API_BeamType, isSlanted);
		ACAPI_ELEMENT_MASK_SET(markBeam, API_BeamType, slantAngle);
		beam.beam.isSlanted = true;

		API_StoryType storyBeam;
		API_StoryType* pointerIteratorBeam = &(*storyInfo.data)[0];
		for (int id = storyInfo.firstStory; id <= storyInfo.lastStory; id++) {
			if (pointerIteratorBeam != nullptr) {
				if (id == beam.header.floorInd) {
					storyBeam = *pointerIteratorBeam;
					break;
				}
			}
			++pointerIteratorBeam;
		}

		API_Coord sb = beam.beam.begC;
		API_Coord eb = beam.beam.endC;
		API_Coord directionBeam{ eb.x - sb.x,eb.y - sb.y };
		directionBeam = NormalizeEle2D(directionBeam);
		Geometry::Vector3D directionBeam3D = Geometry::Vector3D{ directionBeam.x,directionBeam.y, 0 };
		Geometry::Point3D sPlane = IntsecitonPointByVector(plane1,Geometry::Point3D{ sb.x,sb.y,0 }, directionBeam3D);
		Geometry::Point3D ePlane = IntsecitonPointByVector(plane1,Geometry::Point3D{ eb.x,eb.y,0 }, directionBeam3D);
		double ds = Geometry::Dist3D(Geometry::Point3D{ sb.x,sb.y,0 }, sPlane);
		double de = Geometry::Dist3D(Geometry::Point3D{ eb.x,eb.y,0 }, ePlane);
		double zStart = 0;
		double zEnd = 0;
		
		double anpha = 0;
		Geometry::Point3D pointStartAtCenter;
		if (ds < de) 
		{
			zStart = pointTarget1->z;
			zEnd = pointTarget2->z;
			Geometry::Point3D ps = IntsecitonPointByVector(plane1, Geometry::Point3D{ sb.x,sb.y,0 }, directionBeam3D);
			pointStartAtCenter = ps;
			Geometry::Point3D pe = IntsecitonPointByVector(plane2, Geometry::Point3D{ eb.x,eb.y,0 }, directionBeam3D);
			double distance = Geometry::Dist3D(ps, pe);
			double deltalH = std::abs(zStart - zEnd) - 2 * offset;
			anpha = std::atan(deltalH / distance);
		}
		else 
		{
			zStart = pointTarget2->z;
			zEnd = pointTarget1->z;
			Geometry::Point3D ps = IntsecitonPointByVector(plane2, Geometry::Point3D{ sb.x,sb.y,0 }, directionBeam3D);
			pointStartAtCenter = ps;
			Geometry::Point3D pe = IntsecitonPointByVector(plane1, Geometry::Point3D{ eb.x,eb.y,0 }, directionBeam3D);
			double distance = Geometry::Dist3D(ps, pe);
			double deltalH = std::abs(zStart - zEnd) - 2 * offset;
			anpha = std::atan(deltalH / distance);

		}

		/// move ve tam dam cho nay
		ACAPI_Element_GetPropertyDefinitions(beam.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);
		API_Property property;
		for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
		{
			auto name = ToWStringElevation(propertyDefinition.name);
			if (name == L"Height")
			{
				ACAPI_Element_GetPropertyValue(beam.header.guid, propertyDefinition.guid, property);
				break;
			}
		}
		double heightBeam = property.value.singleVariant.variant.doubleValue;
		double offsetFromlLevel = 0;
		double deltaToCenter = Geometry::Dist3D(Geometry::Point3D{ sb.x,sb.y,0 }, pointStartAtCenter);
		double deltaMiddle = std::sqrt((heightBeam / 2) * (heightBeam / 2) + (tan(anpha) * heightBeam / 2) * (tan(anpha) * heightBeam / 2));
		if (zStart < zEnd) {
			offsetFromlLevel = zStart - storyBeam.level + offset + std::tan(anpha) * deltaToCenter + deltaMiddle;
		}
		else {
			offsetFromlLevel = zStart - storyBeam.level - offset - std::tan(anpha) * deltaToCenter + deltaMiddle;
		}
		
		if (zStart < zEnd) anpha = std::abs(anpha);
		else anpha = -std::abs(anpha);

		beam.beam.level = offsetFromlLevel;
		beam.beam.slantAngle = anpha;
		ACAPI_Element_Change(&beam, &markBeam, nullptr, 0UL, true);

	}



	return NoError;
}

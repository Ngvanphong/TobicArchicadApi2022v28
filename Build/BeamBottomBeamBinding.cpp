#include "BeamBottomBeamBinding.h"
#include "basicgeometry.h"

GS::Array<API_Element> SelectBeamBottom(GS::Array<API_Neig> selNeigs) {
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
Geometry::Vector3D	NormalizeBot3D(const Geometry::Vector3D& v)

{
	Geometry::Vector3D ret = v;
	double len = v.x * v.x + v.y * v.y + v.z * v.z;
	if (len > 1e-10) {
		double lambda = 1.0 / sqrt(len);
		ret.x = ret.x * lambda;
		ret.y = ret.y * lambda;
		ret.z = ret.z * lambda;
	}
	return ret;
}

double DotProductBot(const Geometry::Vector3D& v1, const Geometry::Vector3D& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Geometry::Vector3D	VectProdBot(const Geometry::Vector3D& v1, const Geometry::Vector3D& v2)
{
	Geometry::Vector3D v;

	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;

	return v;
}

bool ClickAnElemTopBottomBeam(const char* prompt,
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
	err = ACAPI_UserInput_GetPoint(&pointInfo);
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


GS::Array<Geometry::Plane> GetBottomCenterPlaneOfBeam(const API_Element& beam)
{
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	GS::ErrCode err = NoError;
	GS::Array<Geometry::Plane> result= GS::Array<Geometry::Plane>();
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

			Geometry::Plane bottomPlane;
			Geometry::Point3D pointMin{0,0,1000000};

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

					GS::Array<Geometry::Point3D> arrayCoordPoly = GS::Array<Geometry::Point3D>();
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
									arrayCoordPoly.Push(Geometry::Point3D{pointComponentWorld.x,pointComponentWorld.y,pointComponentWorld.z});
								}
							}
						}
					}

					bool isPolyUse = true;
					for (int g = 1; g < arrayCoordPoly.GetSize(); g++) {

						if (isPolyUse == false) break;
						for (int v = 1; v < arrayCoordPoly.GetSize(); v++) 
						{
							double d = Geometry::Dist3D(arrayCoordPoly[v], arrayCoordPoly[g]);
							if (d < 0.030 && d > 0.00001)
							{
								isPolyUse = false; break;
							}
						}
						
					}
					if (isPolyUse == false) continue;
					Geometry::Point3D pointMinOfItem{ 0,0,1000000 };
					for (auto item : arrayCoordPoly) 
					{
						if ( item.z < pointMinOfItem.z) {
							pointMinOfItem = item;
						}
					}
					if (pointMinOfItem.z < pointMin.z) {
						pointMin = pointMinOfItem;
						Geometry::Vector3D normalGeo = Geometry::Vector3D{ normVect.x,normVect.y,normVect.z };
						Geometry::Plane plane = Geometry::Plane(pointMin, normalGeo);
						bottomPlane = plane;
					}
				}

			}
			result.Push(bottomPlane);
			Geometry::Vector3D directionBeam{ beam.beam.endC.x - beam.beam.begC.x, beam.beam.endC.y - beam.beam.endC.y, 0 };
			Geometry::Vector3D normalBeamVector = VectProdBot(directionBeam, Geometry::Vector3D{ 0,0,1 });
			Geometry::Plane centerPlane = Geometry::Plane(Geometry::Point3D{ beam.beam.begC.x, beam.beam.begC.y,0 }, normalBeamVector);
			result.Push(centerPlane);
			return result;
		}
	}

	return result;
}



Geometry::Point3D IntsecitonPointBotByVector(Geometry::Plane plane, Geometry::Point3D point, Geometry::Vector3D vectorNormalize)
{
	double dot1 = DotProductBot(plane.GetZAxis(), plane.GetBasePoint());
	double dot2 = DotProductBot(plane.GetZAxis(), point);

	double dot3 = DotProductBot(plane.GetZAxis(), vectorNormalize);
	double lineParameter = (dot1 - dot2) / dot3;
	return Geometry::Point3D{ point.x + lineParameter * vectorNormalize.x,
		point.y + lineParameter * vectorNormalize.y,
		point.z + lineParameter * vectorNormalize.z };

}

std::wstring ToWStringBot(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

GS::ErrCode AlignBeamToBottom(void)
{
	GS::ErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	GS::Array<API_Element> beamMains = SelectBeamBottom(selNeigs);
	API_Neig			theNeigBeam1;
	API_Coord3D* pointTarget1 = new API_Coord3D();
	bool isHas = ClickAnElemTopBottomBeam("Click Beam Element", API_ZombieElemID, &theNeigBeam1, nullptr, nullptr, pointTarget1, false);
	if (!isHas) return NoError;
	API_Element beam1 = {};
	beam1.header.guid = theNeigBeam1.guid;
	err = ACAPI_Element_Get(&beam1);
	if (err != NoError) return NoError;

	API_Neig			theNeigBeam2;
	API_Coord3D* pointTarget2 = new API_Coord3D();
	bool isHas2 = ClickAnElemTopBottomBeam("Click Beam Element", API_ZombieElemID, &theNeigBeam2, nullptr, nullptr, pointTarget2, false);
	if (!isHas2) return NoError;
	API_Element beam2 = {};
	beam2.header.guid = theNeigBeam2.guid;
	err = ACAPI_Element_Get(&beam2);
	if (err != NoError) return NoError;

	GS::Array<Geometry::Plane> planeBeam1 = GetBottomCenterPlaneOfBeam(beam1);
	GS::Array<Geometry::Plane> planeBeam2 = GetBottomCenterPlaneOfBeam(beam2);

	API_StoryInfo storyInfo;
	BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
	ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
	for (API_Element beam : beamMains) 
	{
		Geometry::Vector3D directBeam{ beam.beam.endC.x - beam.beam.begC.x,beam.beam.endC.y - beam.beam.begC.y,0 };
		directBeam = NormalizeBot3D(directBeam);

		Geometry::Point3D p11 = IntsecitonPointBotByVector(planeBeam1[1], Geometry::Point3D{ beam.beam.begC.x, beam.beam.begC.y,0 }, directBeam);
		Geometry::Point3D p21 = IntsecitonPointBotByVector(planeBeam1[0], p11, Geometry::Vector3D{ 0,0,1 });

		Geometry::Point3D p12 = IntsecitonPointBotByVector(planeBeam2[1], Geometry::Point3D{ beam.beam.begC.x, beam.beam.begC.y,0 }, directBeam);
		Geometry::Point3D p22 = IntsecitonPointBotByVector(planeBeam2[0], p12, Geometry::Vector3D{ 0,0,1 });

		double zBottom = std::min(p21.z, p22.z);

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

		/// move ve tam dam cho nay
		GS::Array<API_PropertyDefinition> propertyDefinitions;
		ACAPI_Element_GetPropertyDefinitions(beam.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);
		API_Property property;
		for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
		{
			auto name = ToWStringBot(propertyDefinition.name);
			if (name == L"Height")
			{
				ACAPI_Element_GetPropertyValue(beam.header.guid, propertyDefinition.guid, property);
				break;
			}
		}
		double heightBeam = property.value.singleVariant.variant.doubleValue;
		double offset = zBottom - storyBeam.level + heightBeam;
		API_Element markBeam;
		ACAPI_ELEMENT_MASK_SET(markBeam, API_BeamType, level);
		beam.beam.level = offset;
		ACAPI_Element_Change(&beam, &markBeam, nullptr, 0UL, true);

	}


	return NoError;
}

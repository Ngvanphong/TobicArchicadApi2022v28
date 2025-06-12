#include "BeamToRoofBinding.h"
#include "basicgeometry.h"

std::vector<API_Element> GetBeamsForAlignRoof(GS::Array<API_Neig> selNeigs)
{
	std::vector<API_Element> result = std::vector<API_Element>();
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
				result.push_back(element);
			}
		}
	}
	return result;

}

std::wstring ToWStringRoof(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

API_Coord3D		NormalizeRoof(const API_Coord3D& v)

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

API_Coord		NormalizeRoof2D(const API_Coord& v)

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

API_Vector3D	VectProdRoof(const API_Vector3D& v1, const API_Vector3D& v2)

{
	API_Vector3D v;

	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;

	return v;
}

double DotProductRoof(const API_Coord3D& a, const API_Coord3D& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}



Geometry::Plane GetPlaneTargetOfBeam(const API_Element& roof, API_Coord3D* pointClick) 
{
	GS::ErrCode err = NoError;
	Geometry::Plane result;
	std::vector<Geometry::Plane> bottomPlanes = std::vector<Geometry::Plane>();
	std::vector<Geometry::Plane> topPlanes = std::vector<Geometry::Plane>();
	if (roof.header.type.typeID == API_RoofID) 
	{
		API_Elem_Head elementHead;
		BNZeroMemory(&elementHead, sizeof(API_Elem_Head));
		elementHead.guid = roof.header.guid;
		err = ACAPI_Element_GetHeader(&elementHead);
		if (err != NoError) return result;
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
							component.header.index= isPositive ? component.edge.vert1 : component.edge.vert2;
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

				Geometry::Point3D pointPl = Geometry::Point3D { pointOnPlane.x,pointOnPlane.y,pointOnPlane.z };
				Geometry::Vector3D normalGeo = Geometry::Vector3D{ normVect.x,normVect.y,normVect.z };
				Geometry::Plane plane = Geometry::Plane(pointPl,normalGeo);
				Geometry::Point3D pointClickGeo = { pointClick->x,pointClick->y,pointClick->z };
				bool isPointOnPlane = plane.IsPlaneCoord3D(pointClickGeo);
				if (isPointOnPlane) {
					result = plane;
					return result;
				}
			}

		}
	}

	return result;
}

bool ClickAnElemRoof(const char* prompt,
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

GS::ErrCode AlignBeamToRoof(void)
{
	GS::ErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> allBeams = GetBeamsForAlignRoof(selNeigs);
	API_Neig			theNeigRoof;
	API_Coord3D* pointTarget = new API_Coord3D();
	bool isHas = ClickAnElemRoof("Click Roof Element", API_ZombieElemID, &theNeigRoof, nullptr, nullptr, pointTarget, false);
	if (!isHas) return NoError;
	API_Element roof = {};
	roof.header.guid = theNeigRoof.guid;
	err = ACAPI_Element_Get(&roof);
	if (err != NoError) return NoError;
	if (roof.header.type.typeID != API_RoofID) return NoError;
	Geometry::Plane planeTarget = GetPlaneTargetOfBeam(roof, pointTarget);
	API_StoryInfo storyInfo;
	BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
	ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
	for (API_Element beam : allBeams) 
	{
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
		double offsetFromLevel = 0;
		Point2D sb2d = { sb.x,sb.y };
		Point2D eb2d = { eb.x,eb.y };
		double sMinDis = 100000;
		double eMinDis = 100000;
		Geometry::Point3D sResult;
		Geometry::Point3D eResult;
		PlaneEq planeEq = PlaneEq(planeTarget.GetNormalVector(), planeTarget.GetBasePoint());
		Geometry::Point3D sToPlane = Geometry::ProjectAlongZ(sb2d, planeEq);
		Geometry::Point3D eToPlane = Geometry::ProjectAlongZ(eb2d, planeEq);
		double sd = Geometry::Dist3D(Geometry::Point3D{ sb.x,sb.y,0 }, sToPlane);
		double ed = Geometry::Dist3D(Geometry::Point3D{ eb.x,eb.y,0 }, eToPlane);
		if (sMinDis > sd) {
			sMinDis = sd;
			sResult = sToPlane;
		}
		if (eMinDis > ed) {
			eMinDis = ed;
			eResult = eToPlane;
		}

		offsetFromLevel = sResult.z - storyBeam.level;
		API_Element markBeam;
		ACAPI_ELEMENT_MASK_SET(markBeam, API_BeamType, level);
		ACAPI_ELEMENT_MASK_SET(markBeam, API_BeamType, isSlanted);
		ACAPI_ELEMENT_MASK_SET(markBeam, API_BeamType, slantAngle);
		beam.beam.level = offsetFromLevel;
		
		double deltaH = std::abs(sResult.z - eResult.z);
		double lengthBeam = std::sqrt((sb.x - eb.x) * (sb.x - eb.x) + (sb.y - eb.y) * (sb.y - eb.y));
		double anpha = std::atan(deltaH / lengthBeam);
		bool isMinStart = sResult.z < eResult.z ? true : false;
		if (std::abs(anpha) > 0.00001) 
		{
			beam.beam.isSlanted = true;
			if (isMinStart)
			{
				beam.beam.slantAngle = std::abs(anpha);
			}
			else {
				beam.beam.slantAngle = -std::abs(anpha);
			}
		}
		
		ACAPI_Element_Change(&beam, &markBeam, nullptr, 0UL, true);


	}
	


    return NoError;
}

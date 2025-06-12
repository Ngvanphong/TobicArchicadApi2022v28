#include "PointAndVectorUtilities.h"
#include <cmath>
#include "TransformMaxtrixUtilities.h"
#include "AlignElements.h"


bool ClickAnElem(const char* prompt,
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



GS::ErrCode AllignElements()
{
	GSErrCode			err = NoError;
	API_Neig			theNeigTarget;
	API_Coord3D* pointTarget = new API_Coord3D();
	bool isHas = ClickAnElem("Click target element", API_ZombieElemID, &theNeigTarget, nullptr, nullptr, pointTarget, false);
	if (!isHas) return NoError;
	API_Neig			theNeigOrigin;
	API_Coord3D* pointOrigin = new API_Coord3D();
	bool isHas2 = ClickAnElem("Click aligned element", API_ZombieElemID, &theNeigOrigin, nullptr, nullptr, pointOrigin, false);
	if (!isHas2) return NoError;
	API_Element targetElement = {};
	API_Element originElement = {};
	targetElement.header.guid = theNeigTarget.guid;
	err = ACAPI_Element_Get(&targetElement);
	if (err != NoError) return NoError;
	originElement.header.guid = theNeigOrigin.guid;
	err = ACAPI_Element_Get(&originElement);
	if (err != NoError) return NoError;

	API_Elem_Head			 elem;
	API_ElementMemo			 memo;
	BNZeroMemory(&elem, sizeof(API_Elem_Head));
	GS::Array< API_Elem_Head> targetElementHeads;
	GS::Array< API_Elem_Head> originElementHeads;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	std::vector<API_Element> vectorElements;
	vectorElements.push_back(targetElement);
	vectorElements.push_back(originElement);

	UInt32 index = 0;
	for (API_Element el : vectorElements) {
		API_ElemTypeID			 typeID;
		typeID = el.header.type.typeID;
		err = ACAPI_Element_GetMemo(el.header.guid, &memo);
		if (err != NoError) continue;
		//GS::UniString IdOfElemet = *memo.elemInfoString;
		if (typeID != API_CurtainWallID && typeID != API_ColumnID && typeID != API_BeamID) {
			elem.guid = el.header.guid;
			if (ACAPI_Element_GetHeader(&elem) == NoError) {
				if (index == 0) targetElementHeads.Push(elem);
				else originElementHeads.Push(elem);
			}
		}

		if (typeID == API_StairID || typeID == API_BeamID || typeID == API_ColumnID) {
			Int32 idx;
			bool isDegenerate;
			UInt64 mask = APIMemoMask_CWallFrames | APIMemoMask_CWallPanels | APIMemoMask_CWallJunctions | APIMemoMask_CWallAccessories |
				APIMemoMask_StairRiser | APIMemoMask_StairTread | APIMemoMask_StairStructure |
				APIMemoMask_BeamSegment | APIMemoMask_ColumnSegment;
			err = ACAPI_Element_GetMemo(el.header.guid, &memo, mask);
			if (err != NoError) {
				ACAPI_DisposeElemMemoHdls(&memo);
				continue;
			}

			if (typeID == API_StairID) {
				// risers
				GSSize nRisers = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.stairRisers)) / sizeof(API_StairRiserType);
				for (idx = 0; idx < nRisers; ++idx) {

					if (index == 0) targetElementHeads.Push(memo.stairRisers[idx].head);
					else originElementHeads.Push(memo.stairRisers[idx].head);
				}

				// treads
				GSSize nTreads = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.stairTreads)) / sizeof(API_StairTreadType);
				for (idx = 0; idx < nTreads; ++idx) {

					if (index == 0) targetElementHeads.Push(memo.stairTreads[idx].head);
					else originElementHeads.Push(memo.stairTreads[idx].head);
				}

				// structures
				GSSize nStructures = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.stairStructures)) / sizeof(API_StairStructureType);
				for (idx = 0; idx < nStructures; ++idx) {

					if (index == 0) targetElementHeads.Push(memo.stairStructures[idx].head);
					else originElementHeads.Push(memo.stairStructures[idx].head);
				}
			}
			else if (typeID == API_ColumnID) {
				GSSize nSegments = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.columnSegments)) / sizeof(API_ColumnSegmentType);
				for (idx = 0; idx < nSegments; ++idx) {

					if (index == 0) targetElementHeads.Push(memo.columnSegments[idx].head);
					else originElementHeads.Push(memo.columnSegments[idx].head);
				}
			}
			else if (typeID == API_BeamID) {
				GSSize nSegments = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.beamSegments)) / sizeof(API_BeamSegmentType);
				for (idx = 0; idx < nSegments; ++idx) {

					if (index == 0) targetElementHeads.Push(memo.beamSegments[idx].head);
					else originElementHeads.Push(memo.beamSegments[idx].head);
				}
			}
		}
		index += 1;
	}

	API_Coord3D normalTarget = { -1111,-1111,-1111 };
	for (auto& elemHead : targetElementHeads.AsConst())
	{
		API_Elem_Head elementHead = elemHead;
		if (elementHead.type.typeID == API_LineID)
		{
			API_Element lineElement = {};
			lineElement.header.guid = elementHead.guid;
			ACAPI_Element_Get(&lineElement);
			API_Coord directionLine{ lineElement.line.endC.x - lineElement.line.begC.x,lineElement.line.endC.y - lineElement.line.begC.y };
			API_Coord3D normVect{ -directionLine.y, directionLine.x,(*pointOrigin).z };
			double lengthNormal = std::sqrt(normVect.x * normVect.x + normVect.y * normVect.y + normVect.z * normVect.z);
			normalTarget = API_Coord3D{ normVect.x / lengthNormal, normVect.y / lengthNormal, normVect.z / lengthNormal };
			break;
		}
		API_ElemInfo3D elementInfo3d;
		err = ACAPI_ModelAccess_Get3DInfo(elementHead, &elementInfo3d);
		if (err == GS::NoError)
		{
			Int32 StartIndexBody = elementInfo3d.fbody;
			Int32 EndIndexBody = elementInfo3d.lbody;
			for (Int32 i = StartIndexBody; i <= EndIndexBody; i++)
			{
				ACAPI_ModelAccess_Get3DInfo(elementHead, &elementInfo3d);
				API_Component3D component = {};
				component.header.typeID = API_BodyID;
				component.header.index = i;
				err = ACAPI_ModelAccess_GetComponent(&component);
				if (err == GS::NoError) {
					Int32 nPgon = component.body.nPgon;
					API_Tranmat tm = component.body.tranmat;
					for (Int32 j = 1; j <= nPgon; j++)
					{
						component.header.typeID = API_PgonID;
						component.header.index = j;
						err = ACAPI_ModelAccess_GetComponent(&component);
						API_Coord3D normVect;
						if (err == NoError)
						{
							Int32 ivect = component.pgon.ivect;
							Int32 startIndexEdge = component.pgon.fpedg;
							Int32 endIndexEdge = component.pgon.lpedg;
							component.header.typeID = API_VectID;
							component.header.index = std::abs(ivect);
							err = ACAPI_ModelAccess_GetComponent(&component);
							if (err == NoError)
							{
								if (ivect < 0) {
									component.vect.x = -component.vect.x;
									component.vect.y = -component.vect.y;
									component.vect.z = -component.vect.z;
								}
								normVect = { component.vect.x, component.vect.y, component.vect.z };
								API_Coord3D	trVect;	// world coordinates
								trVect.x = tm.tmx[0] * component.vect.x + tm.tmx[1] * component.vect.y + tm.tmx[2] * component.vect.z;
								trVect.y = tm.tmx[4] * component.vect.x + tm.tmx[5] * component.vect.y + tm.tmx[6] * component.vect.z;
								trVect.z = tm.tmx[8] * component.vect.x + tm.tmx[9] * component.vect.y + tm.tmx[10] * component.vect.z;
								normVect = trVect;
								bool isPositive = false;
								API_Coord3D coordPoint = { -1111,-1111,-1111 };
								for (Int32 kk = startIndexEdge; kk <= endIndexEdge; ++kk)
								{
									component.header.typeID = API_PedgID;
									component.header.index = kk;
									err = ACAPI_ModelAccess_GetComponent(&component);
									if (err == NoError)
									{
										component.header.typeID = API_EdgeID;
										component.header.index = std::abs(component.pedg.pedg);
										if (component.pedg.pedg != 0)
										{
											err = ACAPI_ModelAccess_GetComponent(&component);
											if (err == NoError) {
												component.header.typeID = API_VertID;
												component.header.index = isPositive ? component.edge.vert1 : component.edge.vert2;
												err = ACAPI_ModelAccess_GetComponent(&component);
												if (err == NoError) {
													coordPoint.x = component.vert.x;
													coordPoint.y = component.vert.y;
													coordPoint.z = component.vert.z;
													break;
												}
											}
										}
									}
								}
								if (!(coordPoint.x == -1111 && coordPoint.y == -1111 && coordPoint.z == -1111))
								{
									double distance = DistancePointToPlane(normVect, coordPoint, *pointTarget);
									if (std::abs(distance) < 0.0000001) {
										normalTarget = normVect;
										break;
									}
								}
							}
						}

					}
				}
			}
		}

	}


	API_Coord3D normalOrigin = { -1111,-1111,-1111 };
	for (auto& elemHead : originElementHeads.AsConst())
	{
		API_Elem_Head elementHead = elemHead;
		
		API_ElemInfo3D elementInfo3d;
		err = ACAPI_ModelAccess_Get3DInfo(elementHead, &elementInfo3d);
		if (err == GS::NoError)
		{
			Int32 StartIndexBody = elementInfo3d.fbody;
			Int32 EndIndexBody = elementInfo3d.lbody;
			for (Int32 i = StartIndexBody; i <= EndIndexBody; i++)
			{
				ACAPI_ModelAccess_Get3DInfo(elementHead, &elementInfo3d);
				API_Component3D component = {};
				component.header.typeID = API_BodyID;
				component.header.index = i;
				err = ACAPI_ModelAccess_GetComponent(&component);
				if (err == GS::NoError) {
					Int32 nPgon = component.body.nPgon;
					API_Tranmat tm = component.body.tranmat;
					for (Int32 j = 1; j <= nPgon; j++)
					{
						component.header.typeID = API_PgonID;
						component.header.index = j;
						err = ACAPI_ModelAccess_GetComponent(&component);
						API_Coord3D normVect;
						if (err == NoError)
						{
							Int32 ivect = component.pgon.ivect;
							Int32 startIndexEdge = component.pgon.fpedg;
							Int32 endIndexEdge = component.pgon.lpedg;
							component.header.typeID = API_VectID;
							component.header.index = std::abs(ivect);
							err = ACAPI_ModelAccess_GetComponent(&component);
							if (err == NoError)
							{
								if (ivect < 0) {
									component.vect.x = -component.vect.x;
									component.vect.y = -component.vect.y;
									component.vect.z = -component.vect.z;
								}
								normVect = { component.vect.x, component.vect.y, component.vect.z };
								API_Coord3D	trVect;	// world coordinates
								trVect.x = tm.tmx[0] * component.vect.x + tm.tmx[1] * component.vect.y + tm.tmx[2] * component.vect.z;
								trVect.y = tm.tmx[4] * component.vect.x + tm.tmx[5] * component.vect.y + tm.tmx[6] * component.vect.z;
								trVect.z = tm.tmx[8] * component.vect.x + tm.tmx[9] * component.vect.y + tm.tmx[10] * component.vect.z;
								normVect = trVect;
								bool isPositive = false;
								API_Coord3D coordPoint = { -1111,-1111,-1111 };
								for (Int32 kk = startIndexEdge; kk <= endIndexEdge; ++kk)
								{
									component.header.typeID = API_PedgID;
									component.header.index = kk;
									err = ACAPI_ModelAccess_GetComponent(&component);
									if (err == NoError)
									{
										component.header.typeID = API_EdgeID;
										component.header.index = std::abs(component.pedg.pedg);
										if (component.pedg.pedg != 0)
										{
											err = ACAPI_ModelAccess_GetComponent(&component);
											if (err == NoError) {
												component.header.typeID = API_VertID;
												component.header.index = isPositive ? component.edge.vert1 : component.edge.vert2;
												err = ACAPI_ModelAccess_GetComponent(&component);
												if (err == NoError) {
													coordPoint.x = component.vert.x;
													coordPoint.y = component.vert.y;
													coordPoint.z = component.vert.z;
													break;
												}
											}
										}
									}
								}
								if (!(coordPoint.x == -1111 && coordPoint.y == -1111 && coordPoint.z == -1111))
								{
									double distance = DistancePointToPlane(normVect, coordPoint, *pointOrigin);
									if (std::abs(distance) < 0.0000001) {
										normalOrigin = normVect;
										break;
									}
								}
							}
						}

					}
				}
			}
		}
	}


	if (!((normalTarget.x == -1111 && normalTarget.y == -1111 && normalTarget.z == -1111)||
		(normalOrigin.x == -1111 && normalOrigin.y == -1111 && normalOrigin.z == -1111)))
	{
		GS::Array<API_Neig> items;
		items.Push(theNeigOrigin);
		auto dotProdut=	Dot_product(normalTarget, normalOrigin);
		if(std::abs(std::abs(dotProdut)-1)> 0.0001)
		{
			API_EditPars		editParRotates;
			editParRotates.typeID = APIEdit_Rotate;
			editParRotates.origC.x = pointOrigin->x;
			editParRotates.origC.y = pointOrigin->y;
			editParRotates.withDelete = true;
			auto crossVector = VectProd(normalOrigin, normalTarget);
			API_Coord3D targetOnPlane = ProjectOnto(crossVector, *pointOrigin, *pointTarget);
			API_Coord3D vectorX2 = Normalize(targetOnPlane - *pointOrigin);
			API_Coord3D vectorZ2 = Normalize(crossVector);

			double angleVector = AngleTwoVector(normalOrigin, normalTarget);
			if (std::abs(angleVector) > atan(1.0) * 4 / 2) angleVector = atan(1.0) * 4 - std::abs(angleVector);
			API_Coord3D A = { 0,1,0};
			API_Coord3D Az1 = MaxtrixRotateZ(angleVector, &A);
			API_Coord3D Az2 = MaxtrixRotateZ(-angleVector, &A);
			API_Coord3D zVector = { 0,0,1 };
			
			if (std::abs(std::abs(Dot_product(zVector,vectorZ2)) - 1) < 0.0001)
			{
				API_Coord3D end = Az1;
				API_Coord3D startTransfrom = A + *pointOrigin;
				API_Coord3D endTransfrom = end + *pointOrigin;
				editParRotates.begC = startTransfrom;
				editParRotates.endC = endTransfrom;
				err = ACAPI_Element_Edit(&items, editParRotates);

				//Check again;
				//
				API_Coord3D normalOriginCheck;
				for (auto& elemHead : originElementHeads.AsConst())
				{
					API_Elem_Head elementHead = elemHead;
					API_ElemInfo3D elementInfo3d;
					err = ACAPI_ModelAccess_Get3DInfo(elementHead, &elementInfo3d);
					if (err == GS::NoError)
					{
						Int32 StartIndexBody = elementInfo3d.fbody;
						Int32 EndIndexBody = elementInfo3d.lbody;
						for (Int32 i = StartIndexBody; i <= EndIndexBody; i++)
						{
							ACAPI_ModelAccess_Get3DInfo(elementHead, &elementInfo3d);
							API_Component3D component = {};
							component.header.typeID = API_BodyID;
							component.header.index = i;
							err = ACAPI_ModelAccess_GetComponent(&component);
							if (err == GS::NoError) {
								Int32 nPgon = component.body.nPgon;
								API_Tranmat tm = component.body.tranmat;
								for (Int32 j = 1; j <= nPgon; j++)
								{
									component.header.typeID = API_PgonID;
									component.header.index = j;
									err = ACAPI_ModelAccess_GetComponent(&component);
									API_Coord3D normVect;
									if (err == NoError)
									{
										Int32 ivect = component.pgon.ivect;
										Int32 startIndexEdge = component.pgon.fpedg;
										Int32 endIndexEdge = component.pgon.lpedg;
										component.header.typeID = API_VectID;
										component.header.index = std::abs(ivect);
										err = ACAPI_ModelAccess_GetComponent(&component);
										if (err == NoError)
										{
											if (ivect < 0) {
												component.vect.x = -component.vect.x;
												component.vect.y = -component.vect.y;
												component.vect.z = -component.vect.z;
											}
											normVect = { component.vect.x, component.vect.y, component.vect.z };
											API_Coord3D	trVect;	// world coordinates
											trVect.x = tm.tmx[0] * component.vect.x + tm.tmx[1] * component.vect.y + tm.tmx[2] * component.vect.z;
											trVect.y = tm.tmx[4] * component.vect.x + tm.tmx[5] * component.vect.y + tm.tmx[6] * component.vect.z;
											trVect.z = tm.tmx[8] * component.vect.x + tm.tmx[9] * component.vect.y + tm.tmx[10] * component.vect.z;
											normVect = trVect;
											bool isPositive = false;
											API_Coord3D coordPoint = { -1111,-1111,-1111 };
											for (Int32 kk = startIndexEdge; kk <= endIndexEdge; ++kk)
											{
												component.header.typeID = API_PedgID;
												component.header.index = kk;
												err = ACAPI_ModelAccess_GetComponent(&component);
												if (err == NoError)
												{
													component.header.typeID = API_EdgeID;
													component.header.index = std::abs(component.pedg.pedg);
													if (component.pedg.pedg != 0)
													{
														err = ACAPI_ModelAccess_GetComponent(&component);
														if (err == NoError) {
															component.header.typeID = API_VertID;
															component.header.index = isPositive ? component.edge.vert1 : component.edge.vert2;
															err = ACAPI_ModelAccess_GetComponent(&component);
															if (err == NoError) {
																coordPoint.x = component.vert.x;
																coordPoint.y = component.vert.y;
																coordPoint.z = component.vert.z;
																break;
															}
														}
													}
												}
											}
											if (!(coordPoint.x == -1111 && coordPoint.y == -1111 && coordPoint.z == -1111))
											{
												double distance = DistancePointToPlane(normVect, coordPoint, *pointOrigin);
												if (std::abs(distance) < 0.0000001) {
													normalOriginCheck = normVect;
													break;
												}
											}
										}
									}

								}
							}
						}
					}
				}
				///
				///
				double dotCheck = Dot_product(normalTarget, normalOriginCheck);
				if (std::abs(std::abs(dotCheck) - 1) > 0.0001) 
				{
					editParRotates.begC = endTransfrom;
					editParRotates.endC = startTransfrom;
					err = ACAPI_Element_Edit(&items, editParRotates);
					editParRotates.begC = startTransfrom;
					endTransfrom = Az2 + *pointOrigin;
					editParRotates.endC = endTransfrom;
					err = ACAPI_Element_Edit(&items, editParRotates);
				}

			}
		}

		API_Coord3D projectOriginToTargetPlane= ProjectOnto(normalTarget, *pointTarget, *pointOrigin);
		API_EditPars		editPars;
		editPars.typeID = APIEdit_Drag;
		editPars.withDelete = true;
		editPars.begC = *pointOrigin;
		editPars.endC = projectOriginToTargetPlane;
		err = ACAPI_Element_Edit(&items, editPars);
	}
	ACAPI_DisposeElemMemoHdls(&memo);
	return GS::ErrCode();
}

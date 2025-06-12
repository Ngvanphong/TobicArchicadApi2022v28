#include "PointAndVectorUtilities.h"
#include "CreateCoppha.h"
#include "basicgeometry.h"

std::vector<API_Element> SelectionElementCopphas(GS::Array<API_Neig> selNeigs) {
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
		if (err == GS::NoError) {
			vectorElements.push_back(element);
		}
	}
	return vectorElements;
}

bool CheckHasInterseciton(API_Box3D& boxItem, API_Box3D* extent3D) {
	if ((extent3D->zMax <= boxItem.zMax + 0.2 && extent3D->zMax >= boxItem.zMin - 0.2) ||
		(extent3D->zMin <= boxItem.zMax + 0.2 && extent3D->zMin >= boxItem.zMin - 0.2) ||
		(boxItem.zMin <= extent3D->zMax + 0.2 && boxItem.zMin >= extent3D->zMin - 0.2) ||
		(boxItem.zMax >= extent3D->zMin - 0.2 && boxItem.zMax <= extent3D->zMax + 0.2))
	{

		if ((extent3D->xMax <= boxItem.xMax + 0.2 && extent3D->xMax >= boxItem.xMin - 0.2) ||
			(extent3D->xMin <= boxItem.xMax + 0.2 && extent3D->xMin >= boxItem.xMin - 0.2) ||
			(boxItem.xMin <= extent3D->xMax + 0.2 && boxItem.xMin >= extent3D->xMin - 0.2) ||
			(boxItem.xMax >= extent3D->xMin - 0.2 && boxItem.xMax <= extent3D->xMax + 0.2))
		{

			if ((extent3D->yMax <= boxItem.yMax + 0.2 && extent3D->yMax >= boxItem.yMin - 0.2) ||
				(extent3D->yMin <= boxItem.yMax + 0.2 && extent3D->yMin >= boxItem.yMin - 0.2) ||
				(boxItem.yMin <= extent3D->yMax + 0.2 && boxItem.yMin >= extent3D->yMin - 0.2) ||
				(boxItem.yMax >= extent3D->yMin - 0.2 && boxItem.yMax <= extent3D->yMax + 0.2))
			{
				return true;
			}
		}
		
	}
	return false;
}


GS::ErrCode CreateCoppha()
{

	// Change Defaul layer for morph before adrawing
	API_Element elementLayer, mask2;
	BNZeroMemory(&elementLayer, sizeof(API_Element));
	elementLayer.header.type.typeID = API_MorphID;
	ACAPI_Element_GetDefaults(&elementLayer, nullptr);
	ACAPI_ELEMENT_MASK_CLEAR(mask2);
	ACAPI_ELEMENT_MASK_SET(mask2, API_Elem_Head, layer);
	elementLayer.header.layer = APIApplicationLayerAttributeIndex;
	ACAPI_Element_ChangeDefaults(&elementLayer, nullptr, &mask2);

	GS::Array<API_Guid> allGuidProject;
	ACAPI_Element_GetElemList(API_ZombieElemID, &allGuidProject);

	GS::Array<API_Element> allItemCheck;
	for (auto id : allGuidProject)
	{
		API_Element itemElement;
		BNClear(itemElement);
		itemElement.header.guid = id;
		auto err2 = ACAPI_Element_Get(&itemElement);
		if (err2 == NoError) 
		{
			allItemCheck.Push(itemElement);
		}
	}

	GS::ErrCode err = GS::NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> vectorElements = SelectionElementCopphas(selNeigs);
	GS::Array<DataHeadGeo> headerElems;
	API_Elem_Head			 elem;
	API_ElementMemo			 memo;

	BNZeroMemory(&elem, sizeof(API_Elem_Head));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	for (API_Element el : vectorElements) {
		API_ElemTypeID			 typeID;
		typeID = el.header.type.typeID;
		err = ACAPI_Element_GetMemo(el.header.guid, &memo);
		if (err != NoError) continue;
		GS::UniString IdOfElemet = *memo.elemInfoString;
		if (typeID != API_CurtainWallID && typeID != API_ColumnID && typeID != API_BeamID) {
			elem.guid = el.header.guid;
			if (ACAPI_Element_GetHeader(&elem) == NoError) {
				DataHeadGeo dataHeadGeo;
				dataHeadGeo.IdOfElement = IdOfElemet;
				dataHeadGeo.ElementGuid = el.header.guid;
				dataHeadGeo.HeadOfGeometry = elem;
				dataHeadGeo.ElementTypeId = el.header.type.typeID;
				headerElems.Push(dataHeadGeo);
			}
		}

		if (typeID == API_StairID || typeID == API_BeamID || typeID == API_ColumnID) {
			Int32 idx;
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
					DataHeadGeo dataHeadGeo;
					dataHeadGeo.IdOfElement = IdOfElemet;
					dataHeadGeo.ElementGuid = el.header.guid;
					dataHeadGeo.HeadOfGeometry = memo.stairRisers[idx].head;
					dataHeadGeo.ElementTypeId = el.header.type.typeID;
					headerElems.Push(dataHeadGeo);
				}

				// treads
				GSSize nTreads = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.stairTreads)) / sizeof(API_StairTreadType);
				for (idx = 0; idx < nTreads; ++idx) {
					DataHeadGeo dataHeadGeo;
					dataHeadGeo.IdOfElement = IdOfElemet;
					dataHeadGeo.ElementGuid = el.header.guid;
					dataHeadGeo.HeadOfGeometry = memo.stairTreads[idx].head;
					dataHeadGeo.ElementTypeId = el.header.type.typeID;
					headerElems.Push(dataHeadGeo);
				}

				// structures
				GSSize nStructures = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.stairStructures)) / sizeof(API_StairStructureType);
				for (idx = 0; idx < nStructures; ++idx) {
					DataHeadGeo dataHeadGeo;
					dataHeadGeo.IdOfElement = IdOfElemet;
					dataHeadGeo.ElementGuid = el.header.guid;
					dataHeadGeo.HeadOfGeometry = memo.stairStructures[idx].head;
					dataHeadGeo.ElementTypeId = el.header.type.typeID;
					headerElems.Push(dataHeadGeo);
				}
			}
			else if (typeID == API_ColumnID) {
				GSSize nSegments = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.columnSegments)) / sizeof(API_ColumnSegmentType);
				for (idx = 0; idx < nSegments; ++idx) {
					DataHeadGeo dataHeadGeo;
					dataHeadGeo.IdOfElement = IdOfElemet;
					dataHeadGeo.ElementGuid = el.header.guid;
					dataHeadGeo.HeadOfGeometry = memo.columnSegments[idx].head;
					dataHeadGeo.ElementTypeId = el.header.type.typeID;
					headerElems.Push(dataHeadGeo);
				}
			}
			else if (typeID == API_BeamID) {
				GSSize nSegments = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.beamSegments)) / sizeof(API_BeamSegmentType);
				for (idx = 0; idx < nSegments; ++idx) {
					DataHeadGeo dataHeadGeo;
					dataHeadGeo.IdOfElement = IdOfElemet;
					dataHeadGeo.ElementGuid = el.header.guid;
					dataHeadGeo.HeadOfGeometry = memo.beamSegments[idx].head;
					dataHeadGeo.ElementTypeId = el.header.type.typeID;
					headerElems.Push(dataHeadGeo);
				}
			}
		}
	}


	API_Box3D* extent3D = new API_Box3D();

	for (auto& elementHeadGeo : headerElems.AsConst()) {
		API_Elem_Head elementHead = elementHeadGeo.HeadOfGeometry;
		API_ElemInfo3D elementInfo3d;
		err = ACAPI_ModelAccess_Get3DInfo(elementHead, &elementInfo3d);

		if (err == GS::NoError) {
			*extent3D = elementInfo3d.bounds;
			Int32 StartIndexBody = elementInfo3d.fbody;
			Int32 EndIndexBody = elementInfo3d.lbody;
			for (Int32 i = StartIndexBody; i <= EndIndexBody; i++) {
				ACAPI_ModelAccess_Get3DInfo(elementHead, &elementInfo3d);

				API_Component3D component = {};
				component.header.typeID = API_BodyID;
				component.header.index = i;
				err = ACAPI_ModelAccess_GetComponent(&component);
				if (err == GS::NoError) {
					Int32 nPgon = component.body.nPgon;
					API_Tranmat tm = component.body.tranmat;

					void* bodyData = nullptr;
					ACAPI_Body_Create(nullptr, nullptr, &bodyData);
					if (bodyData == nullptr) {
						continue;
					}
					Int32	polyNormals[1000];
					UInt32 polygons[1000];
					UInt32 arrayVertex[10000];
					Int32 edges[10000];
					bool HasPolygon = false;

					for (Int32 j = 1; j <= nPgon; j++) {
						component.header.typeID = API_PgonID;
						component.header.index = j;
						err = ACAPI_ModelAccess_GetComponent(&component);
						API_Coord3D normVect;

						if (err == NoError) {
							Int32 ivect = component.pgon.ivect;
							Int32 startIndexEdge = component.pgon.fpedg;
							Int32 endIndexEdge = component.pgon.lpedg;

							component.header.typeID = API_VectID;
							component.header.index = std::abs(ivect);
							err = ACAPI_ModelAccess_GetComponent(&component);
							if (err == NoError) {
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
								ACAPI_Body_AddPolyNormal(bodyData, normVect, polyNormals[j - 1]);

								std::vector<VertexEdge> arrayVertexEdge;
								bool isPositive = false;
								for (Int32 kk = startIndexEdge; kk <= endIndexEdge; ++kk) {
									component.header.typeID = API_PedgID;
									component.header.index = kk;
									err = ACAPI_ModelAccess_GetComponent(&component);
									if (err == NoError) {
										component.header.typeID = API_EdgeID;
										component.header.index = std::abs(component.pedg.pedg);
										Int32 indexEdge = std::abs(component.pedg.pedg);
										Int32 indexEdgeWithSign = component.pedg.pedg;
										isPositive = component.pedg.pedg > 0;
										if (component.pedg.pedg != 0) {
											err = ACAPI_ModelAccess_GetComponent(&component);
											if (err == NoError) {
												component.header.typeID = API_VertID;
												component.header.index = isPositive ? component.edge.vert1 : component.edge.vert2;
												VertexEdge vertextEdge;
												vertextEdge.Vertext1 = component.edge.vert1 - 1;
												vertextEdge.Vertext2 = component.edge.vert2 - 1;
												vertextEdge.IndexEdge = indexEdge - 1;
												vertextEdge.IndexVertext = component.header.index - 1;
												vertextEdge.IsPositive = isPositive;
												vertextEdge.NormalVector = trVect;

												err = ACAPI_ModelAccess_GetComponent(&component);
												if (err == NoError) {
													API_Coord3D coordPoint;
													coordPoint.x = component.vert.x;
													coordPoint.y = component.vert.y;
													coordPoint.z = component.vert.z;
													vertextEdge.PointVertex = coordPoint;
													arrayVertexEdge.push_back(vertextEdge);
												}
											}
										}
										else {
											VertexEdge vertextEdge;
											API_Coord3D coordPoint;
											coordPoint.x = -1111;
											coordPoint.y = -1111;
											coordPoint.z = -1111;
											vertextEdge.PointVertex = coordPoint;
											arrayVertexEdge.push_back(vertextEdge);
										}
									}
								}

								for (VertexEdge itemVer : arrayVertexEdge) {
									if (!(itemVer.PointVertex.x == -1111 && itemVer.PointVertex.y == -1111 && itemVer.PointVertex.z == -1111))
									{
										ACAPI_Body_AddVertex(bodyData, itemVer.PointVertex, arrayVertex[itemVer.IndexVertext]);
									}
								}
								GS::Array<Int32> arrayEdges;
								for (VertexEdge itemVer : arrayVertexEdge) {
									if (!(itemVer.PointVertex.x == -1111 && itemVer.PointVertex.y == -1111 && itemVer.PointVertex.z == -1111))
									{
										ACAPI_Body_AddEdge(bodyData, arrayVertex[itemVer.Vertext1], arrayVertex[itemVer.Vertext2], edges[itemVer.IndexEdge]);
										arrayEdges.Push(itemVer.IsPositive > 0 ? edges[itemVer.IndexEdge] : -edges[itemVer.IndexEdge]);
									}
									else {
										arrayEdges.Push(0);
									}
								}
								API_OverriddenAttribute material;
								material = ACAPI_CreateAttributeIndex(1);

								API_Vector3D normalVector3D = { normVect.x,normVect.y,normVect.z };
								API_Vector3D normalVectorZZ = { 0,0,1 };
								float dotProductItem2 = Dot_product(normalVector3D, normalVectorZZ);

								if (elementHeadGeo.ElementTypeId == API_ColumnID) {
									if (!std::abs(abs(dotProductItem2) - 1) < 0.0001)
									{
										ACAPI_Body_AddPolygon(bodyData, arrayEdges, polyNormals[j - 1], material, polygons[j - 1]);
										HasPolygon = true;
									}
								}
								else {
									if (!std::abs(dotProductItem2 - 1) < 0.0001)
									{
										ACAPI_Body_AddPolygon(bodyData, arrayEdges, polyNormals[j - 1], material, polygons[j - 1]);
										HasPolygon = true;
									}
								}
							}
						}
					}
					if (HasPolygon == false) continue;
					API_Element element;
					BNZeroMemory(&element, sizeof(API_Element));
					element.header.type.typeID = API_MorphID;
					err = ACAPI_Element_GetDefaults(&element, nullptr);
					element.morph.tranmat = tm;

					API_ElementMemo memoNew = {};
					API_Element		   maskNew;
					ACAPI_Body_Finish(bodyData, &memoNew.morphBody, &memoNew.morphMaterialMapTable);
					ACAPI_Body_Dispose(&bodyData);

					memoNew.elemInfoString = new GS::UniString(elementHeadGeo.IdOfElement);
					ACAPI_Element_ChangeParameters({ element.header.guid }, &element, &memoNew, &maskNew);


					err = ACAPI_Element_Create(&element, &memoNew);
					ACAPI_DisposeElemMemoHdls(&memoNew);
					if (err != NoError) continue;

					//Modify morph

					GS::Array<API_Guid>	guid_Targets;
					err = ACAPI_Element_SolidLink_GetTargets(elementHeadGeo.ElementGuid, &guid_Targets);
					for (API_Guid guidItem : guid_Targets) {
						ACAPI_Element_SolidLink_Create(element.header.guid, guidItem, APISolid_Substract, APISolidFlag_OperatorAttrib);
					}

					GS::Array<API_Guid>	guid_Operators;
					ACAPI_Element_SolidLink_GetOperators(elementHeadGeo.ElementGuid, &guid_Operators);
					for (API_Guid guidItem : guid_Operators) {
						ACAPI_Element_SolidLink_Create(element.header.guid, guidItem, APISolid_Substract, APISolidFlag_OperatorAttrib);
					}
					
					for (API_Element item : allItemCheck) 
					{
						API_ElemTypeID typeItemId = item.header.type.typeID;
						if (typeItemId == API_WallID || typeItemId == API_ColumnID || typeItemId == API_BeamID ||
							typeItemId == API_ObjectID  || typeItemId == API_SlabID ||
							typeItemId == API_RoofID || typeItemId == API_MeshID || typeItemId == API_StairID 
							|| typeItemId == API_LampID ||typeItemId == API_MorphID
							/*||typeItemId == API_WindowID || typeItemId == API_DoorID */
							/*typeItemId == API_BeamSegmentID|| typeItemId == API_ColumnSegmentID */
							)
						{

							if (item.header.guid != element.header.guid && item.header.guid != elementHeadGeo.ElementGuid)
							{
								if (typeItemId == API_MorphID) 
								{
									if (item.header.layer == APIApplicationLayerAttributeIndex) continue;
								}
								API_Box3D boxItem;
								err = ACAPI_Element_CalcBounds(&item.header, &boxItem);
								if (err == NoError) 
								{
									if (CheckHasInterseciton(boxItem, extent3D)) 
									{
										ACAPI_Element_SolidLink_Create(element.header.guid, item.header.guid, APISolid_Substract, APISolidFlag_OperatorAttrib);
									}	
								}

							}
						}
					}

				}
			}
		}
	}


	//Huy memo
	delete extent3D;
	ACAPI_DisposeElemMemoHdls(&memo);
	return NoError;
}
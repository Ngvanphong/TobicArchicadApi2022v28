#pragma once
#include "ACAPinc.h"
struct DataCoppha
{
	API_PgonType PolygonType;

	API_Polygon Polygon;

	API_Coord3D NormalVector;

	API_Guid ElementGuid;

	API_ElemTypeID ElementTypeId;

	API_Elem_Head  HeadOfGeometry;

};

struct DataHeadGeo {
	API_Guid ElementGuid;

	API_Elem_Head HeadOfGeometry;

	API_ElemTypeID ElementTypeId;

	GS::UniString IdOfElement;
};

struct VertexEdge
{
	API_Coord3D NormalVector;
	Int32 Vertext1;
	Int32 Vertext2;
	Int32 IndexEdge;
	bool IsPositive;
	Int32 IndexVertext;
	API_Coord3D PointVertex;

};
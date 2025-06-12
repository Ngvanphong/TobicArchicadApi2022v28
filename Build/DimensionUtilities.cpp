#include "DimensionUtilities.h"



void SetTypeForDimension(API_Element& newDim) {
	newDim.dimension.dimAppear = APIApp_Normal;
	newDim.dimension.textPos = APIPos_Above;
	newDim.dimension.textWay = APIDir_Parallel;
	newDim.dimension.usedIn3D = false;
	newDim.dimension.horizontalText = false;
	newDim.dimension.defNote.noteSize = 2.5;
	newDim.dimension.markerData.markerType = APIMark_FullCirc;
	newDim.dimension.defWitnessForm = APIWtn_None;
	newDim.dimension.clipOtherSide = true;
}

void SetTypeForDimensionColumn(API_Element& newDim) {
	newDim.dimension.dimAppear = APIApp_Normal;
	newDim.dimension.textPos = APIPos_Above;
	newDim.dimension.textWay = APIDir_Parallel;
	newDim.dimension.usedIn3D = false;
	newDim.dimension.horizontalText = false;
	newDim.dimension.defNote.noteSize = 2.5;
	newDim.dimension.markerData.markerType = APIMark_FullCirc;
	newDim.dimension.defWitnessForm = APIWtn_Fix;
	newDim.dimension.defWitnessVal = 0.45;
	newDim.dimension.clipOtherSide = true;
}

API_Coord* SetDirectionElementToDim(const API_Coord p1, const API_Coord p2) {
	API_Coord begin;
	API_Coord end;

	if (abs(p1.x - p2.x) < 0.0001) {
		if (p1.y < p2.y) {
			begin = p1;
			end = p2;
		}
		else {
			begin = p2;
			end = p1;
		}
	}
	else if (abs(p1.y - p2.y) < 0.0001) {
		if (p1.x < p2.x) {
			begin = p2;
			end = p1;
		}
		else {
			
			begin = p1;
			end = p2;
		}
	}
	else {
		begin = p1;
		end = p2;
	}
	API_Coord result[2] = { begin,end };
	return result;
}

API_Coord* SetDirectionDimFollowParallel(const API_Coord p1, const API_Coord p2) {
	API_Coord begin;
	API_Coord end;

	if (abs(p1.x - p2.x) < 0.0001) {
		if (p1.y < p2.y) {
			begin = p1;
			end = p2;
		}
		else {
			begin = p2;
			end = p1;
		}
	}
	else if (abs(p1.y - p2.y) < 0.0001) {
		if (p1.x < p2.x) {
			begin = p1;
			end = p2;
		}
		else {

			begin = p2;
			end = p1;
		}
	}
	else {
		begin = p1;
		end = p2;
	}
	API_Coord result[2] = { begin,end };
	return result;
}


double calculateDistance(std::pair<double, double>& x, std::pair<double, double>& y)
{
	return sqrt(pow(x.first - y.first, 2) +
		pow(x.second - y.second, 2));
}
void crossProduct(int v_A[], int v_B[], int c_P[]) {
	c_P[0] = v_A[1] * v_B[2] - v_A[2] * v_B[1];
	c_P[1] = -(v_A[0] * v_B[2] - v_A[2] * v_B[0]);
	c_P[2] = v_A[0] * v_B[1] - v_A[1] * v_B[0];
}

int dotProduct(int vector_a[], int vector_b[]) {
	int product = 0;
	for (int i = 0; i < 3; i++)
		product = product + vector_a[i] * vector_b[i];
	return product;
}

GS::Array<API_Coord> GetHotspotColumnToDimX(const API_Element element,const API_Coord locationColumn,GS::Array<API_Coord>* arrayHotspotY) {
	GS::Array<API_Coord> result;
	GS::Array <API_ElementHotspot> hotspotArray;
	GS::ErrCode err = ACAPI_Element_GetHotspots(element.header.guid, &hotspotArray);
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
		if (std::abs(locationColumn.x - hotspotPoint.x) < 0.0001 || std::abs(locationColumn.y - hotspotPoint.y) < 0.0001) {
			isExist = true;
		}
		if (!isExist) {
			result.Push(hotspotPoint);
		}
	}
	GS::Array<API_Coord> resutX;
	for (Int32 i = 0; i < result.GetSize(); i++)
	{
		API_Coord pointItem = result.Get(i);
		if (pointItem.y< locationColumn.y) {
			bool isExist = false;
			for (API_Coord item: resutX) {
				if (std::abs(item.x - pointItem.x) < 0.0001) {
					isExist = true;
					break;
				}
			}
			if(!isExist)resutX.Push(pointItem);
		}
		if (pointItem.x > locationColumn.x) {
			bool isExist = false;
			for (API_Coord item : (*arrayHotspotY)) {
				if (std::abs(item.y - pointItem.y) < 0.0001) {
					isExist = true;
					break;
				}
			}
			if (!isExist)(*arrayHotspotY).Push(pointItem);
			
		}
	}

	return resutX;
}


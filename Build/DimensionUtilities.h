#pragma once
#include "ACAPinc.h"

API_Coord* SetDirectionElementToDim(const API_Coord, const API_Coord);

API_Coord* SetDirectionDimFollowParallel(const API_Coord, const API_Coord);

void SetTypeForDimension(API_Element&);

void SetTypeForDimensionColumn(API_Element&);

double calculateDistance(std::pair<double, double>&, std::pair<double, double>&);

void crossProduct(int v_A[], int v_B[], int c_P[]);


int dotProduct(int vector_a[], int vector_b[]);

GS::Array<API_Coord> GetHotspotColumnToDimX(const API_Element element, const API_Coord locationColumn, GS::Array<API_Coord>* arrayHotspotY);


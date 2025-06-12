#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include "PointAndVectorUtilities.h"
#include <cmath>

API_Coord3D MaxtrixRotateX(float angle,API_Coord3D* pointOrigin)
{
	API_Tranmat tmRotate;
	tmRotate.tmx[0] = 1;
	tmRotate.tmx[1] = 0;
	tmRotate.tmx[2] = 0;
	tmRotate.tmx[3] = 0;
	tmRotate.tmx[4] = 0;
	tmRotate.tmx[5] = cos(angle);
	tmRotate.tmx[6] = -sin(angle);
	tmRotate.tmx[7] = 0;
	tmRotate.tmx[8] = 0;
	tmRotate.tmx[9] = sin(angle);
	tmRotate.tmx[10] = cos(angle);
	tmRotate.tmx[11] = 0;
	API_Coord3D	pointTransform;
	pointTransform.x = tmRotate.tmx[0] * pointOrigin->x + tmRotate.tmx[1] * pointOrigin->y + tmRotate.tmx[2] * pointOrigin->z + tmRotate.tmx[3];
	pointTransform.y = tmRotate.tmx[4] * pointOrigin->x + tmRotate.tmx[5] * pointOrigin->y + tmRotate.tmx[6] * pointOrigin->z + tmRotate.tmx[7];
	pointTransform.z = tmRotate.tmx[8] * pointOrigin->x + tmRotate.tmx[9] * pointOrigin->y + tmRotate.tmx[10] * pointOrigin->z + tmRotate.tmx[11];
	return pointTransform;
}

API_Coord3D MaxtrixRotateY(float angle, API_Coord3D* pointOrigin)
{
	API_Tranmat tmRotate;
	tmRotate.tmx[0] = cos(angle);
	tmRotate.tmx[1] = 0;
	tmRotate.tmx[2] = sin(angle);
	tmRotate.tmx[3] = 0;
	tmRotate.tmx[4] = 0;
	tmRotate.tmx[5] = 1;
	tmRotate.tmx[6] = 0;
	tmRotate.tmx[7] = 0;
	tmRotate.tmx[8] = -sin(angle);
	tmRotate.tmx[9] = 0;
	tmRotate.tmx[10] = cos(angle);
	tmRotate.tmx[11] = 0;
	API_Coord3D	pointTransform;
	pointTransform.x = tmRotate.tmx[0] * pointOrigin->x + tmRotate.tmx[1] * pointOrigin->y + tmRotate.tmx[2] * pointOrigin->z + tmRotate.tmx[3];
	pointTransform.y = tmRotate.tmx[4] * pointOrigin->x + tmRotate.tmx[5] * pointOrigin->y + tmRotate.tmx[6] * pointOrigin->z + tmRotate.tmx[7];
	pointTransform.z = tmRotate.tmx[8] * pointOrigin->x + tmRotate.tmx[9] * pointOrigin->y + tmRotate.tmx[10] * pointOrigin->z + tmRotate.tmx[11];
	return pointTransform;
}

API_Coord3D MaxtrixRotateZ(float angle, API_Coord3D* pointOrigin)
{
	API_Tranmat tmRotate;
	tmRotate.tmx[0] = cos(angle);
	tmRotate.tmx[1] = -sin(angle);
	tmRotate.tmx[2] = 0;
	tmRotate.tmx[3] = 0;
	tmRotate.tmx[4] = sin(angle);
	tmRotate.tmx[5] = cos(angle);
	tmRotate.tmx[6] = 0;
	tmRotate.tmx[7] = 0;
	tmRotate.tmx[8] = 0;
	tmRotate.tmx[9] = 0;
	tmRotate.tmx[10] = 1;
	tmRotate.tmx[11] = 0;
	API_Coord3D	pointTransform;
	pointTransform.x = tmRotate.tmx[0] * pointOrigin->x + tmRotate.tmx[1] * pointOrigin->y + tmRotate.tmx[2] * pointOrigin->z + tmRotate.tmx[3];
	pointTransform.y = tmRotate.tmx[4] * pointOrigin->x + tmRotate.tmx[5] * pointOrigin->y + tmRotate.tmx[6] * pointOrigin->z + tmRotate.tmx[7];
	pointTransform.z = tmRotate.tmx[8] * pointOrigin->x + tmRotate.tmx[9] * pointOrigin->y + tmRotate.tmx[10] * pointOrigin->z + tmRotate.tmx[11];
	return pointTransform;
}




#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"




static API_Coord3D		operator- (const API_Coord3D& c1, const API_Coord3D& c2)

{
	API_Coord3D c;

	c.x = c1.x - c2.x;
	c.y = c1.y - c2.y;
	c.z = c1.z - c2.z;
	return c;
}

static API_Coord3D		operator+ (const API_Coord3D& c1, const API_Coord3D& c2)

{
	API_Coord3D c;

	c.x = c1.x + c2.x;
	c.y = c1.y + c2.y;
	c.z = c1.z + c2.z;
	return c;
}

static API_Coord		operator- (API_Coord c1, API_Coord c2)

{
	API_Coord c;

	c.x = c1.x - c2.x;
	c.y = c1.y - c2.y;
	return c;
}

static API_Coord		operator+ (API_Coord c1, API_Coord c2)

{
	API_Coord c;

	c.x = c1.x + c2.x;
	c.y = c1.y + c2.y;
	return c;
}
static API_Coord3D		Normalize(const API_Coord3D& v)

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
static double Dot_product(const API_Vector3D& v1, const API_Vector3D& v2) {
	double product = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return product;
}

static API_Vector3D	VectProd(const API_Vector3D& v1, const API_Vector3D& v2)

{
	API_Vector3D v;

	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;

	return v;
}

static double		operator* (const API_Coord3D& a, const API_Coord3D& b)

{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static API_Coord3D		operator* (const double& a, const API_Coord3D& b)

{
	return { a * b.x, a * b.y, a * b.z };
}

static double DistancePointToPlane(API_Vector3D normal, API_Coord3D pointOfPlane, API_Coord3D point)
{
	auto vector = point - pointOfPlane;
	auto normalNormalize = Normalize(normal);
	return Dot_product(normalNormalize, vector);
}

static API_Coord3D ProjectOnto(API_Coord3D normal, API_Coord3D pointOfPlane, API_Coord3D point) {
	double d = DistancePointToPlane(normal, pointOfPlane, point);
	API_Coord3D q= point- d * Normalize(normal);
	return q;
}

static float Mag(API_Coord3D a)  //calculates magnitude of a
{
	return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}
static float AngleTwoVector(API_Coord3D v1, API_Coord3D v2) {
	if(std::round(Mag(v1)*1000000)==0|| std::round(Mag(v2) * 1000000)==0) return 0;
	float angle = std::acos(Dot_product(v1, v2) / (Mag(v1) * Mag(v2)));
	return angle;
}

static float AngleVectorClockwise(API_Coord3D v1, API_Coord3D v2) {
	auto x1 = v1.x;
	auto x2 = v2.x;
	auto y1 = v1.y;
	auto y2 = v2.y;
	auto z1 = v1.z;
	auto z2 = v2.z;
	auto dot = x1 * x2 + y1 * y2 + z1 * z2;
	auto	cross_x = (y1 * z2 - z1 * y2);
	auto	cross_y = (z1 * x2 - x1 * z2);
	auto	cross_z = (x1 * y2 - y1 * x2);
	auto	det = sqrt(cross_x * cross_x + cross_y * cross_y + cross_z * cross_z);
	auto	angle = atan2(det, dot);
	return angle;
}
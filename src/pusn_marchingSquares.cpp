#include "pusn_marchingSquares.h"

using namespace pusn;

MarchingSquares::MarchingSquares(VertexPosNormal* cubeVertices, int materialWidth, int materialHeight, float radius, float offsetZ)
{
	this->cubeVertices = cubeVertices;
	this->materialWidth = materialWidth;
	this->materialHeight = materialHeight;
	this->radius = radius;
	this->offsetZ = offsetZ;
}

pusn::VertexPosNormal& MarchingSquares::GetHeightMapValue(int x, int y)
{
	int index = materialWidth * y + x;
	return cubeVertices[index];
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int MarchingSquares::orientation(Point p, Point q, Point r)
{
	float val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0;  // colinear
	return (val > 0) ? 1 : 2; // clock or counterclock wise
}

float MarchingSquares::cross(const Point &O, const Point &A, const Point &B)
{
	return (long)(A.x - O.x) * (B.y - O.y) - (long)(A.y - O.y) * (B.x - O.x);
}

// Prints convex hull of a set of n points.
vector<XMFLOAT2> MarchingSquares::convexHull(vector<XMFLOAT2> points, int n)
{
	int k = 0;
	vector<Point> H(2 * n);

	vector<Point> ps;

	for (int i = 0; i < n; i++)
	{
		Point po;
		po.x = points[i].x;
		po.y = points[i].y;
		ps.push_back(po);
	}

	sort(ps.begin(), ps.end());

	
	// Build lower hull
	for (int i = 0; i < n; ++i) {
		while (k >= 2 && cross(H[k - 2], H[k - 1], ps[i]) <= 0) k--;
		H[k++] = ps[i];
	}

	// Build upper hull
	for (int i = n - 2, t = k + 1; i >= 0; i--) {
		while (k >= t && cross(H[k - 2], H[k - 1], ps[i]) <= 0) k--;
		H[k++] = ps[i];
	}

	H.resize(k);


	int index = 0;
	//XMFLOAT2* orderedPoints = new XMFLOAT2[n];
	vector<XMFLOAT2> orderedPoints;
	for (int i = 0; i < H.size(); i++)
	{
			XMFLOAT2 vert;
			vert.x = H[i].x;
			vert.y = H[i].y;
			orderedPoints.push_back(vert);
	}
	//delete[] points;
	//points = orderedPoints;
	return orderedPoints;
}

bool MarchingSquares::FindContour(XMFLOAT2& origin, XMFLOAT2& first, XMFLOAT2& second)
{
	int x0 = origin.x;
	int y0 = origin.y;

	int tolerance = 1.0f;

	float a = /*sqrtf(2) * */radius + tolerance;
	float r = radius + tolerance;

	bool p1 = GetHeightMapValue(x0, y0 + 1).Pos.y > offsetZ;
	bool p2 = GetHeightMapValue(x0 + 1, y0 + 1).Pos.y > offsetZ;
	bool p3 = GetHeightMapValue(x0 + 1, y0).Pos.y > offsetZ;
	bool p4 = GetHeightMapValue(x0, y0).Pos.y > offsetZ;

	//1
	if (p1 && !p2 && !p3 && !p4)
	{
		first.x = x0;
		first.y = y0 + 1 - a;
		second.x = x0 + a;
		second.y = y0 + 1;
		return true ;
	}

	//2
	if (!p1 && p2 && !p3 && !p4)
	{
		first.x = x0 + 1 - a;
		first.y = y0 + 1;
		second.x = x0 + 1;
		second.y = y0 + 1 - a;
		return true;
	}

	//3
	if (p1 && p2 && !p3 && !p4)
	{
		first.x = x0;
		first.y = y0 + 1 - r;
		second.x = x0 + 1;
		second.y = y0 + 1 - r;
		return true;
	}

	//4
	if (!p1 && !p2 && p3 && !p4)
	{
		first.x = x0 + 1 - a;
		first.y = y0;
		second.x = x0 + 1;
		second.y = y0 + a;
		return true;
	}

	//5
	if (p1 && !p2 && p3 && !p4)
	{
		first.x = x0 + a;
		first.y = y0 + 1;
		second.x = x0 + 1;
		second.y = y0 + a;
		int d = 5;
		return true;
	}

	//6
	if (!p1 && p2 && p3 && !p4)
	{
		first.x = x0 + 1 - r;
		first.y = y0;
		second.x = x0 + 1 - r;
		second.y = y0 + 1;
		return true;
	}

	//7
	if (p1 && p2 && p3 && !p4)
	{
		first.x = x0;
		first.y = y0 + 1 - a;
		second.x = x0 + 1 - a;
		second.y = y0;
		return true;
	}

	//8
	if (!p1 && !p2 && !p3 && p4)
	{
		first.x = x0;
		first.y = y0 + a;
		second.x = x0 + a;
		second.y = y0;
		return true;
	}

	//9
	if (p1 && !p2 && !p3 && p4)
	{
		first.x = x0 + r;
		first.y = y0;
		second.x = x0 + r;
		second.y = y0 + 1;
		return true;
	}

	//10
	if (!p1 && p2 && !p3 && p4)
	{
		first.x = x0;
		first.y = y0 + a;
		second.x = x0 + 1 - a;
		second.y = y0 + 1;
		int d = 5;
		return true;
	}

	//11
	if (p1 && p2 && !p3 && p4)
	{
		first.x = x0 + a;
		first.y = y0;
		second.x = x0 + 1;
		second.y = y0 + 1 - a;
		return true;
	}

	//12
	if (!p1 && !p2 && p3 && p4)
	{
		first.x = x0;
		first.y = y0 + r;
		second.x = x0 + 1;
		second.y = y0 + r;
		return true;
	}

	//13
	if (p1 && !p2 && p3 && p4)
	{
		first.x = x0 + a;
		first.y = y0 + 1;
		second.x = x0 + 1;
		second.y = y0 + a;
		return true;
	}

	//14
	if (!p1 && p2 && p3 && p4)
	{
		first.x = x0;
		first.y = y0 + a;
		second.x = x0 + 1 - a;
		second.y = y0 + 1;
		return true;
	}
	int d = 5;
	return false;
}


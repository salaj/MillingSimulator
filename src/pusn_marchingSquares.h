#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include <xnamath.h>
#include <vector>
#include <algorithm>
#include "pusn_vertices.h"

using namespace std;

namespace pusn{
	struct Point
	{
		float x;
		float y;
		bool operator <(const Point &p) const {
			return x < p.x || (x == p.x && y < p.y);
		}
	};


	class MarchingSquares {
	public:
		MarchingSquares(VertexPosNormal* cubeVertices, int materialWidth, int materialHeight, float radius, float offsetZ);
		VertexPosNormal& GetHeightMapValue(int x, int y);
		bool FindContour(XMFLOAT2& orgigin, XMFLOAT2& first, XMFLOAT2& second);
		vector<XMFLOAT2> convexHull(vector<XMFLOAT2> points, int n);
	private:
		int orientation(Point p, Point q, Point r);
		float cross(const Point &O, const Point &A, const Point &B);
		VertexPosNormal* cubeVertices;
		int materialWidth = 150;
		int materialHeight = 150;
		float radius;
		float offsetZ;
	};
}
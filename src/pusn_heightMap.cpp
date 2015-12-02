#include "pusn_heightMap.h"
using namespace std;
using namespace pusn;

const unsigned int HeightMap::VB_STRIDE = sizeof(pusn::VertexPosNormal);
const unsigned int HeightMap::VB_OFFSET = 0;

HeightMap::HeightMap(PusnService& service)
{
	m_service = service;
	Initialize();
}


HeightMap::HeightMap(const HeightMap& other)
{
}


HeightMap::~HeightMap()
{
	m_dbVertexes.reset();
}



void HeightMap::InitializeMap(int n, int m)
{
	// Create the structure to hold the height map data.
	m_heightMap = new XMFLOAT3[n * m];
	//m_lowMap = new XMFLOAT3[2 * (n + m) - 4];
	m_lowMap = new XMFLOAT3[n * m];

	int lowIndex = 0;
	// Read the image data into the height map.
	float map_width = def_map_len * map_x / def_map_number;
	float map_depth = def_map_len * map_z / def_map_number;
	float map_height = def_map_len * map_y / def_map_number;

	for (int j = 0; j<n; j++)
	{
		for (int i = 0; i<m; i++)
		{
			float x = (float)i / (float)(m - 1) * map_width - map_width / 2;
			
			float z = (float)j / (float)(n - 1) * map_depth - map_depth / 2;

			int index = (n * j) + i;

			m_heightMap[index].x = x;
			m_heightMap[index].y = map_height;
			m_heightMap[index].z = z;

			float low_heigth = -map_height;
			m_lowMap[lowIndex].x = x;
			m_lowMap[lowIndex].y = 0;
			m_lowMap[lowIndex].z = z;
			lowIndex++;
		}
	}
	int a = 10;
}

void HeightMap::probeSurface(BSplineSurface* surface, XMFLOAT2& min, XMFLOAT2& max, XMFLOAT2& parMin, XMFLOAT2& parMax)
{
	float big = 1000;
	min.x = big;
	min.y = big;
	max.x = -big;
	max.y = -big;

	parMin.x = big;
	parMin.y = big;
	parMax.x = -big;
	parMax.y = -big;
	int k = 100;

	for (int i = 0; i < k; i++)
	{
		bool increasing = false;
		bool decreasing = false;
		for (int j = 0; j < k; j++)
		{
			XMFLOAT4 val = surface->Q(i/(float)(k-1), j/(float)(k-1));


			if (val.x < min.x)
				min.x = val.x;
			if (val.y < min.y)
				min.y = val.y;
			if (val.x > max.x)
				max.x = val.x;
			if (val.y > max.y)
				max.y = val.y;

			/*if (val.z > 0 && (!decreasing || !increasing) )
			{
				if (val.x < min.x)
					min.x = val.x;
				if (val.y < min.y)
					min.y = val.y;
				if (val.x > max.x)
					max.x = val.x;
				if (val.y > max.y)
					max.y = val.y;

				float parX = i / (float)(k - 1);
				float parY = j / (float)(k - 1);


				if (parX < parMin.x)
					parMin.x = parX;
				if (parY < parMin.y)
					parMin.y = parY;
				if (parX > parMax.x)
					parMax.x = parX;
				if (parY > parMax.y)
					parMax.y = parY;

				increasing = true;
			}
			else
				decreasing = true;*/
		}
	}
	int a = 10;
}

void HeightMap::preparePrecisePoints(ModelsManager* m_modelsManager, BSplineSurface* surface, vector<XMFLOAT2>& collisions, vector<XMFLOAT3>& orderedPoints, int iter_u, int iter_v, float tolerance, bool forceStopOnCollisiondetected = false)
{
	//float v = 0.5;
	//int iter_u = 100;
	//int iter_v = 80;
	float factor = 0.7;
	float m_radius = 4.0 / map_x * 2;
	float offsetInNormalDirection = m_radius;
	float offsetZ = def_map_len * map_y / def_map_number;// 1.0f;
	float offsetV = 0.8;

	XMFLOAT4* collisionPoints = new XMFLOAT4[collisions.size()];
	for (int a = 0; a < collisions.size(); a++)
	{
		XMFLOAT4 collisionPosition = surface->Q(collisions[a].x, collisions[a].y);
		collisionPoints[a] = collisionPosition;
	}

	for (int i = 0; i < iter_u; i++)
	{
		float u = i / (float)(iter_u - 1);
		bool sentinel = false;
		XMFLOAT3 readyPos;
		for (int j = 0; j < iter_v - 1; j++)
		{
			float v;
			if (i % 2)
			{
				v = j / (float)(iter_v - 1) + offsetV;
				if (v > 1)
					v = v - 1;

				sentinel = false;
				XMFLOAT4 currentPosition = surface->Q(u, v);
				for (int a = 0; a < collisions.size(); a++)
				{
					if (forceStopOnCollisiondetected)
					{
						if ((collisionPoints[a].x - currentPosition.x) * (collisionPoints[a].x - currentPosition.x) +
							(collisionPoints[a].y - currentPosition.y) * (collisionPoints[a].y - currentPosition.y) +
							(collisionPoints[a].z + m_radius - currentPosition.z) * (collisionPoints[a].z + m_radius - currentPosition.z) < tolerance)
						{
							sentinel = true;
							break;
						}
					}
					if ((collisionPoints[a].x - currentPosition.x) * (collisionPoints[a].x - currentPosition.x) +
						(collisionPoints[a].y - currentPosition.y) * (collisionPoints[a].y - currentPosition.y) +
						(collisionPoints[a].z - currentPosition.z) * (collisionPoints[a].z - currentPosition.z) < tolerance)
					{
						sentinel = true;
						break;
					}
				}
				if (sentinel)
					break;
			}
			else
			{
				v = offsetV - j / (float)(iter_v - 1);
				if (v < 0)
					v = v + 1;

				sentinel = false;
				XMFLOAT4 currentPosition = surface->Q(u, v);
				for (int a = 0; a < collisions.size(); a++)
				{
					if (forceStopOnCollisiondetected)
					{
						if ((collisionPoints[a].x - currentPosition.x) * (collisionPoints[a].x - currentPosition.x) +
							(collisionPoints[a].y - currentPosition.y) * (collisionPoints[a].y - currentPosition.y) +
							(collisionPoints[a].z + m_radius - currentPosition.z) * (collisionPoints[a].z + m_radius - currentPosition.z) < tolerance)
						{
							sentinel = true;
							break;
						}
					}
					if ((collisionPoints[a].x - currentPosition.x) * (collisionPoints[a].x - currentPosition.x) +
						(collisionPoints[a].y - currentPosition.y) * (collisionPoints[a].y - currentPosition.y) +
						(collisionPoints[a].z - currentPosition.z) * (collisionPoints[a].z - currentPosition.z) < tolerance)
					{
						sentinel = true;
						break;
					}
				}
				if (sentinel)
					break;
			}
			XMFLOAT4 pos = surface->Q(u, v);
			XMFLOAT4 normal = surface->Q_normal(u, v);
			XMVECTOR p = XMVectorSet(pos.x, pos.y, pos.z, 1.0f);
			p = p * factor;
			XMVECTOR n = XMVectorSet(normal.x, normal.y, normal.z, 1.0f);
			p = p + n * offsetInNormalDirection;

			float x = XMVectorGetX(p);
			float y = XMVectorGetZ(p);
			float z = -XMVectorGetY(p);
			y -= m_radius;

			if (y < 0)
				continue;

			readyPos = XMFLOAT3(x, y, z);
			readyPos.y += offsetZ;
			orderedPoints.push_back(readyPos);

		}
		if (sentinel)
		{
			if (forceStopOnCollisiondetected)
			{
				break;
			}
			XMFLOAT3 upPos = readyPos;
			upPos.y += 0.3;
			orderedPoints.push_back(upPos);
			XMFLOAT3 backPos = upPos;
			backPos.z -= 0.5;
			orderedPoints.push_back(backPos);
		}
	}
}


void HeightMap::prepareEvolutePoints()
{

	int width = materialWidth;
	int height = materialHeight;
	float factor = 0.7;
	float m_radius = (6.0 - 1.0) / map_x * 2;
	float radius = m_radius * materialWidth / 2;
	float inverseFactor = 1.0;//1 / factor;
	float offsetZ = def_map_len * map_y / def_map_number;// 1.0f;
	///checking evolute points

	vector<XMFLOAT2> contour;

	for (int i = 1; i < materialHeight - 1; i++)
		for (int j = 1; j < materialHeight - 1; j++)
		{
			if (GetHeightMapValue(i, j).Pos.y == GetHeightMapValue(0, 0).Pos.y)
				continue;
			bool sentinel = false;
			for (int a = -1; a <= 1; a++)
				for (int b = -1; b <= 1; b++)
				{
					if ((a == 0 && b == 0)/*||
										  (a == -1 && b == -1) ||
										  (a == -1 && b == 1) ||
										  (a == 1 && b == -1) ||
										  (a == 1 && b == 1)*/)
										  continue;
					if (GetHeightMapValue(i + a, j + b).Pos.y == GetHeightMapValue(0, 0).Pos.y)
					{
						sentinel = true;
					}
				}
			if (sentinel)
			{
				GetHeightMapValue(i, j).Normal = XMFLOAT3(-1, -1, -1);
				contour.push_back(XMFLOAT2(i, j));
			}
		}


	MarchingSquares marchingSquares = MarchingSquares(cubeVertices, materialWidth, materialHeight, radius, offsetZ);
	vector<XMFLOAT2> contourPoints;

	bool contourMap[150][150];
	XMFLOAT2 current;
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			contourMap[i][j] = false;
	int foundContourPoints = 0;
	for (int i = 1; i < materialHeight - 1; i++)
		for (int j = 1; j < materialHeight - 1; j++)
		{
			XMFLOAT2 first;
			XMFLOAT2 second;
			XMFLOAT2 origin = XMFLOAT2(j, i);
			bool isContour = marchingSquares.FindContour(origin, first, second);
			if (isContour)
			{

				XMFLOAT3 firstPos = GetHeightMapValue(first.x, first.y).Pos;
				XMFLOAT3 secondPos = GetHeightMapValue(second.x, second.y).Pos;
				bool senFirst = true;
				bool senSecond = true;
				int tolerance = 0;
				for (int a = -radius + tolerance; a <= radius - tolerance; a++)
					for (int b = -radius + tolerance; b <= radius - tolerance; b++)
					{
						if (abs(a) + abs(b) > radius)
							continue;
						//NOT TOO CLOSE
						if (GetHeightMapValue(first.x + a, first.y + b).Pos.y > offsetZ)
							senFirst = false;
						if (GetHeightMapValue(second.x + a, second.y + b).Pos.y > offsetZ)
							senSecond = false;
					}

				bool senFirstFar = true;
				bool senSecondFar = true;

				//bool senFirstFar = false;
				//bool senSecondFar = false;
				//tolerance = 1;
				//for (int a = -radius + tolerance; a <= radius - tolerance; a++)
				//	for (int b = -radius + tolerance; b <= radius - tolerance; b++)
				//	{
				//		//NOT TOO FAR
				//		if (GetHeightMapValue(first.x + a, first.y + b).Pos.y > offsetZ)
				//			senFirstFar = true;
				//		if (GetHeightMapValue(second.x + a, second.y + b).Pos.y > offsetZ)
				//			senSecondFar = true;
				//	}

				//check if we do not cross heightMap
				if (firstPos.y == offsetZ && secondPos.y == offsetZ)
				{
					if (senFirst && senFirstFar)
					{
						GetHeightMapValue(first.x, first.y).Normal = XMFLOAT3(-1, -1, -1);
						contourPoints.push_back(first);
						contourMap[(int)first.x][(int)first.y] = true;
						current = first;
						foundContourPoints++;
					}
					if (senSecond && senSecondFar)
					{
						GetHeightMapValue(second.x, second.y).Normal = XMFLOAT3(-1, -1, -1);
						contourPoints.push_back(second);
						contourMap[(int)second.x][(int)second.y] = true;
						foundContourPoints++;
					}
				}
				else
				{
					int d = 5;
				}
			}
		}

	XMFLOAT2 firstOrigin = current;
	orderedContourPoints.push_back(firstOrigin);
	contourMap[(int)firstOrigin.x][(int)firstOrigin.y] = false;
	do
	{
		int tolerance = 40;
		int dist, minDist;
		minDist = 10000;
		XMFLOAT2 currentMin;
		bool changed = false;
		for (int i = current.x - tolerance; i <= current.x + tolerance; i++)
			for (int j = current.y - tolerance; j <= current.y + tolerance; j++)
			{
				if (i < 0 || j < 0 || i >= width || j >= height)
					continue;
				/*if (firstOrigin.x == current.x && firstOrigin.y == current.y && firstOrigin.x == i && firstOrigin.y == j)
				continue;*/

				if (contourMap[i][j])
				{
					dist = (i - current.x) * (i - current.x) + (j - current.y) * (j - current.y);
					if (dist < minDist)
					{
						minDist = dist;
						currentMin.x = i;
						currentMin.y = j;
						changed = true;
					}
				}
			}
		if (changed)
		{
			orderedContourPoints.push_back(currentMin);
			contourMap[(int)currentMin.x][(int)currentMin.y] = false;
			current = currentMin;
		}
		else
			break;
	} while (true/*foundContourPoints--*/);//while (firstOrigin.x != current.x || firstOrigin.y != current.y);

	//we want to close the evolute
	orderedContourPoints.push_back(firstOrigin);

	XMFLOAT2* points = new XMFLOAT2[contourPoints.size()];
	//copy from vector to array
	for (int i = 0; i < contourPoints.size(); i++)
	{
		points[i] = contourPoints[i];
	}
	//orderedContourPoints = marchingSquares.convexHull(contourPoints, contourPoints.size());

	//NULLIFY
	//for (int i = 1; i < materialHeight - 1; i++)
	//	for (int j = 1; j < materialHeight - 1; j++)
	//	{
	//		GetHeightMapValue(i, j).Pos.y = GetHeightMapValue(0, 0).Pos.y;
	//	}
}

static int zCounter = 0;

void HeightMap:: PrepareSurface(ModelsManager* m_modelsManager)
{
	float big = 1000;
	float** lens = new float*[materialWidth];
	for (int i = 0; i < materialWidth; i++)
	{
		lens[i] = new float[materialHeight];
		for (int j = 0; j < materialHeight; j++)
			lens[i][j] = big;
	}
	BSplineSurface* core;
	int width = materialWidth;
	int height = materialHeight;
	float factor = 0.7;
	float m_radius = 6.0 / map_x * 2;
	float radius = m_radius * materialWidth / 2;
	float inverseFactor = 1.0;//1 / factor;
	float offsetZ = def_map_len * map_y / def_map_number;// 1.0f;
	float tolerance = 0.05;
	if (m_modelsManager != nullptr)
	{
		int n = m_modelsManager->GetBSplineSurfaceCount();
		for (int m = 0; m < n; m++)
		{
			//wiert³o
			if (m == 3)
				continue;
			
			BSplineSurface* surface = m_modelsManager->PusnGetBSplineSurface(m);
			if (m == 0)
				core = surface;
			//XMFLOAT4 a = surface->Q(0, 0);
			//XMFLOAT4 b = surface->Q(0, 0.2);


			XMFLOAT2 min, max, parMin, parMax;
			probeSurface(surface, min, max, parMin, parMax);

			int x_start = width / 2 * (min.x * factor + 1) * inverseFactor;
			int x_end = width / 2 * (max.x * factor + 1) * inverseFactor;
			int x_len = x_end - x_start;
			int x_offset = 0.4;//(materialWidth - x_len) / 2;

			int y_start = height / 2 * (min.y * factor + 1) * inverseFactor;
			int y_end = height / 2 * (max.y * factor + 1) * inverseFactor;
			int y_len = y_end - y_start;
			int y_offset = 0.4;//(materialHeight - y_len) / 2;

			x_len = y_len = 300;
			for (int i = 0; i < x_len; i++)
				for (int j = 0; j < y_len; j++)
				{
					XMFLOAT4 a = surface->Q(i / (float)(x_len - 1), j / (float)(y_len - 1));
					float z = (a.z  * factor * inverseFactor + offsetZ);
					float x_origin = (width / 2 * (a.x * factor + 1) * inverseFactor);
					float y_origin = (height / 2 * (a.y * factor + 1) * inverseFactor);
					int x = roundf(x_origin); //+ x_offset;
					int y = roundf(y_origin);// -y_offset;
					float len = (x  - x_origin) * (x - x_origin) + (y  - y_origin)*(y - y_origin);
					
					if (GetHeightMapValue(x, height - y).Pos.y < z)
						if (GetHeightMapValue(x, height - y).Pos.y == offsetZ)
							GetHeightMapValue(x, height - y).Pos.y = z;
						/*else
						GetHeightMapValue(x, height - y).Pos.y = (z + GetHeightMapValue(x, height - y).Pos.y ) / (float)2;*/
					int k = 10;

					//if (abs(z - offsetZ) < 0.005)
					//{
					//	zCounter++;
					//	XMVECTOR aV = XMVectorSet(a.x, a.y, a.z, 1.0f);
					//	XMFLOAT4 b = core->Q_derivative(i / (float)(x_len - 1), j / (float)(y_len - 1));
					//	XMVECTOR bV = XMVectorSet(b.x, b.y, b.z, 1.0f) * 0.2;
					//	//bV = XMVector3Normalize(bV);
					//	XMFLOAT3 c = XMFLOAT3(
					//		XMVectorGetX(aV + aV*bV),
					//		XMVectorGetY(aV + aV*bV),
					//		XMVectorGetZ(aV + aV*bV) //this might be null
					//		);
					//	int x_origin_c = (width / 2 * (c.x * factor + 1) * inverseFactor);
					//	int y_origin_c = (height / 2 * (c.y * factor + 1) * inverseFactor);
					//	int x_c = roundf(x_origin_c); //+ x_offset;
					//	int y_c = roundf(y_origin_c);// -y_offset;
					//	if (GetHeightMapValue(x_c, height - y_c).Pos.y > GetHeightMapValue(x, height - y).Pos.y)
					//	{
					//		c = XMFLOAT3(
					//			XMVectorGetX(aV - aV*bV),
					//			XMVectorGetY(aV - aV*bV),
					//			XMVectorGetZ(aV - aV*bV) //this might be null
					//			);
					//	}
					//	x_origin_c = (width / 2 * (c.x * factor + 1) * inverseFactor);
					//	y_origin_c = (height / 2 * (c.y * factor + 1) * inverseFactor);
					//	x_c = roundf(x_origin_c); //+ x_offset;
					//	y_c = roundf(y_origin_c);// -y_offset;

					//	GetHeightMapValue(x_c, height - y_c).Normal = XMFLOAT3(-1, -1, -1);
					//}
				}
			//for (int b = 0; b < 100; b++)
			//fixSurfaceHoles( x_len,  y_len,  x_start,  y_start,  x_offset, y_offset);
		}

		prepareEvolutePoints();
		
		IntersectionSurfaceBSpline* intersectionBSpline = m_modelsManager->PusnGetBSplineIntersectionSurface(0);
		int countNormal = intersectionBSpline->GetCountOfData();
		for (int i = 0; i < countNormal; i++)
		{
			float u = intersectionBSpline->GetParametrization(i, 0);
			float v = intersectionBSpline->GetParametrization(i, 1);

			float s = intersectionBSpline->GetParametrization(i, 2);
			float t = intersectionBSpline->GetParametrization(i, 3);

			XMFLOAT2 dangerPoint = XMFLOAT2(u, v);
			collisionPoints.push_back(dangerPoint);
			collisionPointsBackHandle.push_back(XMFLOAT2(s, t));
		}
		int countNormalReversed = intersectionBSpline->GetCountOfDataReversed();
		for (int i = 0; i < countNormalReversed; i++)
		{
			float u = intersectionBSpline->GetParametrizationReversed(i, 0);
			float v = intersectionBSpline->GetParametrizationReversed(i, 1);

			float s = intersectionBSpline->GetParametrizationReversed(i, 2);
			float t = intersectionBSpline->GetParametrizationReversed(i, 3);

			XMFLOAT2 dangerPoint = XMFLOAT2(u, v);
			collisionPoints.push_back(dangerPoint);
			collisionPointsBackHandle.push_back(XMFLOAT2(s, t));
		}

		intersectionBSpline = m_modelsManager->PusnGetBSplineIntersectionSurface(1);
		countNormal = intersectionBSpline->GetCountOfData();
		for (int i = 0; i < countNormal; i++)
		{
			float u = intersectionBSpline->GetParametrization(i, 0);
			float v = intersectionBSpline->GetParametrization(i, 1);

			float s = intersectionBSpline->GetParametrization(i, 2);
			float t = intersectionBSpline->GetParametrization(i, 3);

			XMFLOAT2 dangerPoint = XMFLOAT2(u, v);
			collisionPoints.push_back(dangerPoint);
			collisionPointsFrontHandle.push_back(XMFLOAT2(s, t));
		}
		countNormalReversed = intersectionBSpline->GetCountOfDataReversed();
		for (int i = 0; i < countNormalReversed; i++)
		{
			float u = intersectionBSpline->GetParametrizationReversed(i, 0);
			float v = intersectionBSpline->GetParametrizationReversed(i, 1);

			float s = intersectionBSpline->GetParametrizationReversed(i, 2);
			float t = intersectionBSpline->GetParametrizationReversed(i, 3);

			XMFLOAT2 dangerPoint = XMFLOAT2(u, v);
			collisionPoints.push_back(dangerPoint);
			collisionPointsFrontHandle.push_back(XMFLOAT2(s, t));
		}

		tolerance = 0.001;
		BSplineSurface* surface = m_modelsManager->PusnGetBSplineSurface(0);
		preparePrecisePoints(m_modelsManager, surface, collisionPoints, orderedPrecisePoints, 150, 80, tolerance);
		surface = m_modelsManager->PusnGetBSplineSurface(1);
		tolerance = 0.005;
		preparePrecisePoints(m_modelsManager, surface, collisionPointsBackHandle, orderedPrecisePointsBackHandle, 50, 50, tolerance, true);
		surface = m_modelsManager->PusnGetBSplineSurface(2);
		tolerance = 0.005;
		preparePrecisePoints(m_modelsManager, surface, collisionPointsFrontHandle, orderedPrecisePointsFrontHandle, 50, 50, tolerance, true);
		//checkIntersections(intersectionBSpline, core);
		//intersectionBSpline = m_modelsManager->PusnGetBSplineIntersectionSurface(1);
		//checkIntersections(intersectionBSpline, core);
		//intersectionBSpline = m_modelsManager->PusnGetBSplineIntersectionSurface(2);
		//checkIntersections(intersectionBSpline, core);
	}

	for (int i = 0; i < materialWidth; i++)
	{
		delete[] lens[i];
	}
	int c = 5;
}

void HeightMap::checkIntersections(IntersectionSurfaceBSpline* intersectionBSpline, BSplineSurface* core)
{
	int width = materialWidth;
	int height = materialHeight;
	float factor = 0.7;
	float m_radius = 6.0 / map_x * 2;
	float offsetInNormalDirection = m_radius;
	float inverseFactor = 1.0;//1 / factor;
	float offsetZ = def_map_len * map_y / def_map_number; //1.0f;
	//INTERSECTION CURVES
	int countNormal = intersectionBSpline->GetCountOfData();
	for (int i = 0; i < countNormal; i++)
	{
		XMVECTOR normal = intersectionBSpline->GetNormals(i);
		float u = intersectionBSpline->GetParametrization(i, 0);
		float v = intersectionBSpline->GetParametrization(i, 1);

		float s = intersectionBSpline->GetParametrization(i, 2);
		float t = intersectionBSpline->GetParametrization(i, 3);
		XMFLOAT4 a = core->Q(s, t);
		float x_origin = (width / 2 * (a.x * factor + 1) * inverseFactor);
		float y_origin = (height / 2 * (a.y * factor + 1) * inverseFactor);
		int x = roundf(x_origin); //+ x_offset;
		int y = roundf(y_origin);// -y_offset;

		a = core->Q(u, v);
		XMVECTOR aV = XMVectorSet(a.x, a.y, a.z, 1.0f);
		XMFLOAT4 b = core->Q_normal(u, v);
		XMVECTOR bV = XMVectorSet(b.x, b.y, b.z, 1.0f) * offsetInNormalDirection;
		XMFLOAT3 c = XMFLOAT3(
			XMVectorGetX(aV + aV*bV),
			XMVectorGetY(aV + aV*bV),
			XMVectorGetZ(aV + aV*bV)
			);
		x_origin = (width / 2 * (c.x * factor + 1) * inverseFactor);
		y_origin = (height / 2 * (c.y * factor + 1) * inverseFactor);
		x = roundf(x_origin); //+ x_offset;
		y = roundf(y_origin);// -y_offset;
		GetHeightMapValue(x, materialHeight - y).Normal = XMFLOAT3(-1, -1, -1);
		int d = 10;
	}


	int countReversed = intersectionBSpline->GetCountOfDataReversed();
	for (int i = 0; i < countReversed; i++)
	{
		XMVECTOR normal = intersectionBSpline->GetNormalsReversed(i);
		float u = intersectionBSpline->GetParametrizationReversed(i, 0);
		float v = intersectionBSpline->GetParametrizationReversed(i, 1);

		float s = intersectionBSpline->GetParametrizationReversed(i, 2);
		float t = intersectionBSpline->GetParametrizationReversed(i, 3);
		XMFLOAT4 a = core->Q(s, t);
		float x_origin = (width / 2 * (a.x * factor + 1) * inverseFactor);
		float y_origin = (height / 2 * (a.y * factor + 1) * inverseFactor);
		int x = roundf(x_origin); //+ x_offset;
		int y = roundf(y_origin);// -y_offset;
		a = core->Q(u, v);
		XMVECTOR aV = XMVectorSet(a.x, a.y, a.z, 1.0f);
		XMFLOAT4 b = core->Q_normal(u, v);
		XMVECTOR bV = XMVectorSet(b.x, b.y, b.z, 1.0f) * offsetInNormalDirection;
		//bV = XMVector3Normalize(bV);
		XMFLOAT3 c = XMFLOAT3(
			XMVectorGetX(aV + aV*bV),
			XMVectorGetY(aV + aV*bV),
			XMVectorGetZ(aV + aV*bV)
			);
		x_origin = (width / 2 * (c.x * factor + 1) * inverseFactor);
		y_origin = (height / 2 * (c.y * factor + 1) * inverseFactor);
		x = roundf(x_origin); //+ x_offset;
		y = roundf(y_origin);// -y_offset;
		GetHeightMapValue(x, materialHeight - y).Normal = XMFLOAT3(-1, -1, -1);
		int d = 10;
	}
}

void HeightMap::fixSurfaceHoles(int x_len, int y_len, int x_start, int y_start, float x_offset, float y_offset)
{

	for (int i = 0; i < x_len; i++)
		for (int j = 0; j < y_len; j++)
		{
			int x = i + x_start; //+ x_offset;
			int y = j + y_start; //- y_offset;
			if (GetHeightMapValue(x, y).Pos.y <= 1)
			{
				float sr = 0;
				int count = 0;
				for (int n = -1; n <= 1; n++)
					for (int m = -1; m <= 1; m++)
					{
						if (GetHeightMapValue((x + n), (y + m)).Pos.y > 1)
						{
							sr += GetHeightMapValue((x + n), (y + m)).Pos.y;
							count++;
						}
					}
				if (count!=0)
				{
					int d = 1;
					GetHeightMapValue(x, y).Pos.y = sr / count;
				}

				/*float val_x_prev = GetHeightMapValue((x - 1), (y)).Pos.y;
				float val_x = GetHeightMapValue((x), (y)).Pos.y;
				float val_x_next = GetHeightMapValue((x + 1), (y)).Pos.y;
				float val_y_prev = GetHeightMapValue((x), (y - 1)).Pos.y;
				float val_y = GetHeightMapValue((x), (y)).Pos.y;
				float val_y_next = GetHeightMapValue((x), (y + 1 )).Pos.y;


				if (val_x_prev > val_x)
				{
					if (val_x_next > val_x)
					{
						GetHeightMapValue(x, y).Pos.y = (val_x_prev + val_x_next) / 2;
						continue;
					}
				}
				else
				{
					if (val_y_prev > val_y)
					{
						if (val_y_next > val_y)
							GetHeightMapValue(x, y).Pos.y = (val_y_prev + val_y_next) / 2;
					}
				}*/
			}
		}



	
	for (int j = 0; j < y_len; j++)
		for (int i = 0; i < x_len; i++)
		{
			int x = i + x_start; //+ x_offset;
			int y = j + y_start; //- y_offset;
			if (GetHeightMapValue(x, y).Pos.y <= 1)
			{

				float val_x_prev = GetHeightMapValue((x - 1), (y)).Pos.y;
				float val_x = GetHeightMapValue((x), (y)).Pos.y;
				float val_x_next = GetHeightMapValue((x + 1), (y)).Pos.y;
				float val_y_prev = GetHeightMapValue((x), (y - 1)).Pos.y;
				float val_y = GetHeightMapValue((x), (y)).Pos.y;
				float val_y_next = GetHeightMapValue((x), (y + 1)).Pos.y;


				if (val_x_prev > val_x)
				{
					if (val_x_next > val_x)
					{
						GetHeightMapValue(x, y).Pos.y = (val_x_prev + val_x_next) / 2;
						continue;
					}
				}
				else
				{
					if (val_y_prev > val_y)
					{
						if (val_y_next > val_y)
						{
							GetHeightMapValue(x, y).Pos.y = (val_y_prev + val_y_next) / 2;
							continue;
						}
					}
				}
				int d = 1;
			}
		}
}

void HeightMap::Initialize()
{
	InitializeMap(materialWidth, materialHeight);

	int verticesCount = materialWidth * materialHeight * 2;
	indicesMeshCount = (materialWidth * materialHeight + 4 * materialHeight) * 12;
	indicesMaterialCount = materialWidth * materialHeight * 6;

	cubeVertices = new VertexPosNormal[verticesCount];
	activeVertices = new bool[materialWidth * materialHeight];

	cubeIndicesMesh = new unsigned short[indicesMeshCount];
	cubeIndicesMaterial = new unsigned short[indicesMaterialCount];

	// Initialize the index to the vertex buffer.
	int index = 0;

	for (int j = 0; j < materialHeight; j++)
	{
		for (int i = 0; i < materialWidth; i++)
		{
			cubeVertices[index].Pos = m_heightMap[index];
			cubeVertices[index].Normal = XMFLOAT3(0.0, 1.0, 0.0);
			index++;
		}
	}

	int k = 0;

	for (int j = 0; j < materialHeight; j++)
	{
		for (int i = 0; i < materialWidth; i++)
		{
			cubeVertices[index].Pos = m_lowMap[k++];
			cubeVertices[index].Normal = XMFLOAT3(0.0, 1.0, 0.0);
			index++;
		}
	}

	index = 0;
	for (int j = 0; j < (materialHeight - 1); j++)
	{
		for (int i = 0; i < (materialWidth - 1); i++)
		{
			int	index1 = (materialHeight * j) + i;          // Bottom left.
			int index2 = (materialHeight * j) + (i + 1);      // Bottom right.
			int index3 = (materialHeight * (j + 1)) + i;      // Upper left.
			int index4 = (materialHeight * (j + 1)) + (i + 1);  // Upper right.

			// Upper left.
			cubeIndicesMaterial[index] = index1;
			index++;

			// Upper Right.
			cubeIndicesMaterial[index] = index4;
			index++;

			// Bottom left.
			cubeIndicesMaterial[index] = index3;
			index++;


			// Upper Right.
			cubeIndicesMaterial[index] = index1;
			index++;

			// Bottom right.
			cubeIndicesMaterial[index] = index2;
			index++;

			// Bottom left.
			cubeIndicesMaterial[index] = index4;
			index++;
		}
	}

	index = 0;
	// Load Mesh
	for (int j = 0; j < (materialHeight - 1); j++)
	{
		for (int i = 0; i < (materialWidth - 1); i++)
		{
			int	index1 = (materialHeight * j) + i;          // Bottom left.
			int index2 = (materialHeight * j) + (i + 1);      // Bottom right.
			int index3 = (materialHeight * (j + 1)) + i;      // Upper left.
			int index4 = (materialHeight * (j + 1)) + (i + 1);  // Upper right.

			// Upper left.
			cubeIndicesMesh[index] = index3;
			index++;

			// Upper right.
			cubeIndicesMesh[index] = index4;
			index++;

			// Upper right.
			cubeIndicesMesh[index] = index4;
			index++;

			// Bottom left
			cubeIndicesMesh[index] = index1;
			index++;

			// Bottom left.
			cubeIndicesMesh[index] = index1;
			index++;

			// Upper left.
			cubeIndicesMesh[index] = index3;
			index++;

			// Bottom left.
			cubeIndicesMesh[index] = index1;
			index++;

			// Upper right.
			cubeIndicesMesh[index] = index4;
			index++;

			// Upper right.
			cubeIndicesMesh[index] = index4;
			index++;

			// Bottom right.
			cubeIndicesMesh[index] = index2;
			index++;

			// Bottom right.
			cubeIndicesMesh[index] = index2;
			index++;

			// Bottom left.
			cubeIndicesMesh[index] = index1;
			index++;
		}
	}

	//index = 0;
	for (int j = 0; j < (materialHeight - 1); j++)
	{
		for (int i = 0; i < (materialWidth - 1); i++)
		{
			if (j != 0 && j != materialHeight - 2 && i != 0 && i != materialWidth - 2)
				continue;
			int index1, index2, index3, index4;
			if (j == 0)
			{
				//int index1 = (materialHeight * j) + i;          // Bottom left.
				//int index2 = (materialHeight * j) + (i + 1);      // Bottom right.
				//int index3 = (materialHeight * (j + 1)) + i;      // Upper left.
				//int index4 = (materialHeight * (j + 1)) + (i + 1);  // Upper right.

				index1 = (materialHeight * j) + i;          // Bottom left.
				index2 = (materialHeight * j) + (i + 1);      // Bottom right.
				index3 = index1 + materialHeight * materialWidth;      // Upper left.
				index4 = index2 + materialHeight * materialWidth;   // Upper right.
				fillIndices(index, index1, index2, index3, index4);
			}
			if (j == materialHeight - 2)
			{
				index3 = (materialHeight * (j + 1)) + i;      // Upper left.
				index4 = (materialHeight * (j + 1)) + (i + 1);  // Upper right.
				index1 = index3 + materialHeight * materialWidth;          // Bottom left.
				index2 = index4 + materialHeight * materialWidth;      // Bottom right.
				fillIndices(index, index1, index2, index3, index4);
			}
			if (i == 0)
			{
				index3 = (materialHeight * (j + 1)) + i;      // Upper left.
				index4 = (materialHeight * j) + i; //Upper right
				index1 = index3 + materialHeight * materialWidth; //Bottom left
				index2 = index4 + materialHeight * materialWidth; //Bottom right
				fillIndices(index, index1, index2, index3, index4);
			}
			if (i == materialWidth - 2)
			{
				index4 = (materialHeight * (j + 1)) + i + 1;    // Upper right.
				index3 = (materialHeight * j) + i + 1; //Upper left
				index1 = index3 + materialHeight * materialWidth; //Bottom left
				index2 = index4 + materialHeight * materialWidth; //Bottom right.
				fillIndices(index, index1, index2, index3, index4);
			}

		}
	}


	//int dataCount = ARRAYSIZE(triangleVertices);

	m_dbVertexes.reset(new DBVertexes(m_service.Device, verticesCount));
	indicesPointerMesh = m_service.Device.CreateIndexBuffer(cubeIndicesMesh, indicesMeshCount);
	indicesPointerMaterial = m_service.Device.CreateIndexBuffer(cubeIndicesMaterial, indicesMaterialCount);
	delete[] cubeIndicesMesh;
	delete[] cubeIndicesMaterial;
}

void HeightMap::Draw(bool isMaterialActive)
{
	m_service.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//const XMMATRIX worldMtx = XMMatrixIdentity();
	//m_cbWorld->Update(m_context, worldMtx);

	m_dbVertexes->Update(m_service.Context, cubeVertices);
	ID3D11Buffer* b = m_dbVertexes->getBufferObject().get();
	m_service.Context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_service.Context->IASetIndexBuffer(indicesPointerMesh.get(), DXGI_FORMAT_R16_UINT, 0);
	if (!isMaterialActive)
		m_service.Context->DrawIndexed(indicesMeshCount, 0, 0);

	m_service.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_service.Context->IASetIndexBuffer(indicesPointerMaterial.get(), DXGI_FORMAT_R16_UINT, 0);
	if (isMaterialActive)
		m_service.Context->DrawIndexed(indicesMaterialCount, 0, 0);
}


pusn::VertexPosNormal& HeightMap::GetHeightMapValue(int x, int y)
{
	int index = materialWidth * y + x;
	return cubeVertices[index];
}

bool& HeightMap::GetVerticeActiveValue(int x, int y)
{
	int index = materialWidth * y + x;
	return activeVertices[index];
}



void HeightMap::fillIndices(int &index, int index1, int index2, int index3, int index4)
{

	// Upper left.
	cubeIndicesMesh[index] = index3;
	index++;

	// Upper right.
	cubeIndicesMesh[index] = index4;
	index++;

	// Upper right.
	cubeIndicesMesh[index] = index4;
	index++;

	// Bottom left
	cubeIndicesMesh[index] = index1;
	index++;

	// Bottom left.
	cubeIndicesMesh[index] = index1;
	index++;

	// Upper left.
	cubeIndicesMesh[index] = index3;
	index++;

	// Bottom left.
	cubeIndicesMesh[index] = index1;
	index++;

	// Upper right.
	cubeIndicesMesh[index] = index4;
	index++;

	// Upper right.
	cubeIndicesMesh[index] = index4;
	index++;

	// Bottom right.
	cubeIndicesMesh[index] = index2;
	index++;

	// Bottom right.
	cubeIndicesMesh[index] = index2;
	index++;

	// Bottom left.
	cubeIndicesMesh[index] = index1;
	index++;
}
#include "pusn_miller.h"

using namespace pusn;


Miller::Miller(PusnService &service) : PusnModelClass(service)
{
	//m_Type = ModelType::CursorType;
	PusnModelClass::Initialize();
	Miller::Initialize();
}


Miller::Miller(const Miller& other) : PusnModelClass(other)
{
}


Miller::~Miller()
{
	PusnModelClass::~PusnModelClass();
}

void Miller :: SetupPosition(XMFLOAT3 startMillerPosition, XMFLOAT3 endMillerPosition)
{
	StartMillerPosition = startMillerPosition;
	EndMillerPosition = endMillerPosition;

	//float val = m_heightMap->map_y;

	//StartMillerPosition.z = val - StartMillerPosition.z;
	//EndMillerPosition.z = val - EndMillerPosition.z;

	float factor = m_heightMap ->map_x / 2;

	StartMillerPosition.x /= factor;
	StartMillerPosition.y /= (factor);
	StartMillerPosition.z /= (factor);

	EndMillerPosition.x /= factor;
	EndMillerPosition.y /= (factor);
	EndMillerPosition.z /= (factor);

	if (type == millerType::Sphere)
	{
		StartMillerPosition.z += (m_radius);
		EndMillerPosition.z += (m_radius);
	}

	SetPosition(StartMillerPosition.x, StartMillerPosition.z, -StartMillerPosition.y);
}

void Miller::Reset(HeightMap* heightMap)
{
	m_heightMap = heightMap;
	m_heightMap->activePoints.clear();
	//for (int i = 0; i < m_heightMap->materialHeight; i++)
	//	for (int j = 0; j < m_heightMap->materialWidth; j++)
	//	{
	//		XMFLOAT2* index = new XMFLOAT2(i, j);
	//		updateNormals(index);
	//		delete index;
	//	}
}

void Miller::UpdateActivePoints()
{
		//przeszukaj wszystkie
		for (int j = 0; j < m_heightMap->materialHeight; j++)
		{
			for (int i = 0; i < m_heightMap->materialWidth; i++)
			{
				XMFLOAT3 p = m_heightMap->GetHeightMapValue(j, i).Pos;
				XMVECTOR pos = XMVectorSet(p.x, p.y, p.z, 1);
				bool inside = CheckIfPointIsInside(pos);
				if (inside)
				{
					XMFLOAT2 *newPoint = new XMFLOAT2(j, i);
					m_heightMap->activePoints.insert(pair<XMFLOAT2*, XMFLOAT2*>(newPoint, newPoint));
					m_heightMap->GetVerticeActiveValue(j, i) = true;
				}
			}
		}
	//ustaw pozycjê pocz¹tkow¹ i koñcow¹
	prepareBresenham(StartMillerPosition.x, StartMillerPosition.y, EndMillerPosition.x, EndMillerPosition.y);
	calculateBresenham();
}

bool Miller::CheckIfPointIsInside(XMVECTOR& pos)
{
	XMVECTOR miller_center = XMVectorSet(GetPosition3().x, GetPosition3().y, GetPosition3().z, 1);
	XMVECTOR len = (pos - miller_center) * (pos - miller_center);
	float abs = 0;
	float l = XMVectorGetX(XMVector3Length(len));
	float diff = l - m_radius * m_radius;

	if (type == millerType::Sphere)
	{
		if (l < (m_radius * m_radius - abs))
		{
			calculateOffset(pos, miller_center);
			return true;
		}
		//check if not above the drill
		XMVECTOR modPos = XMVECTOR(pos);
		modPos = XMVectorSetY(pos, XMVectorGetY(miller_center));
		float length = XMVectorGetX(XMVector3LengthSq(modPos - miller_center));
		if (length < m_radius * m_radius)
		{
			calculateOffset(pos, miller_center);
			return true;
		}
	}
	else
	{
		XMVECTOR modPos = XMVECTOR(pos);
		modPos = XMVectorSetY(modPos, XMVectorGetY(miller_center));
		float length = XMVectorGetX(XMVector3LengthSq(modPos - miller_center));
		if (length <= m_radius * m_radius)
		{
			if (XMVectorGetY(pos) >= XMVectorGetY(modPos))
			{
				//if (isMovingVertically)
				//	MessageBox(NULL, L"Frez p³aski próbuje wycinaæ w dó³.", NULL, NULL);
				pos = modPos;
				return true;
			}
		}
	}

	return false;
}

void Miller::prepareBresenham(float x0, float y0, float x1, float y1)
{

	x0 = (x0 * m_heightMap->materialWidth / 2);
	y0 = (y0 * m_heightMap->materialHeight / 2);
	x1 = (x1 * m_heightMap->materialWidth / 2);
	y1 = (y1 * m_heightMap->materialHeight / 2);

	//float cellSize = 2 / (float)m_heightMap->materialWidth;

	dx = abs(x1 - x0), sx = x0 < x1 ? millerStep : -millerStep;
	dy = -abs(y1 - y0), sy = y0 < y1 ? millerStep : -millerStep;
	err = dx + dy, e2; /* error value e_xy */

	start_x = x0;
	start_y = y0;
	end_x = x1;
	end_y = y1;
}
void Miller::calculateNormal(XMFLOAT2& index, XMFLOAT2& index1, XMFLOAT2& index2, XMVECTOR& f, int& counter)
{
	int maxX = m_heightMap->materialWidth - 1;
	int maxY = m_heightMap->materialWidth - 1;
	int minX = 0;
	int minY = 0;

	if (index.x >= minX && index.x <= maxX &&
		index1.x >= minX && index1.x <= maxX &&
		index2.x >= minX && index2.x <= maxX &&
		index.y >= minY && index.y <= maxY &&
		index1.y >= minY && index1.y <= maxY &&
		index2.y >= minY && index2.y <= maxY)
		counter++;
	else
		return;

	XMFLOAT3 pos, pos1, pos2;
	XMVECTOR p1, p2, p3;


	pos = m_heightMap->GetHeightMapValue(index.x, index.y).Pos;
	pos1 = m_heightMap->GetHeightMapValue(index1.x, index1.y).Pos;
	pos2 = m_heightMap->GetHeightMapValue(index2.x, index2.y).Pos;

	p1 = XMVectorSet(pos.x, pos.y, pos.z, 1);
	p2 = XMVectorSet(pos1.x, pos1.y, pos1.z, 1);
	p3 = XMVectorSet(pos2.x, pos2.y, pos2.z, 1);
	XMVECTOR u = p2 - p1;
	XMVECTOR v = p3 - p1;
	XMVECTOR n = XMVector3Cross(u, v);

	f = f + n;
}


void  Miller::updateNormals(XMFLOAT2* index)
{

	int c = 1;

	int x = index->x;
	int y = index->y;

	int counter = 0;
	XMFLOAT2 index1, index2;
	XMVECTOR f = XMVectorZero();

	//1
	index1.x = x;
	index1.y = y + 1;

	index2.x = x + 1;
	index2.y = y;
	
	calculateNormal(*index, index1, index2, f, counter);

	//2
	index1 = index2;

	index2.x = x;
	index2.y = y - 1;

	calculateNormal(*index, index1, index2, f, counter);

	//3
	index1 = index2;

	index2.x = x - 1;
	index2.y = y;

	calculateNormal(*index, index1, index2, f, counter);

	//3
	index1 = index2;

	index2.x = x;
	index2.y = y + 1;

	calculateNormal(*index, index1, index2, f, counter);
	f = f / counter;

	m_heightMap->GetHeightMapValue(index->x, index->y).Normal = XMFLOAT3(XMVectorGetX(f), XMVectorGetY(f), XMVectorGetZ(f));
}

bool Miller::calculateBresenham()
{
	float cellSize = 2 / (float)m_heightMap->materialWidth;
	float len = dx + dy ;//abs(dx) + abs(dy);
	int iter = len / millerStep;
	for (int i = 0; i < iter; i++)
	{
		e2 = 2 * err;
		if (e2 >= dy) 
		{ 
			err += dy;
			if (start_x + sx > end_x)
				break;
			start_x += sx;
			for (map<XMFLOAT2*, XMFLOAT2*> ::iterator it = m_heightMap->activePoints.begin(); it != m_heightMap->activePoints.end(); it++)
			{
				if ((*it).second->x + sx < 0 || (*it).second->x + sx > m_heightMap->materialWidth - 1)
					continue;
				(*it).second->x += sx;
				updateNormals((*it).first);
			}
			Translate(XMFLOAT4(sx * cellSize * millerStep, 0, 0, 1));
		}
		if (e2 <= dx) 
		{ 
			err += dx;
			if (start_y + sy > end_y)
				break;
			start_y += sy;
			for (map<XMFLOAT2*, XMFLOAT2*> ::iterator it = m_heightMap->activePoints.begin(); it != m_heightMap->activePoints.end(); it++)
			{
				if ((*it).second->y + sy < 0 || (*it).second->y + sy > m_heightMap->materialWidth - 1)
					continue;
				(*it).second->y += sy;
				updateNormals((*it).first);
			}
			Translate(XMFLOAT4(0, 0, sy * cellSize * millerStep, 1));
		}
		for (map<XMFLOAT2*, XMFLOAT2*> ::iterator it = m_heightMap->activePoints.begin(); it != m_heightMap->activePoints.end(); it++)
		{
			XMFLOAT2* index = (*it).first;
			int factor = 1;
			int x = index->x * factor;
			int y = index->y * factor;
			if (x < 0 || y < 0)
				continue;
			XMFLOAT3 p = m_heightMap->GetHeightMapValue(x, y).Pos;
			XMVECTOR pos = XMVectorSet(p.x, p.y, p.z, 1);
			bool c = CheckIfPointIsInside(pos);
			m_heightMap->GetHeightMapValue(x, y).Pos.y = XMVectorGetY(pos);
		}
	}
	for (map<XMFLOAT2*, XMFLOAT2*> ::iterator it = m_heightMap->activePoints.begin(); it != m_heightMap->activePoints.end(); it++)
	{
		XMFLOAT2* index = (*it).first;
		int factor = 1;
		int x = index->x * factor;
		int y = index->y * factor;
		if (x < 0 || y < 0)
			continue;
		XMFLOAT3 p = m_heightMap->GetHeightMapValue(x, y).Pos;
		XMVECTOR pos = XMVectorSet(p.x, p.y, p.z, 1);
		CheckIfPointIsInside(pos);
		m_heightMap->GetHeightMapValue(x, y).Pos.y = XMVectorGetY(pos);
		updateNormals((*it).first);
	}
	return true;
}

void Miller::calculateOffset(XMVECTOR& pos, XMVECTOR& miller_center)
{
	XMVECTOR o, cen, l, d;
	float r = m_radius;
	//XMVECTOR miller_center = XMVectorSet(circleCenter.x, circleCenter.y, circleCenter.z, 1);
	o = pos;
	cen = miller_center;
	l = XMVectorSet(0, -1, 0, 1);

	//double A = vx * vx + vy * vy + vz * vz;
	double A = XMVectorGetX(XMVector3LengthSq(l));
	//double B = 2.0 * (px * vx + py * vy + pz * vz - vx * cx - vy * cy - vz * cz);
	XMVECTOR B_vec = 2 * (l * (o - cen));
	double B = XMVectorGetX(B_vec) + XMVectorGetY(B_vec) + XMVectorGetZ(B_vec);
	XMVECTOR C_vec = -2 * o * cen;
	float c_val = XMVectorGetX(C_vec) + XMVectorGetY(C_vec) + XMVectorGetZ(C_vec);
	double C = XMVectorGetX(XMVector3LengthSq(o)) + c_val + XMVectorGetX(XMVector3LengthSq(cen)) - m_radius * m_radius;;

	//double C = px * px - 2 * px * cx + cx * cx + py * py - 2 * py * cy + cy * cy +
		//pz * pz - 2 * pz * cz + cz * cz - m_radius * m_radius;

	// discriminant
	double D = B * B - 4 * A * C;

	if (D < 0)
	{
		return;
	}

	double t1 = (-B - sqrt(D)) / (2.0 * A);
	double t2 = (-B + sqrt(D)) / (2.0 * A);
	double t = t1 > t2 ? t1 : t2;
	float oldVal = XMVectorGetY(pos);
	//float val = XMVectorGetY(l) - XMVectorGetY(pos);
	float val = XMVectorGetY(l);
	float newVal = oldVal + val * t ;
	if (newVal > oldVal)
		return;
	pos = XMVectorSetY(pos, newVal);

}

static int pathCounter = 0;

void Miller::FollowPaths()
{
	Path path = paths[pathCounter++];
	//SetupPosition(path.StartPosition, path.EndPosition);
	path.StartPosition.y += m_radius;
	path.EndPosition.y += m_radius;
	StartMillerPosition = path.StartPosition;
	EndMillerPosition = path.EndPosition;
	SetPosition(StartMillerPosition.x, StartMillerPosition.y, StartMillerPosition.z);
	UpdateActivePoints();
}

void Miller::generateUpperMovement(XMFLOAT2& first, XMFLOAT2& second, XMFLOAT3& lastPos)
{
	float factor = m_heightMap->materialWidth / 2;
	float upperingInMillimeters = 30;
	float offsetZ = upperingInMillimeters / factor;
	XMFLOAT3 firstPos = m_heightMap->GetHeightMapValue(first.x, first.y).Pos;
	XMFLOAT3 firstPosUpper = firstPos;
	firstPosUpper.y += offsetZ;
	XMFLOAT3 secondPos = m_heightMap->GetHeightMapValue(second.x, second.y).Pos;
	XMFLOAT3 secondPosUpper = secondPos;
	secondPosUpper.y += offsetZ;

	Path alongPath;
	alongPath.StartPosition = lastPos;
	alongPath.EndPosition = firstPos;
	paths.push_back(alongPath);

	VertexPosNormal v_start;
	v_start.Pos = alongPath.StartPosition;
	v_start.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_start);
	VertexPosNormal v_end;
	v_end.Pos = alongPath.EndPosition;
	v_end.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_end);

	Path upPath;
	upPath.StartPosition = firstPos;
	upPath.EndPosition = firstPosUpper;
	paths.push_back(upPath);
	v_start.Pos = upPath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = upPath.EndPosition;
	vertices_container.push_back(v_end);

	Path abovePath;
	abovePath.StartPosition = firstPosUpper;
	abovePath.EndPosition = secondPosUpper;
	paths.push_back(abovePath);
	v_start.Pos = abovePath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = abovePath.EndPosition;
	vertices_container.push_back(v_end);

	Path downPath;
	downPath.StartPosition = secondPosUpper;
	downPath.EndPosition = secondPos;
	paths.push_back(downPath);
	v_start.Pos = downPath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = downPath.EndPosition;
	vertices_container.push_back(v_end);

	lastPos = secondPos;
}

void Miller::generateSinglePaths(XMFLOAT3& lastPos, XMFLOAT2& previous, float currentZ)
{
	XMFLOAT3 previousPos = m_heightMap->GetHeightMapValue(previous.x, previous.y).Pos;
	previousPos.y = lastPos.y;

	Path lastPath;
	lastPath.StartPosition = lastPos;
	lastPath.EndPosition = previousPos;
	paths.push_back(lastPath);

	Path changeZPath;
	changeZPath.StartPosition = previousPos;
	XMFLOAT3 modifiedPreviousPos = previousPos;
	modifiedPreviousPos.y = currentZ;
	changeZPath.EndPosition = modifiedPreviousPos;
	paths.push_back(changeZPath);


	VertexPosNormal v_start;
	v_start.Pos = lastPath.StartPosition;
	v_start.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_start);
	VertexPosNormal v_end;
	v_end.Pos = lastPath.EndPosition;
	v_end.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_end);

	v_start.Pos = changeZPath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = changeZPath.EndPosition;
	vertices_container.push_back(v_end);

	//update lastPos to start from lastPoint
	lastPos = modifiedPreviousPos;
}

void Miller::startFromSafePositionFlatMiller(XMFLOAT3 targetPos)
{
	//We are moving DOWN
	XMFLOAT3 modifiedSafePointZ = safePosition;
	modifiedSafePointZ.y = targetPos.y;

	Path bottomPath;
	bottomPath.StartPosition = safePosition;
	bottomPath.EndPosition = modifiedSafePointZ;
	paths.push_back(bottomPath);

	VertexPosNormal v_start;
	v_start.Pos = bottomPath.StartPosition;
	v_start.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_start);
	VertexPosNormal v_end;
	v_end.Pos = bottomPath.EndPosition;
	v_end.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_end);

	XMFLOAT3 targetPosRight = targetPos;
	targetPosRight.x = modifiedSafePointZ.x;

	Path downPath;
	downPath.StartPosition = modifiedSafePointZ;
	downPath.EndPosition = targetPosRight;
	paths.push_back(downPath);

	v_start.Pos = downPath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = downPath.EndPosition;
	vertices_container.push_back(v_end);

	Path leftPath;
	leftPath.StartPosition = targetPosRight;
	leftPath.EndPosition = targetPos;
	paths.push_back(leftPath);

	v_start.Pos = leftPath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = leftPath.EndPosition;
	vertices_container.push_back(v_end);
}

void Miller::startFromSafePosition(XMFLOAT3 targetPos)
{
	float factor = m_heightMap->materialWidth / 2;
	float upperingInMillimeters = 50;
	float offsetZ = upperingInMillimeters / factor;


	//We are moving UP
	XMFLOAT3 modifiedSafePointZ = safePosition;
	modifiedSafePointZ.y += offsetZ;

	Path upPath;
	upPath.StartPosition = safePosition;
	upPath.EndPosition = modifiedSafePointZ;
	paths.push_back(upPath);

	VertexPosNormal v_start;
	v_start.Pos = upPath.StartPosition;
	v_start.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_start);
	VertexPosNormal v_end;
	v_end.Pos = upPath.EndPosition;
	v_end.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_end);

	XMFLOAT3 modifiedTargetPosZ = targetPos;
	modifiedTargetPosZ.y = modifiedSafePointZ.y;

	Path path;
	path.StartPosition = modifiedSafePointZ;
	path.EndPosition = modifiedTargetPosZ;
	paths.push_back(path);

	v_start.Pos = path.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = path.EndPosition;
	vertices_container.push_back(v_end);

	Path downPath;
	downPath.StartPosition = modifiedTargetPosZ;
	downPath.EndPosition = targetPos;
	paths.push_back(downPath);

	v_start.Pos = downPath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = downPath.EndPosition;
	vertices_container.push_back(v_end);
}


void Miller::returnToSafePosition()
{
	float factor = m_heightMap->materialWidth / 2;
	float upperingInMillimeters = 50;
	float offsetZ = upperingInMillimeters / factor;

	XMFLOAT3 modifiedSafePointZ = safePosition;
	modifiedSafePointZ.y += offsetZ;

	//We are moving UP
	XMFLOAT3 lastPos;
	int index;
	if (index = paths.size() == 0)
		lastPos = safePosition;
	else
		lastPos = paths[paths.size() - 1].EndPosition;
	XMFLOAT3 modifiedlastPosZ = lastPos;
	modifiedlastPosZ.y = modifiedSafePointZ.y;

	Path upPath;
	upPath.StartPosition = lastPos;
	upPath.EndPosition = modifiedlastPosZ;
	paths.push_back(upPath);

	VertexPosNormal v_start;
	v_start.Pos = upPath.StartPosition;
	v_start.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_start);
	VertexPosNormal v_end;
	v_end.Pos = upPath.EndPosition;
	v_end.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_end);

	Path path;
	path.StartPosition = modifiedlastPosZ;
	path.EndPosition = modifiedSafePointZ;
	paths.push_back(path);

	v_start.Pos = path.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = path.EndPosition;
	vertices_container.push_back(v_end);

	Path downPath;
	downPath.StartPosition = modifiedSafePointZ;
	downPath.EndPosition = safePosition;
	paths.push_back(downPath);

	v_start.Pos = downPath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = downPath.EndPosition;
	vertices_container.push_back(v_end);
}

void Miller::upperLayerMilling(bool isMovingRight, int radius, int left, int right, int last_y, int MilledDist, float offsetZ)
{
	XMFLOAT3 previousPos, lastPos, downPos;
	if (isMovingRight)
	{
		previousPos = m_heightMap->GetHeightMapValue(right, last_y).Pos;
		lastPos = m_heightMap->GetHeightMapValue(left, last_y).Pos;
	}
	else
	{
		lastPos = m_heightMap->GetHeightMapValue(right, last_y).Pos;
		previousPos = m_heightMap->GetHeightMapValue(left, last_y).Pos;
	}
	//We are moving with miller to the end of material
	previousPos.y += offsetZ;
	lastPos.y += offsetZ;

	Path lastPath;
	lastPath.StartPosition = lastPos;
	lastPath.EndPosition = previousPos;
	paths.push_back(lastPath);

	VertexPosNormal v_start;
	v_start.Pos = lastPath.StartPosition;
	v_start.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_start);
	VertexPosNormal v_end;
	v_end.Pos = lastPath.EndPosition;
	v_end.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_end);

	//We are going down, moving to next row
	last_y += MilledDist;
	if (isMovingRight)
	{
		downPos = m_heightMap->GetHeightMapValue(right, last_y).Pos;
	}
	else
	{
		downPos = m_heightMap->GetHeightMapValue(left, last_y).Pos;
	}
	downPos.y += offsetZ;
	Path downPath;
	downPath.StartPosition = previousPos;
	downPath.EndPosition = downPos;
	paths.push_back(downPath);

	v_start.Pos = downPath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = downPath.EndPosition;
	vertices_container.push_back(v_end);
}

void Miller::moveMill(bool isMovingRight, int iteration, int MilledDist, int columnStep, int radius, int miller_diameter, XMFLOAT2& last, XMFLOAT2& previous, XMFLOAT3& lastPos, float& currentZ, int left, int right)
{
	float big = 100;
	float tolerance = 0;
	//RIGHT MOVEMENT
	//We are checking next columns
		if (isMovingRight)
		{
			for (int i = previous.x; i < m_heightMap->materialWidth; i+=columnStep)
			{
				//We are moving in rows
				int startPoint = last.y - radius + 1;// MilledDist * iteration;
				float maxZ = -big;
				for (int quaziI = i - radius; quaziI < i + radius; quaziI++)
				{
					if (quaziI < 0 || quaziI >  m_heightMap->materialWidth)
						continue;
					for (int j = startPoint; j < startPoint + miller_diameter; j++)
					{
						if (j < 0 || j >  m_heightMap->materialHeight)
							continue;
						float z_offset = /*m_radius*/ + tolerance;
						float z = m_heightMap->GetHeightMapValue(quaziI, j).Pos.y + z_offset;
						//if (i == miller_diameter && iteration == 6)
						//	m_heightMap->GetHeightMapValue(quaziI, j).Normal = XMFLOAT3(-1, -1, -1);
						if (z > maxZ)
						{
							previous.x = i - columnStep;
							previous.y = last.y;

							maxZ = z;
						}
					}
				}
				if (maxZ != currentZ)
				{
					currentZ = maxZ;
					generateSinglePaths(lastPos, previous, currentZ);
				}

			}
		}
		else
		{
			for (int i = previous.x; i >= 0; i -= columnStep)
			{
				//We are moving in rows
				int startPoint = last.y - radius + 1;//MilledDist * iteration;
				float maxZ = -big;
				for (int quaziI = i + radius; quaziI > i - radius; quaziI--)
				{
					if (quaziI < 0 || quaziI >  m_heightMap->materialWidth)
						continue;
					for (int j = startPoint; j < startPoint + miller_diameter; j++)
					{
						if (j < 0 || j >  m_heightMap->materialHeight)
							continue;
						float z_offset = /*m_radius*/ + tolerance;
						float z = m_heightMap->GetHeightMapValue(quaziI, j).Pos.y + z_offset;
						if (z > maxZ)
						{
							previous.x = i + columnStep;
							previous.y = last.y;

							maxZ = z;
						}
					}
				}
				if (maxZ != currentZ)
				{
					currentZ = maxZ;
					generateSinglePaths(lastPos, previous, currentZ);
				}
			}
		}
		XMFLOAT2 EdgeIndex;

		if (isMovingRight)
			EdgeIndex.x = right;
		else
			EdgeIndex.x = left;
	//We are moving with miller to the end of material
	XMFLOAT3 previousPos;
	previousPos = m_heightMap->GetHeightMapValue(EdgeIndex.x, last.y).Pos;
	previousPos.y = lastPos.y;

	Path lastPath;
	lastPath.StartPosition = lastPos;
	lastPath.EndPosition = previousPos;
	paths.push_back(lastPath);

	VertexPosNormal v_start;
	v_start.Pos = lastPath.StartPosition;
	v_start.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_start);
	VertexPosNormal v_end;
	v_end.Pos = lastPath.EndPosition;
	v_end.Normal = XMFLOAT3(0, 0, 1);
	vertices_container.push_back(v_end);

	//CO JAK BY£O PODNIESIENIE W OSTATNIM?!

	//We are going down, moving to next row

	lastPos = previousPos;
	last.y += MilledDist;
	previousPos = m_heightMap->GetHeightMapValue(EdgeIndex.x, last.y).Pos;
	previousPos.y = lastPos.y;

	Path downPath;
	downPath.StartPosition = lastPos;
	downPath.EndPosition = previousPos;
	paths.push_back(downPath);

	v_start.Pos = downPath.StartPosition;
	vertices_container.push_back(v_start);
	v_end.Pos = downPath.EndPosition;
	vertices_container.push_back(v_end);

	lastPos = previousPos;
}

void Miller::GeneratePathsThirdPart(vector<XMFLOAT3>& orderedPrecisePoints)
{
	float radius = m_radius * m_heightMap->materialWidth / 2;
	float upperingInMillimeters = 50;
	float factor = m_heightMap->materialWidth / 2;
	float offsetZ = upperingInMillimeters / factor;

	Path path;

	XMFLOAT3 previousPos;
	XMFLOAT3 firstPos = orderedPrecisePoints[0];
	startFromSafePosition(firstPos);

	previousPos = orderedPrecisePoints[0];
	for (int i = 1; i < orderedPrecisePoints.size(); i++)
	{
		path.StartPosition = previousPos;
		XMFLOAT3 currentPos = orderedPrecisePoints[i];
		path.EndPosition = currentPos;
		paths.push_back(path);

		VertexPosNormal v_start;
		v_start.Pos = path.StartPosition;
		v_start.Normal = XMFLOAT3(0, 0, 1);
		vertices_container.push_back(v_start);
		VertexPosNormal v_end;
		v_end.Pos = path.EndPosition;
		v_end.Normal = XMFLOAT3(0, 0, 1);
		vertices_container.push_back(v_end);

		previousPos = currentPos;
	}

	returnToSafePosition();
}

void Miller::GeneratePathsSecondPart(vector<XMFLOAT2>& orderedContourPoints)
{
	float radius = m_radius * m_heightMap->materialWidth / 2;
	float upperingInMillimeters = 50;
	float factor = m_heightMap->materialWidth / 2;
	float offsetZ = upperingInMillimeters / factor;

	vector<int> columns[150];

	Path path;

	XMFLOAT3 previousPos;
	XMFLOAT2 previousIndex;
	int x = orderedContourPoints[0].x;
	int y = orderedContourPoints[0].y;
	previousIndex = XMFLOAT2(x, y);
	XMFLOAT3 firstPos = m_heightMap->GetHeightMapValue(x, y).Pos;
	startFromSafePositionFlatMiller(firstPos);

	previousPos = m_heightMap->GetHeightMapValue(x, y).Pos;
	for (int i = 1; i < orderedContourPoints.size(); i++)
	{
		path.StartPosition = previousPos;
		x = orderedContourPoints[i].x;
		y = orderedContourPoints[i].y;
		XMFLOAT3 currentPos = m_heightMap->GetHeightMapValue(x, y).Pos;
		path.EndPosition = currentPos;
		paths.push_back(path);

		VertexPosNormal v_start;
		v_start.Pos = path.StartPosition;
		v_start.Normal = XMFLOAT3(0, 0, 1);
		vertices_container.push_back(v_start);
		VertexPosNormal v_end;
		v_end.Pos = path.EndPosition;
		v_end.Normal = XMFLOAT3(0, 0, 1);
		vertices_container.push_back(v_end);


		int diffX = x - previousIndex.x;
		int diffY = y - previousIndex.y;

		int max = abs(diffX) > abs(diffY) ? abs(diffX) : abs(diffY);

		if (abs(diffX) == abs(diffY))
		{
			if (diffX > 0 && diffY > 0)
			{
				for (int a = 0; a <= max; a++)
				{
					columns[(int)previousIndex.y + a].push_back((int)previousIndex.x + a);
				}
			}
			else if (diffX > 0 && diffY < 0)
			{
				for (int a = 0; a <= max; a++)
				{
					columns[(int)previousIndex.y - a].push_back(previousIndex.x + a);
				}
			}
			else if (diffX < 0 && diffY > 0)
			{
				for (int a = 0; a <= max; a++)
				{
					columns[(int)previousIndex.y + a].push_back((int)previousIndex.x - a);
				}
			}
			else if (diffX < 0 && diffY < 0)
			{
				for (int a = 0; a <= max; a++)
				{
					columns[(int)previousIndex.y - a].push_back((int)previousIndex.x - a);
				}
			}
		}
		else
		{
			if (abs(diffX) > 0)
			{
				if (diffX > 0)
				for (int a = 0; a <= max; a++)
				{
					columns[(int)previousIndex.y].push_back((int)previousIndex.x + a);
				}
				else
				for (int a = 0; a <= max; a++)
				{
					columns[(int)previousIndex.y].push_back((int)previousIndex.x - a);
				}
			}
			else
			{
				if (diffY > 0)
					for (int a = 0; a <= max; a++)
					{
						columns[(int)previousIndex.y + a].push_back((int)previousIndex.x);
					}
				else
					for (int a = 0; a <= max; a++)
					{
						columns[(int)previousIndex.y - a].push_back((int)previousIndex.x);
					}
			}
		}

		previousIndex.x = x;
		previousIndex.y = y;
		previousPos = currentPos;
	}

	returnToSafePosition();

	//ZIG ZAG to remove rest of the material

	firstPos = m_heightMap->GetHeightMapValue(0, 0).Pos;
	startFromSafePosition(firstPos);
	int tolerance = 0;
	int last = m_heightMap->materialWidth - 1;
	for (int i = 0; i < m_heightMap->materialHeight; i+=radius)
	{
		bool sentinel = false;
		int lastJ = 0;
		int firstJ = -1;
		int secondJ = -1;
		int lastK = 0;
		XMFLOAT3 lastPos = m_heightMap->GetHeightMapValue(0, i).Pos;
		for (int j = 0; j < m_heightMap->materialWidth; j++)
		{
			for (int k = i - radius; k <= i + radius; k++)
			{
				if (k < 0 || k > m_heightMap->materialHeight)
					continue;
				float z = m_heightMap->GetHeightMapValue(j, k).Pos.y;
				if (z > offsetZ)
				{
					if (firstJ == -1)
					{
						firstJ = j - radius - tolerance;

						int min;
						int minDist = 1000;
						int dist;
						//wyznacz najbli¿szy
						for (int a = 0; a < columns[i].size(); a++)
						{
							dist = abs(firstJ - columns[i][a]);
							if (dist < minDist)
							{
								minDist = dist;
								min = columns[i][a];
							}
						}

						if (columns[i].size() == 0)
							min = firstJ;

						firstJ = min;




						lastK = k;
						break;
					}
				}
				else
				{
					if (firstJ != -1 && lastK == k)
					{
						//tzn jest ju¿ ustawione podniesienie
						secondJ = j + radius + tolerance;
						j = secondJ;
						int min;
						int minDist = 1000;
						int dist;
						//wyznacz najbli¿szy
						for (int a = 0; a < columns[i].size(); a++)
						{
							dist = abs(secondJ - columns[i][a]);
							if (dist < minDist)
							{
								minDist = dist;
								min = columns[i][a];
							}
						}

						if (columns[i].size() == 0)
							min = secondJ;

						secondJ = min;

						//teraz idŸ od firstJ do secondJ
						generateUpperMovement(XMFLOAT2(firstJ, i), XMFLOAT2(secondJ, i), lastPos);
						firstJ = secondJ = -1;
						break;
					}
				}
			}
		}
		Path oneWay;
		oneWay.StartPosition = lastPos;
		//nie by³o nic na œcie¿ce
		if (!sentinel)
		{
			oneWay.EndPosition = m_heightMap->GetHeightMapValue(last, i).Pos;
		}
		else
		{
			oneWay.EndPosition = m_heightMap->GetHeightMapValue(lastJ, i).Pos;
		}
		paths.push_back(oneWay);

		VertexPosNormal v_start;
		v_start.Pos = oneWay.StartPosition;
		v_start.Normal = XMFLOAT3(0, 0, 1);
		vertices_container.push_back(v_start);
		VertexPosNormal v_end;
		v_end.Pos = oneWay.EndPosition;
		v_end.Normal = XMFLOAT3(0, 0, 1);
		vertices_container.push_back(v_end);

		//UP WAY
		Path upWay;
		upWay.StartPosition = oneWay.EndPosition;
		upWay.EndPosition = oneWay.EndPosition;;
		upWay.EndPosition.y += offsetZ;
		paths.push_back(upWay);
		v_start.Pos = upWay.StartPosition;
		vertices_container.push_back(v_start);
		v_end.Pos = upWay.EndPosition;
		vertices_container.push_back(v_end);

		//prevent last iteration to loop
		if (i + radius >= m_heightMap->materialHeight)
			continue;

		//RETURN WAY
		Path returnWay;
		returnWay.StartPosition = upWay.EndPosition;
		returnWay.EndPosition = m_heightMap->GetHeightMapValue(0, i + radius).Pos;
		returnWay.EndPosition.y += offsetZ;
		paths.push_back(returnWay);
		v_start.Pos = returnWay.StartPosition;
		vertices_container.push_back(v_start);
		v_end.Pos = returnWay.EndPosition;
		vertices_container.push_back(v_end);
	

		//DOWN WAY
		Path downWay;
		downWay.StartPosition = returnWay.EndPosition;
		downWay.EndPosition = m_heightMap->GetHeightMapValue(0, i + radius).Pos;
		paths.push_back(downWay);
		v_start.Pos = downWay.StartPosition;
		vertices_container.push_back(v_start);
		v_end.Pos = downWay.EndPosition;
		vertices_container.push_back(v_end);
	}

	returnToSafePosition();
}

void Miller::GeneratePaths()
{
	float cellSize = 2 / (float)m_heightMap->materialWidth;
	float radius = m_radius * m_heightMap->materialWidth / 2;
	int MilledDist = 4;
	int columnStep = 2;

	//We are moving with miller of radius given by m_radius
	float big = 1000;
	float currentZ = m_heightMap->def_map_len * m_heightMap->map_y / m_heightMap->def_map_number;// 1.0;
	XMFLOAT2 last, previous;
	XMFLOAT3 lastPos;
	int miller_diameter = 2 * radius;
	float factor = m_heightMap->materialWidth / 2;
	float upperingInMillimeters = 20;
	float offsetZ = upperingInMillimeters / factor;

	for (int j = 0; j < miller_diameter; j++)
	{
		for (int i = 0; i < miller_diameter; i++)
		{
			float z = m_heightMap->GetHeightMapValue(j, i).Pos.y;
			if (z > currentZ)
				currentZ = z;
		}
	}

	last.x = abs(m_heightMap->materialWidth - ((int)((m_heightMap->materialWidth - 0.0001) / columnStep) - 1) * columnStep);
	last.y = 0;
	previous = last;

	lastPos = m_heightMap->GetHeightMapValue(last.x, last.y).Pos;
	//lastPos.y += m_radius;
	int pathNo = (m_heightMap->materialHeight) / MilledDist;

	int left = last.x;
	int right = abs(m_heightMap->materialWidth - left);
	XMFLOAT3 targetStartPosition = m_heightMap->GetHeightMapValue(left, last.y).Pos;
	targetStartPosition.y += offsetZ;
	startFromSafePosition(targetStartPosition);
	//upper
	for (int n = 0; n < pathNo; n++)
	{
		if (n % 2)
		{
			upperLayerMilling(false, radius, left, right, n * MilledDist, MilledDist, offsetZ);
		}
		else
		{
			upperLayerMilling(true, radius, left, right, n * MilledDist, MilledDist, offsetZ);
		}
	}
	returnToSafePosition();
	targetStartPosition.y -= offsetZ;
	startFromSafePosition(targetStartPosition);
	for (int n = 0; n < pathNo; n++)
	{
		if (n % 2)
		{
			moveMill(false, n, MilledDist, columnStep, radius, miller_diameter, last, previous, lastPos, currentZ, left, right);
		}
		else
		{
			moveMill(true, n, MilledDist, columnStep, radius, miller_diameter, last, previous, lastPos, currentZ, left, right);
		}
	}
	returnToSafePosition();
	int end = 0;
}

void Miller::Initialize()
{
	safePosition = XMFLOAT3(1.5, 0.8, 0);
	InitializeCyllinder();

	float height = 1.0f;
	float radius = m_radius;
	float topRadius = 1.0f;
	float bottomRadius = 1.0f;
	int sliceCount = 30;
	int stackCount = 30;
	float stackHeight = height / (float)stackCount;
	float radiusStep = (topRadius - bottomRadius) / stackCount;
	int ringCount = stackCount + 1;
	verticesCount = (stackCount - 1) * (sliceCount + 1) + 2;
	indicesCount = sliceCount * 3 + sliceCount * 3 + (stackCount - 2) * sliceCount * 6;
	VertexPosNormal* vertices = new VertexPosNormal[verticesCount];
	unsigned short* indices = new unsigned short[indicesCount];
	//vertices[0].Pos = XMFLOAT3(circleCenter.x, circleCenter.y + radius, circleCenter.z);
	vertices[0].Pos = XMFLOAT3(0, radius, 0);
	vertices[0].Normal = XMFLOAT3(0, 0, 1);
	//ret.Vertices.Add(new Vertex(0, radius, 0, 0, 1, 0, 1, 0, 0, 0, 0));
	float phiStep = XM_PI / (float)stackCount;
	float thetaStep = XM_2PI / (float)sliceCount;

	for (int i = 1; i <= stackCount - 1; i++) {
		float phi = i*phiStep;
		for (int j = 0; j <= sliceCount; j++) {
			float theta = j*thetaStep;
			XMVECTOR p = XMVectorSet(
				(radius*sin(phi)*cos(theta)),
				(radius*cos(phi)),
				(radius*sin(phi)*sin(theta)),
				1
				);

			//var t = new Vector3(-radius*MathF.Sin(phi)*MathF.Sin(theta), 0, radius*MathF.Sin(phi)*MathF.Cos(theta));
			//t.Normalize();
			//var n = p;
			//n.Normalize();
			//var uv = new Vector2(theta / (MathF.PI * 2), phi / MathF.PI);
			int index = (i - 1) * (sliceCount + 1) + j + 1;
			//p = XMVector3Transform(p, XMMatrixTranslation(circleCenter.x, circleCenter.y, circleCenter.z));
			vertices[index].Pos = XMFLOAT3(XMVectorGetX(p), XMVectorGetY(p), XMVectorGetZ(p));
			vertices[index].Normal = XMFLOAT3(0, 0, 1);
			//ret.Vertices.Add(new Vertex(p, n, t, uv));
		}
	}
	/*vertices[verticesCount - 1].Pos = XMFLOAT3(circleCenter.x, circleCenter.y -radius, circleCenter.z);*/
	vertices[verticesCount - 1].Pos = XMFLOAT3(0, 0 - radius, 0);
	vertices[verticesCount - 1].Normal = XMFLOAT3(0, 0, 1);

	//ret.Vertices.Add(new Vertex(0, -radius, 0, 0, -1, 0, 1, 0, 0, 0, 1));


	for (int i = 1; i <= sliceCount; i++) {
		int index = i * 3;
		indices[index++] = 0;
		indices[index++] = i + 1;
		indices[index++] = i;
		//ret.Indices.Add(0);
		//ret.Indices.Add(i + 1);
		//ret.Indices.Add(i);
	}
	int baseIndex = 1;
	int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount - 2; i++) {
		for (int j = 0; j < sliceCount; j++) {
			int index = (i * sliceCount + j) * 6 + sliceCount * 3;
			indices[index++] = baseIndex + i*ringVertexCount + j;
			indices[index++] = baseIndex + i*ringVertexCount + j + 1;
			indices[index++] = baseIndex + (i + 1)*ringVertexCount + j;
			//ret.Indices.Add(baseIndex + i*ringVertexCount + j);
			//ret.Indices.Add(baseIndex + i*ringVertexCount + j + 1);
			//ret.Indices.Add(baseIndex + (i + 1)*ringVertexCount + j);

			indices[index++] = baseIndex + (i + 1)*ringVertexCount + j;
			indices[index++] = baseIndex + i*ringVertexCount + j + 1;
			indices[index++] = baseIndex + (i + 1)*ringVertexCount + j + 1;

			//ret.Indices.Add(baseIndex + (i + 1)*ringVertexCount + j);
			//ret.Indices.Add(baseIndex + i*ringVertexCount + j + 1);
			//ret.Indices.Add(baseIndex + (i + 1)*ringVertexCount + j + 1);
		}
	}
	int southPoleIndex = verticesCount - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (int i = 0; i < sliceCount; i++) {
		int index = i * 3 + sliceCount * 3 + (stackCount - 2) * sliceCount * 6;
		indices[index++] = southPoleIndex;
		indices[index++] = baseIndex + i;
		indices[index++] = baseIndex + i + 1;

		//ret.Indices.Add(southPoleIndex);
		//ret.Indices.Add(baseIndex + i);
		//ret.Indices.Add(baseIndex + i + 1);
	}
	m_vertexBuffer = m_service.Device.CreateVertexBuffer(vertices, verticesCount);
	m_indexBuffer = m_service.Device.CreateIndexBuffer(indices, indicesCount);
	delete[] vertices;
	delete[] indices;

	//SetPosition(circleCenter.x, circleCenter.y, circleCenter.z);
}


void Miller::InitializeCyllinder()
{
	// Set the number of vertices in the vertex array.
	m_vertexCount = 4;
	//ilosc punktow przyblizaj¹cych okrag
	//verticesCount = 120;
	//indicesCount = verticesCount * 2;
	//VertexPosNormal* vertices = new VertexPosNormal[verticesCount];
	//unsigned short* indices = new unsigned short[indicesCount];

	float height = 0.5f;
	float topRadius = m_radius;
	float bottomRadius = m_radius;
	int sliceCount = 20;
	int stackCount = 20;
	float stackHeight = height / (float)stackCount;
	float radiusStep = (topRadius - bottomRadius) / (float)stackCount;
	int ringCount = stackCount + 1;

	verticesCyllinderCount = ringCount * (sliceCount + 1) + sliceCount + 1 + 1;
	indicesCyllinderCount = stackCount * sliceCount * 6 + 3 * sliceCount;
	VertexPosNormal* vertices = new VertexPosNormal[verticesCyllinderCount];
	unsigned short* indices = new unsigned short[indicesCyllinderCount];


	//k¹t o jaki bêd¹ obracane punkty
	//float delta = XM_2PI / (float)verticesCount;

	//wyznaczanie punktów
	//for (int t = 0; t < verticesCount; t++)
	//{

	//	XMVECTOR pos = XMLoadFloat3(
	//		&XMFLOAT3(
	//		circleRadius * cos(t * delta),
	//		circleRadius * sin(t * delta),
	//		0));

	//	//pos = XMVector3Transform(pos, XMMatrixRotationY(XM_PIDIV2));
	//	//pos = XMVector3Transform(pos, XMMatrixRotationZ(XM_PI / 6.0f));
	//	pos = XMVector3Transform(pos, XMMatrixTranslation(circleCenter.x, circleCenter.y, circleCenter.z));

	//	vertices[t].Pos = XMFLOAT3(XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));
	//	vertices[t].Normal = XMFLOAT3(sqrt(3) / 2.0f, 0.5f, 0.0f);
	//}

	//XMVECTOR offset = XMVectorSet(circleCenter.x, circleCenter.y, circleCenter.z, 1);

	for (int i = 0; i < ringCount; i++)
	{
		float y = -0.5f*height + i*stackHeight;
		float r = bottomRadius + i*radiusStep;
		float dTheta = XM_2PI / (float)sliceCount;
		for (int j = 0; j <= sliceCount; j++) {

			float c = cos(j*dTheta);
			float s = sin(j*dTheta);

			XMVECTOR v = XMVectorSet(r*c, y + height / 2, r*s, 1);
			//v = v + offset;
			//XMVECTOR uv = XMVectorSet((float)j / (float)sliceCount, 1.0f - (float)i / (float)stackCount, 0, 1);
			//XMVECTOR t = XMVectorSet(-s, 0.0f, c, 1);

			//float dr = bottomRadius - topRadius;
			//XMVECTOR bitangent = XMVectorSet(dr*c, -height, dr*s, 1);

			//XMVECTOR n = XMVector3Normalize( XMVector3Cross(t, bitangent));
			int index = i * (sliceCount + 1) + j;

			vertices[index].Pos = XMFLOAT3(XMVectorGetX(v), XMVectorGetY(v), XMVectorGetZ(v));
			vertices[index].Normal = XMFLOAT3(0, 0, 1);
			//ret.Vertices.Add(new Vertex(v, n, t, uv));
		}
	}

	int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount; i++) {
		for (int j = 0; j < sliceCount; j++) {
			int index = (i * sliceCount + j) * 6;
			indices[index++] = i*ringVertexCount + j;
			indices[index++] = (i + 1)*ringVertexCount + j;
			indices[index++] = (i + 1)*ringVertexCount + j + 1;
			//ret.Indices.Add(i*ringVertexCount + j);
			//ret.Indices.Add((i + 1)*ringVertexCount + j);
			//ret.Indices.Add((i + 1)*ringVertexCount + j + 1);

			indices[index++] = i*ringVertexCount + j;
			indices[index++] = (i + 1)*ringVertexCount + j + 1;
			indices[index++] = i*ringVertexCount + j + 1;
			//ret.Indices.Add(i*ringVertexCount + j);
			//ret.Indices.Add((i + 1)*ringVertexCount + j + 1);
			//ret.Indices.Add(i*ringVertexCount + j + 1);
		}
	}


	int baseIndex = ringCount * (sliceCount + 1) ;
	int index = baseIndex;
	float y = 0.5f*height;
	float dTheta = XM_2PI / (float)sliceCount;

	for (int i = 0; i <= sliceCount; i++) {
		float x = topRadius*cos(i*dTheta);
		float z = topRadius*sin(i*dTheta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;
		//ret.Vertices.Add(new Vertex(x, y, z, 0, 1, 0, 1, 0, 0, u, v));
		vertices[index].Pos = XMFLOAT3(x, y + height / 2, z);
		vertices[index].Normal = XMFLOAT3(0, 0, 1);
		index++;
	}
	////ret.Vertices.Add(new Vertex(0, y, 0, 0, 1, 0, 1, 0, 0, 0.5f, 0.5f));
	vertices[index].Pos = XMFLOAT3(0, y + height / 2, 0);
	vertices[index].Normal = XMFLOAT3(0, 0, 1);

	int centerIndex = verticesCyllinderCount - 1;
	index = stackCount * sliceCount * 6;
	for (int i = 0; i < sliceCount; i++) {
		//ret.Indices.Add(centerIndex);
		//ret.Indices.Add(baseIndex + i + 1);
		//ret.Indices.Add(baseIndex + i);
		indices[index++] = centerIndex;
		indices[index++] = baseIndex + i + 1;
		indices[index++] = baseIndex + i;
	}




	m_vertexBuffer_SquareFirst = m_service.Device.CreateVertexBuffer(vertices, verticesCyllinderCount);
	//int counter = 0;
	//for (int i = 0; i < indicesCount - 2; i += 2)
	//{
	//	indices[i] = counter++;
	//	indices[i + 1] = counter;
	//}
	//indices[indicesCount - 2] = counter;
	//indices[indicesCount - 1] = 0;
	m_indexBuffer_SquareFirst = m_service.Device.CreateIndexBuffer(indices, indicesCyllinderCount);
	delete[] vertices;
	delete[] indices;
}


//void  CreateSphere(float radius, int sliceCount, int stackCount) {
//	//var ret = new MeshData();
//	ret.Vertices.Add(new Vertex(0, radius, 0, 0, 1, 0, 1, 0, 0, 0, 0));
//	var phiStep = MathF.PI / stackCount;
//	var thetaStep = 2.0f*MathF.PI / sliceCount;
//
//	for (int i = 1; i <= stackCount - 1; i++) {
//		var phi = i*phiStep;
//		for (int j = 0; j <= sliceCount; j++) {
//			var theta = j*thetaStep;
//			var p = new Vector3(
//				(radius*MathF.Sin(phi)*MathF.Cos(theta)),
//				(radius*MathF.Cos(phi)),
//				(radius* MathF.Sin(phi)*MathF.Sin(theta))
//				);
//
//			var t = new Vector3(-radius*MathF.Sin(phi)*MathF.Sin(theta), 0, radius*MathF.Sin(phi)*MathF.Cos(theta));
//			t.Normalize();
//			var n = p;
//			n.Normalize();
//			var uv = new Vector2(theta / (MathF.PI * 2), phi / MathF.PI);
//			ret.Vertices.Add(new Vertex(p, n, t, uv));
//		}
//	}
//	ret.Vertices.Add(new Vertex(0, -radius, 0, 0, -1, 0, 1, 0, 0, 0, 1));
//
//
//	for (int i = 1; i <= sliceCount; i++) {
//		ret.Indices.Add(0);
//		ret.Indices.Add(i + 1);
//		ret.Indices.Add(i);
//	}
//	var baseIndex = 1;
//	var ringVertexCount = sliceCount + 1;
//	for (int i = 0; i < stackCount - 2; i++) {
//		for (int j = 0; j < sliceCount; j++) {
//			ret.Indices.Add(baseIndex + i*ringVertexCount + j);
//			ret.Indices.Add(baseIndex + i*ringVertexCount + j + 1);
//			ret.Indices.Add(baseIndex + (i + 1)*ringVertexCount + j);
//
//			ret.Indices.Add(baseIndex + (i + 1)*ringVertexCount + j);
//			ret.Indices.Add(baseIndex + i*ringVertexCount + j + 1);
//			ret.Indices.Add(baseIndex + (i + 1)*ringVertexCount + j + 1);
//		}
//	}
//	var southPoleIndex = ret.Vertices.Count - 1;
//	baseIndex = southPoleIndex - ringVertexCount;
//	for (int i = 0; i < sliceCount; i++) {
//		ret.Indices.Add(southPoleIndex);
//		ret.Indices.Add(baseIndex + i);
//		ret.Indices.Add(baseIndex + i + 1);
//	}
//	return ret;
//}

void Miller::preDraw()
{
	m_service.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	XMFLOAT4 color = XMFLOAT4(1.0, 1.0, 0.0f, 1.0);
	m_service.Context->UpdateSubresource(m_service.cbSurfaceColor.get(), 0, 0, &color, 0, 0);
	XMFLOAT4 colors[5];
	ZeroMemory(colors, sizeof(XMFLOAT4) * 5);
	colors[0] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); //ambient color
	colors[1] = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); //surface [ka, kd, ks, m]
	colors[2] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	//colors[3] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	//colors[4] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	m_service.Context->UpdateSubresource(m_service.cbLightColors.get(), 0, 0, colors, 0, 0);
}

void Miller::onDraw()
{
	
	ID3D11Buffer* b = m_vertexBuffer.get();
	if (type == millerType::Sphere)
	{
		m_service.Context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE_WITH_NORMAL, &VB_OFFSET);
		m_service.Context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
		m_service.Context->DrawIndexed(indicesCount, 0, 0);
	}
	//XMFLOAT4 color = XMFLOAT4(0.0, 1.0, 0.0f, 1.0);
	//m_service.Context->UpdateSubresource(m_service.cbSurfaceColor.get(), 0, 0, &color, 0, 0);
	b = m_vertexBuffer_SquareFirst.get();
	m_service.Context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE_WITH_NORMAL, &VB_OFFSET);
	m_service.Context->IASetIndexBuffer(m_indexBuffer_SquareFirst.get(), DXGI_FORMAT_R16_UINT, 0);
	m_service.Context->DrawIndexed(indicesCyllinderCount, 0, 0);
	//color = XMFLOAT4(0.0, 0.0, 1.0f, 1.0);
	//m_service.Context->UpdateSubresource(m_service.cbSurfaceColor.get(), 0, 0, &color, 0, 0);
	//b = m_vertexBuffer_SquareSecond.get();
	//m_service.Context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE_WITH_NORMAL, &VB_OFFSET);
	//m_service.Context->IASetIndexBuffer(m_indexBuffer_SquareSecond.get(), DXGI_FORMAT_R16_UINT, 0);
	//m_service.Context->DrawIndexed(indicesCount, 0, 0);
}

void Miller::afterDraw()
{
	m_service.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	XMFLOAT4 color = XMFLOAT4(1.0, 1.0, 1.0f, 1.0);
	m_service.Context->UpdateSubresource(m_service.cbSurfaceColor.get(), 0, 0, &color, 0, 0);
	XMFLOAT4 colors[5];
	ZeroMemory(colors, sizeof(XMFLOAT4) * 5);
	colors[0] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); //ambient color
	colors[1] = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); //surface [ka, kd, ks, m]
	colors[2] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	//colors[3] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	//colors[4] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	m_service.Context->UpdateSubresource(m_service.cbLightColors.get(), 0, 0, colors, 0, 0);
}

void Miller::setLineTopology()
{
}

void Miller::setTriangleTopology()
{
}
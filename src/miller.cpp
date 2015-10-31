#include "miller.h"

using namespace gk2;


Miller::Miller(Service &service) : ModelClass(service)
{
	//m_Type = ModelType::CursorType;
	ModelClass::Initialize();
	Miller::Initialize();
}


Miller::Miller(const Miller& other) : ModelClass(other)
{
}


Miller::~Miller()
{
	ModelClass::~ModelClass();
}

void Miller :: SetupPosition(XMFLOAT3 startMillerPosition, XMFLOAT3 endMillerPosition)
{
	StartMillerPosition = startMillerPosition;
	EndMillerPosition = endMillerPosition;

	float val = m_heightMap->def_map_y;

	StartMillerPosition.z = val - StartMillerPosition.z;
	EndMillerPosition.z = val - EndMillerPosition.z;

	float factor = m_heightMap ->def_map_x / 2;

	StartMillerPosition.x /= factor;
	StartMillerPosition.y /= (factor);
	StartMillerPosition.z /= (val);

	EndMillerPosition.x /= factor;
	EndMillerPosition.y /= (factor);
	EndMillerPosition.z /= (val);

	if (type == millerType::Sphere)
	{
		StartMillerPosition.z += m_radius;
		EndMillerPosition.z += m_radius;
	}

	SetPosition(StartMillerPosition.x, -StartMillerPosition.z, -StartMillerPosition.y);
}

void Miller::Reset(HeightMap* heightMap)
{
	m_heightMap = heightMap;
	m_heightMap->activePoints.clear();
}

void Miller::searchRecursively(int x, int y, map<XMFLOAT2*, XMFLOAT2*>* futureActivePoints)
{
	for (int a = -1; a <= 1; a++)
		for (int b = -1; b <= 1; b++)
		{
			int x_c = x + a;
			int y_c = y + b;
			if (x_c < 0 || y_c < 0)
				continue;
			if (x_c >= m_heightMap->materialWidth || y_c >= m_heightMap->materialHeight)
				continue;
			bool& val = m_heightMap->GetVerticeActiveValue(x_c, y_c);
			if (!val)
			{
				XMFLOAT2 *checkPoint = new XMFLOAT2(x_c, y_c);
				XMFLOAT3 p = m_heightMap->GetHeightMapValue(checkPoint->x, checkPoint->y).Pos;
				XMVECTOR pos = XMVectorSet(p.x, p.y, p.z, 1);
				bool inside = CheckIfPointIsInside(pos);
				if (inside)
				{
					//if (!(a == 0 && b == 0))
					//{
					//	int d = 10;
					//}
					//float przed = m_heightMap->GetHeightMapValue(checkPoint->x, checkPoint->y).Pos.y;
					m_heightMap->GetHeightMapValue(checkPoint->x, checkPoint->y).Pos.y = XMVectorGetY(pos);
					//float po = m_heightMap->GetHeightMapValue(checkPoint->x, checkPoint->y).Pos.y;
					//if (!(a == 0 && b == 0))
					futureActivePoints->insert(pair<XMFLOAT2*, XMFLOAT2*>(checkPoint, checkPoint));
					val = true;
					searchRecursively(checkPoint->x, checkPoint->y, futureActivePoints);
				}
				else
				{
					delete checkPoint;
				}
				//delete checkPoint;
			}
		}
}

void Miller::CheckNeighbours()
{
	//for (int i = 0; i < m_heightMap->activePoints.size(); i++)
	int counter = 0;
	map<XMFLOAT2*, XMFLOAT2*>* futureActivePoints = new map<XMFLOAT2*, XMFLOAT2*>();
	for (map<XMFLOAT2*, XMFLOAT2*> ::iterator it = m_heightMap->activePoints.begin(); it != m_heightMap->activePoints.end(); it++)
	{
		counter++;
		XMFLOAT2* index = (*it).first;
		XMFLOAT3 p = m_heightMap->GetHeightMapValue(index->x, index->y).Pos;
		XMVECTOR pos = XMVectorSet(p.x, p.y, p.z, 1);
		if (CheckIfPointIsInside(pos))
		{
			//XMFLOAT2* index = m_heightMap->activePoints[i];
			//float przed = m_heightMap->GetHeightMapValue(index->x, index->y).Pos.y;
			m_heightMap->GetHeightMapValue(index->x, index->y).Pos.y = XMVectorGetY(pos);
			//float po = m_heightMap->GetHeightMapValue(index->x, index->y).Pos.y;
			futureActivePoints->insert(pair<XMFLOAT2*, XMFLOAT2*>(index, index));
			searchRecursively(index->x, index->y, futureActivePoints);
		}
		else
		{
			int x_c = index->x;
			int y_c = index->y;
			bool& val = m_heightMap->GetVerticeActiveValue(x_c, y_c);
			val = false;
			delete (*it).first;
			//m_heightMap->activePoints.erase((*it).first);
		}
	}

	//m_heightMap->activePoints.clear(); //WYCIEK
	m_heightMap->activePoints = *futureActivePoints;
	int a = m_heightMap->activePoints.size();
	int b = 10;
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
				if (isMovingVertically)
					MessageBox(NULL, L"Frez p³aski próbuje wycinaæ w dó³.", NULL, NULL);
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
	float len = dx + dy;//abs(dx) + abs(dy);
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
				if ((*it).second->x + sx <= 0 || (*it).second->x + sx > m_heightMap->materialWidth)
					continue;
				(*it).second->x += sx;
				updateNormals((*it).first);
			}
			Translate(XMFLOAT4(cellSize * millerStep, 0, 0, 1));
		}
		if (e2 <= dx) 
		{ 
			err += dx;
			if (start_y + sy > end_y)
				break;
			start_y += sy;
			for (map<XMFLOAT2*, XMFLOAT2*> ::iterator it = m_heightMap->activePoints.begin(); it != m_heightMap->activePoints.end(); it++)
			{
				if ((*it).second->y + sy<= 0 || (*it).second->y + sy > m_heightMap->materialWidth)
					continue;
				(*it).second->y += sy;
				updateNormals((*it).first);
			}
			Translate(XMFLOAT4(0, 0, cellSize * millerStep, 1));
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
			if (c)
				int b = 10;
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
	float val = XMVectorGetY(l) - XMVectorGetY(pos);
	float newVal = oldVal + val * t ;
	if (newVal > oldVal)
		return;
	pos = XMVectorSetY(pos, newVal);

}

void Miller::Initialize()
{
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
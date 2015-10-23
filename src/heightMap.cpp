#include "heightMap.h"
using namespace std;
using namespace gk2;

const unsigned int HeightMap::VB_STRIDE = sizeof(VertexPosNormal);
const unsigned int HeightMap::VB_OFFSET = 0;

HeightMap::HeightMap(Service& service)
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

	float w = 2;
	float d = 2;
	float h = 0;

	float map_width = map_x / def_map_x *w;
	float map_depth = map_z/ def_map_z *d;
	float map_height = map_y / def_map_y  * h;

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

			float low_heigth = map_height + low_y;
			m_lowMap[lowIndex].x = x;
			m_lowMap[lowIndex].y = low_heigth;
			m_lowMap[lowIndex].z = z;
			lowIndex++;
		}
	}
	int a = 10;
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


VertexPosNormal& HeightMap::GetHeightMapValue(int x, int y)
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
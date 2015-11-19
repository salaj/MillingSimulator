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

	float w = 2;
	float d = 2;
	float h = 1;

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
	if (m_modelsManager != nullptr)
	{
		int n = m_modelsManager->GetBSplineSurfaceCount();
		for (int m = 0; m < n; m++)
		{
			if (m == 3)
				continue;
			BSplineSurface* surface = m_modelsManager->PusnGetBSplineSurface(m);
			//XMFLOAT4 a = surface->Q(0, 0);
			//XMFLOAT4 b = surface->Q(0, 0.2);
			int width = materialWidth;
			int height = materialHeight;
			float factor = 0.7;
			float inverseFactor = 1.0;//1 / factor;
			float offsetZ = 1.0f;

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
			
			//x_start = y_start = 0;
			//x_end = y_end = 150;
			x_len = y_len = 300;
			//XMFLOAT4 c = surface->Q(0, 0);
			//int x = width / 2 * (c.x * factor + 1) * inverseFactor;
			//int y = height / 2 * (c.y * factor + 1) * inverseFactor;
			//float parXLen = 1;//parMax.x - parMin.x;
			//float parYLen = 1;//parMax.y - parMin.y;
			/*parMin.x = 0;
			parMin.y = 0;

			parMax.x = 1;
			parMax.y = 1;*/

			for (int i = 0; i < x_len; i++)
				for (int j = 0; j < y_len; j++)
				{

					/*XMFLOAT4 a = surface->Q(i / (float)(x_len - 1), j / (float)(y_len - 1));*/
					/*XMFLOAT4 a = surface->Q(parMin.x + parXLen * i / (x_len - 1), parMin.y + parYLen * j / (y_len - 1));*/
					XMFLOAT4 a = surface->Q(i / (float)(x_len - 1), j / (float)(y_len - 1));
					float z = (a.z  * factor * inverseFactor + offsetZ);
					//int x = i + x_start + x_offset;
					//int y = j + y_start - y_offset;
					//if (GetHeightMapValue(x, y).Pos.y < z)
						/*GetHeightMapValue(x, y).Pos.y = z;*/
					float x_origin = (width / 2 * (a.x * factor + 1) * inverseFactor);
					float y_origin = (height / 2 * (a.y * factor + 1) * inverseFactor);
					int x = roundf(x_origin); //+ x_offset;
					int y = roundf(y_origin);// -y_offset;
					float len = (x - x_origin) * (x - x_origin) + (y - y_origin)*(y - y_origin);
					/*if (x == 0 && y == 0)
					{
						x_start = x;
						y_start = y;
					}*/
					if (GetHeightMapValue(x, height - y).Pos.y < z)
						if (len < lens[x][height-y])
						{
							lens[x][height-y] = len;
							GetHeightMapValue(x, height - y).Pos.y = z;
						}

						//if (z > 0)
						/*if (GetHeightMapValue(x, height - y).Pos.y == 1)
							GetHeightMapValue(x, height - y).Pos.y = z;
						else
						GetHeightMapValue(x, height - y).Pos.y = (z + GetHeightMapValue(x, height - y).Pos.y ) / (float)2;*/
					int k = 10;
				}
			//for (int b = 0; b < 100; b++)
			//fixSurfaceHoles( x_len,  y_len,  x_start,  y_start,  x_offset, y_offset);
		}
	}

	for (int i = 0; i < materialWidth; i++)
	{
		delete[] lens[i];
	}
	int c = 5;
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
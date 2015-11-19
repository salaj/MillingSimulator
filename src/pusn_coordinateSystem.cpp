#include "pusn_coordinateSystem.h"

using namespace pusn;


CoordinateSystem::CoordinateSystem(PusnService &service) : PusnModelClass(service)
{
	//m_Type = ModelType::CursorType;
	PusnModelClass::Initialize();
	CoordinateSystem::Initialize();
}


CoordinateSystem::CoordinateSystem(const CoordinateSystem& other) : PusnModelClass(other)
{
}


CoordinateSystem::~CoordinateSystem()
{
	PusnModelClass::~PusnModelClass();
}

void CoordinateSystem::initializeAxis(int axis)
{
	float height = 0.1;
	int gridDensity = 20;
	verticesCount = 2 * (gridDensity + 1) + 2;
	indicesCount = verticesCount;
	VertexPosNormal* vertices = new VertexPosNormal[verticesCount];
	unsigned short* indices = new unsigned short[indicesCount];
	int index = 0;
	float val_start = -3;
	float val_end = 3;
	float val_len = val_end - val_start;
	for (int i = 0; i <= gridDensity; i++)
	{
		float val = val_start + i * ((float)val_len / (float)gridDensity);
		if (axis == 0)
		{
			vertices[index].Pos = XMFLOAT3(val, height, 0.0f);
			vertices[index].Normal = XMFLOAT3(0, 0, 1);
			indices[index] = index;
			index++;
			vertices[index].Pos = XMFLOAT3(val, -height, 0.0f);
			vertices[index].Normal = XMFLOAT3(0, 0, 1);
			indices[index] = index;
			index++;
		}
		else if (axis == 1)
		{
			vertices[index].Pos = XMFLOAT3(height, val, 0.0f);
			vertices[index].Normal = XMFLOAT3(0, 0, 1);
			indices[index] = index;
			index++;
			vertices[index].Pos = XMFLOAT3(-height, val, 0.0f);
			vertices[index].Normal = XMFLOAT3(0, 0, 1);
			indices[index] = index;
			index++;
		}
		else if (axis == 2)
		{
			vertices[index].Pos = XMFLOAT3(0.0, height, val);
			vertices[index].Normal = XMFLOAT3(0, 0, 1);
			indices[index] = index;
			index++;
			vertices[index].Pos = XMFLOAT3(0.0, -height, val);
			vertices[index].Normal = XMFLOAT3(0, 0, 1);
			indices[index] = index;
			index++;
		}


	}
	if (axis == 0)
	{
		vertices[index].Pos = XMFLOAT3(val_start, 0.0, 0.0f);
		vertices[index].Normal = XMFLOAT3(0, 0, 1);
		indices[index] = index;
		index++;
		vertices[index].Pos = XMFLOAT3(val_end, 0.0, 0.0f);
		vertices[index].Normal = XMFLOAT3(0, 0, 1);
		indices[index] = index;
		m_vertexBuffer = m_service.Device.CreateVertexBuffer(vertices, verticesCount);
		m_indexBuffer = m_service.Device.CreateIndexBuffer(indices, indicesCount);
	}
	else if (axis == 1)
	{
		vertices[index].Pos = XMFLOAT3(0.0, val_start, 0.0f);
		vertices[index].Normal = XMFLOAT3(0, 0, 1);
		indices[index] = index;
		index++;
		vertices[index].Pos = XMFLOAT3(0.0, val_end, 0.0f);
		vertices[index].Normal = XMFLOAT3(0, 0, 1);
		indices[index] = index;
		m_vertexBuffer_SquareFirst = m_service.Device.CreateVertexBuffer(vertices, verticesCount);
		m_indexBuffer_SquareFirst = m_service.Device.CreateIndexBuffer(indices, indicesCount);
	}
	else if (axis == 2)
	{
		vertices[index].Pos = XMFLOAT3(0.0, 0.0f, val_start);
		vertices[index].Normal = XMFLOAT3(0, 0, 1);
		indices[index] = index;
		index++;
		vertices[index].Pos = XMFLOAT3(0.0, 0.0f, val_end);
		vertices[index].Normal = XMFLOAT3(0, 0, 1);
		indices[index] = index;
		m_vertexBuffer_SquareSecond = m_service.Device.CreateVertexBuffer(vertices, verticesCount);
		m_indexBuffer_SquareSecond = m_service.Device.CreateIndexBuffer(indices, indicesCount);
	}
	delete[] vertices;
	delete[] indices;
}

void CoordinateSystem::Initialize()
{
	// Set the number of vertices in the vertex array.
	m_vertexCount = 4;


	initializeAxis(0);
	initializeAxis(1);
	initializeAxis(2);
}

void CoordinateSystem::setLineTopology()
{

	// Set the number of indices in the index array.
	m_indexCount = 4;

	unsigned short indices[] =
	{
		0, 3, 1, 2,
	};
	m_indexBuffer = m_service.Device.CreateIndexBuffer(indices, m_indexCount);
}

void CoordinateSystem::setTriangleTopology()
{
	//// Set the number of indices in the index array.
	//m_indexCount = 36;

	//unsigned short indices[] =
	//{
	//	0, 1, 2, 0, 2, 3,		//Front face
	//	4, 5, 6, 4, 6, 7,		//Left face
	//	8, 9, 10, 8, 10, 11,	//Botton face
	//	12, 13, 14, 12, 14, 15,	//Back face
	//	16, 17, 18, 16, 18, 19,	//Right face
	//	20, 21, 22, 20, 22, 23	//Top face
	//};
	//m_indexBuffer = m_device.CreateIndexBuffer(indices, m_indexCount);
}

void CoordinateSystem::setStereoscopy(bool isStereoscopic)
{
	m_isStereoscopic = isStereoscopic;
}

void CoordinateSystem::preDraw()
{
	m_service.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	XMFLOAT4 color = XMFLOAT4(1.0, 0.0, 0.0f, 1.0);
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

void CoordinateSystem::onDraw()
{
	ID3D11Buffer* b = m_vertexBuffer.get();
	m_service.Context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE_WITH_NORMAL, &VB_OFFSET);
	m_service.Context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	m_service.Context->DrawIndexed(indicesCount, 0, 0);
	XMFLOAT4 color = XMFLOAT4(0.0, 1.0, 0.0f, 1.0);
	m_service.Context->UpdateSubresource(m_service.cbSurfaceColor.get(), 0, 0, &color, 0, 0);
	b = m_vertexBuffer_SquareFirst.get();
	m_service.Context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE_WITH_NORMAL, &VB_OFFSET);
	m_service.Context->IASetIndexBuffer(m_indexBuffer_SquareFirst.get(), DXGI_FORMAT_R16_UINT, 0);
	m_service.Context->DrawIndexed(indicesCount, 0, 0);
	color = XMFLOAT4(0.0, 0.0, 1.0f, 1.0);
	m_service.Context->UpdateSubresource(m_service.cbSurfaceColor.get(), 0, 0, &color, 0, 0);
	b = m_vertexBuffer_SquareSecond.get();
	m_service.Context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE_WITH_NORMAL, &VB_OFFSET);
	m_service.Context->IASetIndexBuffer(m_indexBuffer_SquareSecond.get(), DXGI_FORMAT_R16_UINT, 0);
	m_service.Context->DrawIndexed(indicesCount, 0, 0);
}

void CoordinateSystem::afterDraw()
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
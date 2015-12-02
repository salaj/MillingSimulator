#include "pusn_pathDrawer.h"

using namespace pusn;


PathsDrawer::PathsDrawer(PusnService &service) : PusnModelClass(service)
{
	//m_Type = ModelType::CursorType;
	PusnModelClass::Initialize();
}


PathsDrawer::PathsDrawer(const PathsDrawer& other) : PusnModelClass(other)
{
}


PathsDrawer::~PathsDrawer()
{
	PusnModelClass::~PusnModelClass();
}

void PathsDrawer::initializeAxis(int axis)
{

}

void PathsDrawer::Feed(vector<pusn::VertexPosNormal>& vertices_container, vector<unsigned short>& indices_container)
{
	pusn::VertexPosNormal* vertices = new pusn::VertexPosNormal[vertices_container.size()];
	unsigned short* indices = new unsigned short[vertices_container.size() * 2 - 2];
	m_parametrizationFirst_indexCount = vertices_container.size() * 2 - 2;
	for (int i = 0; i < vertices_container.size(); i++)
	{
		vertices[i] = vertices_container[i];
		if (i != vertices_container.size() - 1)
		{
			indices[2 * i] = i;
			indices[2 * i + 1] = i + 1;
		}
	}

	m_vertexBuffer_ParametrizationFirst = m_service.Device.CreateVertexBuffer(vertices, vertices_container.size());
	m_indexBuffer_ParametrizationFirst = m_service.Device.CreateIndexBuffer(indices, m_parametrizationFirst_indexCount);
	delete[] vertices;
	delete[] indices;
}

void PathsDrawer::Initialize()
{
}

void PathsDrawer::setLineTopology()
{

	// Set the number of indices in the index array.
	m_indexCount = 4;

	unsigned short indices[] =
	{
		0, 3, 1, 2,
	};
	m_indexBuffer = m_service.Device.CreateIndexBuffer(indices, m_indexCount);
}

void PathsDrawer::setTriangleTopology()
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

void PathsDrawer::setStereoscopy(bool isStereoscopic)
{
	m_isStereoscopic = isStereoscopic;
}

void PathsDrawer::preDraw()
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

void PathsDrawer::onDraw()
{
	return;
	//PATHS
	XMFLOAT4 color = XMFLOAT4(0.0, 0.0, 1.0f, 1.0);
	m_service.Context->UpdateSubresource(m_service.cbSurfaceColor.get(), 0, 0, &color, 0, 0);
	ID3D11Buffer* b = m_vertexBuffer_ParametrizationFirst.get();
	m_service.Context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE_WITH_NORMAL, &VB_OFFSET);
	m_service.Context->IASetIndexBuffer(m_indexBuffer_ParametrizationFirst.get(), DXGI_FORMAT_R16_UINT, 0);
	m_service.Context->DrawIndexed(m_parametrizationFirst_indexCount, 0, 0);
}

void PathsDrawer::afterDraw()
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
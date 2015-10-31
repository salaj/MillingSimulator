#include "gk2_scene.h"
#include "gk2_utils.h"
#include "gk2_vertices.h"
#include "gk2_window.h"
#include <fstream>
#include <iostream>

using namespace std;
using namespace gk2;



#define RESOURCES_PATH L"resources/"
const wstring Scene::ShaderFile = RESOURCES_PATH L"shaders/Puma.hlsl";
const wstring Scene::SceneFiles[6] = {
	RESOURCES_PATH L"Puma/mesh1.txt",
	RESOURCES_PATH L"Puma/mesh2.txt",
	RESOURCES_PATH L"Puma/mesh3.txt",
	RESOURCES_PATH L"Puma/mesh4.txt",
	RESOURCES_PATH L"Puma/mesh5.txt",
	RESOURCES_PATH L"Puma/mesh6.txt"
};

XMFLOAT4 Scene::lightPos = XMFLOAT4(-4, 4, 4, 1);
const unsigned int Scene::VB_STRIDE = sizeof(VertexPosNormal);
const unsigned int Scene::VB_OFFSET = 0;
const unsigned int Scene::BS_MASK = 0xffffffff;


const float Scene::LAP_TIME = 10.0f;

void* Scene::operator new(size_t size)
{
	return Utils::New16Aligned(size);
}

void Scene::operator delete(void* ptr)
{
	Utils::Delete16Aligned(ptr);
}

Scene::Scene(HINSTANCE hInstance)
	: ApplicationBase(hInstance), m_camera(0.01f, 100.0f)
{

}

Scene::~Scene()
{

}

void Scene::InitializeShaders()
{
	shared_ptr<ID3DBlob> vsByteCode = m_device.CompileD3DShader(ShaderFile, "VS_Main", "vs_4_0");
	shared_ptr<ID3DBlob> psByteCode = m_device.CompileD3DShader(ShaderFile, "PS_Main", "ps_4_0");
	m_vertexShader = m_device.CreateVertexShader(vsByteCode);
	m_pixelShader = m_device.CreatePixelShader(psByteCode);
	m_inputLayout = m_device.CreateInputLayout<VertexPosNormal>(vsByteCode);
	vsByteCode = m_device.CompileD3DShader(ShaderFile, "VS_Bilboard", "vs_4_0");
	psByteCode = m_device.CompileD3DShader(ShaderFile, "PS_Bilboard", "ps_4_0");
	/*
	m_vsBilboard = m_device.CreateVertexShader(vsByteCode);
	m_psBilboard = m_device.CreatePixelShader(psByteCode);
	m_ilBilboard = m_device.CreateInputLayout<VertexPos>(vsByteCode);
	*/
}

void Scene::InitializeConstantBuffers()
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(XMMATRIX);
	desc.Usage = D3D11_USAGE_DEFAULT;
	m_cbWorld.reset(new CBMatrix(m_device));
	m_cbProj.reset(new CBMatrix(m_device));
	desc.ByteWidth = sizeof(XMMATRIX) * 2;
	m_cbView.reset(new CBMatrix(m_device));
	desc.ByteWidth = sizeof(XMFLOAT4) * 3;
	m_cbLightPos = m_device.CreateBuffer(desc);
	desc.ByteWidth = sizeof(XMFLOAT4) * 5;
	m_cbLightColors = m_device.CreateBuffer(desc);
	desc.ByteWidth = sizeof(XMFLOAT4);
	m_cbSurfaceColor = m_device.CreateBuffer(desc);

	m_lightPosCB.reset(new ConstantBuffer<XMFLOAT4>(m_device));
	m_surfaceColorCB.reset(new ConstantBuffer<XMFLOAT4>(m_device));
	m_cameraPosCB.reset(new ConstantBuffer<XMFLOAT4>(m_device));
}

void Scene::InitializeRenderStates()
{
	D3D11_DEPTH_STENCIL_DESC dssDesc = m_device.DefaultDepthStencilDesc();
	dssDesc.StencilEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	m_dssWrite = m_device.CreateDepthStencilState(dssDesc);

	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	m_dssTest = m_device.CreateDepthStencilState(dssDesc);

	D3D11_RASTERIZER_DESC rsDesc = m_device.DefaultRasterizerDesc();
	rsDesc.FrontCounterClockwise = true;
	m_rsCounterClockwise = m_device.CreateRasterizerState(rsDesc);

	D3D11_BLEND_DESC bsDesc = m_device.DefaultBlendDesc();
	bsDesc.RenderTarget[0].BlendEnable = true;
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_bsAlpha = m_device.CreateBlendState(bsDesc);
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_COLOR;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_bsAdd = m_device.CreateBlendState(bsDesc);


	D3D11_DEPTH_STENCIL_DESC dssDesc2 = m_device.DefaultDepthStencilDesc();
	dssDesc2.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dssNoWrite = m_device.CreateDepthStencilState(dssDesc2);


	D3D11_RASTERIZER_DESC rsDesc3 = m_device.DefaultRasterizerDesc();
	rsDesc3.CullMode = D3D11_CULL_FRONT;
	m_rsCullFace = m_device.CreateRasterizerState(rsDesc3);
	rsDesc3.CullMode = D3D11_CULL_BACK;
	m_rsCullBack = m_device.CreateRasterizerState(rsDesc3);
	D3D11_DEPTH_STENCIL_DESC dssDesc3 = m_device.DefaultDepthStencilDesc();
	dssDesc3.DepthEnable = true;
	dssDesc3.StencilEnable = true;
	dssDesc3.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dssDesc3.DepthFunc = D3D11_COMPARISON_LESS;
	//dssDesc3.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dssDesc3.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dssDesc3.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;

	m_dssShadowsOne = m_device.CreateDepthStencilState(dssDesc3);


	//dssDesc3.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dssDesc3.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dssDesc3.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;

	m_dssShadowsSecond = m_device.CreateDepthStencilState(dssDesc3);
}

void Scene::InitializeCamera()
{
	SIZE s = getMainWindow()->getClientSize();
	float ar = static_cast<float>(s.cx) / s.cy;
	m_projMtx = XMMatrixPerspectiveFovRH(XM_PIDIV4, ar, 0.01f, 100.0f);
	m_cbProj->Update(m_context, m_projMtx);
	m_camera.Zoom(7);

	UpdateCamera(m_camera.GetViewMatrix());
	//XMMATRIX view;
	//m_camera.GetViewMatrix(view);
	//m_cbView->Update(m_context, view);
	//m_cameraPosCB->Update(m_context, m_camera.GetPosition());
}


void Scene::InitializeRoom()
{
	float size = 10.0f;
	VertexPosNormal vertices[] =
	{
		//Front face
		{ XMFLOAT3(-size, -1.0f, size), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(size, -1.0f, size), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(size, size, size), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-size, size, size), XMFLOAT3(0.0f, 0.0f, -1.0f) },

		//Left face
		{ XMFLOAT3(-size, -1.0f, size), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-size, size, size), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-size, size, -size), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-size, -1.0f, -size), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		//Bottom face
		{ XMFLOAT3(-size, -1.0f, size), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-size, -1.0f, -size), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(size, -1.0f, -size), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(size, -1.0f, size), XMFLOAT3(0.0f, 1.0f, 0.0f) },

		//Back face
		{ XMFLOAT3(-size, -1.0f, -size), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-size, size, -size), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(size, size, -size), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(size, -1.0f, -size), XMFLOAT3(0.0f, 0.0f, 1.0f) },

		//Right face
		{ XMFLOAT3(size, -1.0f, size), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(size, -1.0f, -size), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(size, size, -size), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(size, size, size), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		//Top face
		{ XMFLOAT3(-size, size, size), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(size, size, size), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(size, size, -size), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-size, size, -size), XMFLOAT3(0.0f, -1.0f, 0.0f) },
	};
	m_vbRoom = m_device.CreateVertexBuffer(vertices, 24);
	unsigned short indices[] =
	{
		0, 1, 2, 0, 2, 3,		//Front face
		4, 5, 6, 4, 6, 7,		//Left face
		8, 9, 10, 8, 10, 11,	//Botton face
		12, 13, 14, 12, 14, 15,	//Back face
		16, 17, 18, 16, 18, 19,	//Right face
		20, 21, 22, 20, 22, 23	//Top face
	};
	m_ibRoom = m_device.CreateIndexBuffer(indices, 36);
}
void Scene::InitializeShadowEffects()
{

	m_phongEffect.reset(new PhongEffect(m_device, m_layout));
	m_phongEffect->SetProjMtxBuffer(m_cbProj);
	m_phongEffect->SetViewMtxBuffer(m_cbView);
	m_phongEffect->SetWorldMtxBuffer(m_cbWorld);
	m_phongEffect->SetLightPosBuffer(m_lightPosCB);
	m_phongEffect->SetSurfaceColorBuffer(m_surfaceColorCB);

	m_lightShadowEffect.reset(new LightShadowEffect(m_device, m_layout));
	m_lightShadowEffect->SetProjMtxBuffer(m_cbProj);
	m_lightShadowEffect->SetViewMtxBuffer(m_cbView);
	m_lightShadowEffect->SetWorldMtxBuffer(m_cbWorld);
	m_lightShadowEffect->SetLightPosBuffer(m_lightPosCB);
	m_lightShadowEffect->SetSurfaceColorBuffer(m_surfaceColorCB);
	m_lightShadowEffect->UpdateLight(0.0f, m_context);
}

void Scene::InitializePlane()
{
	float size = 3.0f;
	float sizeZ = 2.0f;
	VertexPosNormal vertices[] =
	{
		{ XMFLOAT3(-0.9f, -1.0f, sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.9f, -1.0f, -sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.9f - (size / 2.0f), (size / 2.0f)* sqrt(3) - 1.0f, sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.9f - (size / 2.0f), (size / 2.0f)* sqrt(3) - 1.0f, -sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) }
	};
	m_vbPlane = m_device.CreateVertexBuffer(vertices, 8);
	unsigned short indices[] =
	{
		0, 1, 2, 1, 3, 2,
		0, 2, 1, 1, 2, 3
	};
	m_ibPlane = m_device.CreateIndexBuffer(indices, 12);
}

void Scene::InitializeScene()
{
	for (int i = 0; i < 6; i++)
	{
		ifstream file;
		file.open(SceneFiles[i]);
		int differentVertexPositionCount = 0;
		int differentVertexCount = 0;
		int trianglesCount = 0;

		file >> differentVertexPositionCount;
		for (int j = 0; j < differentVertexPositionCount; j++)
		{
			float x, y, z;
			file >> x >> y >> z;
			vertexes[i].push_back(XMFLOAT3(x, y, z));
		}

		file >> differentVertexCount;
		for (int j = 0; j < differentVertexCount; j++)
		{
			int ind;
			float x, y, z;
			file >> ind >> x >> y >> z;
			VertexPosNormal v;
			v.Pos = vertexes[i][ind];
			v.Normal = XMFLOAT3(x, y, z);
			vertices[i].push_back(v);
		}

		file >> trianglesCount;
		for (int j = 0; j < trianglesCount; j++)
		{
			unsigned short ind1, ind2, ind3;
			file >> ind1 >> ind2 >> ind3;
			indices[i].push_back(ind1);
			indices[i].push_back(ind2);
			indices[i].push_back(ind3);
			triangles[i].push_back(vector < int > { ind1, ind2, ind3 });
		}
		int edgesCount;
		file >> edgesCount;
		for (int j = 0; j < edgesCount; j++)
		{
			int v1, v2, t1, t2;
			file >> v1 >> v2 >> t1 >> t2;
			edges[i].push_back(vector < int > { v1, v2, t1, t2 });
		}
		m_vbScene[i] = m_device.CreateVertexBuffer(&vertices[i][0], differentVertexCount, D3D11_USAGE_DYNAMIC);
		//m_vbScene[i] = m_device.CreateVertexBuffer(&vertices[i][0], differentVertexCount);
		m_ibScene[i] = m_device.CreateIndexBuffer(&indices[i][0], 3 * trianglesCount);
		SceneIndicesCount[i] = 3 * trianglesCount;
		m_SceneMtx[i] = XMMatrixIdentity();
	}

}
void Scene::InitializeCyllinder()
{
	//ilosc punktow przyblizaj¹cych okrag
	int verticesAmount = 120;
	cyllinderVertices = new VertexPosNormal[2 * verticesAmount];

	//k¹t o jaki bêd¹ obracane punkty
	float delta = XM_2PI / (float)verticesAmount;

	XMFLOAT3 normal = XMFLOAT3(0, 0, 1);

	//wyznaczanie punktów
	for (int t = 0; t < verticesAmount; t++)
	{

		XMVECTOR pos = XMLoadFloat3(
			&XMFLOAT3(
			circleRadius * cos(t * delta),
			circleRadius * sin(t * delta),
			0));

		pos = XMVector3Transform(pos, XMMatrixRotationY(XM_PIDIV2));
		pos = XMVector3Transform(pos, XMMatrixRotationX(XM_PI));
		pos = XMVector3Transform(pos, XMMatrixTranslation(-0.5f, -0.5f, 1.0f));


		cyllinderVertices[t].Pos = XMFLOAT3(XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));
		XMStoreFloat3(&cyllinderVertices[t].Normal, (XMVectorSet(0.0f, 1.0f, -1.0f, 1.0f)));
	}

	for (int t = verticesAmount; t < 2 * verticesAmount; t++)
	{

		XMVECTOR pos = XMLoadFloat3(
			&XMFLOAT3(
			circleRadius * cos(t * delta),
			circleRadius * sin(t * delta),
			0));

		pos = XMVector3Transform(pos, XMMatrixRotationY(XM_PIDIV2));
		pos = XMVector3Transform(pos, XMMatrixRotationX(XM_PI));
		pos = XMVector3Transform(pos, XMMatrixTranslation(2.5f, -0.5f, 1.0f));

		cyllinderVertices[t].Pos = XMFLOAT3(XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));
		XMStoreFloat3(&cyllinderVertices[t].Normal, (XMVectorSet(0.0f, 1.0f, -1.0f, 1.0f)));
	}
	m_vbCyllinder = m_device.CreateVertexBuffer(cyllinderVertices, 2 * verticesAmount);

	int indicesAmount = verticesAmount * 2;
	unsigned short* indices = new unsigned short[indicesAmount + 4];
	int counter = 0;
	for (int i = 0; i < indicesAmount; i += 2)
	{
		indices[i] = counter + verticesAmount;
		indices[i + 1] = counter;
		counter++;
	}
	//indices[indicesAmount] = verticesAmount;
	//indices[indicesAmount + 1] = 0;
	//indices[indicesAmount + 2] = 0;
	//indices[indicesAmount + 2] = counter + verticesAmount;

	m_ibCyllinder = m_device.CreateIndexBuffer(indices, indicesAmount);

}

void Scene::InitializeCircle()
{
	//ilosc punktow przyblizaj¹cych okrag
	int verticesAmount = 120;
	circleVertices = new VertexPosNormal[verticesAmount];

	//k¹t o jaki bêd¹ obracane punkty
	float delta = XM_2PI / (float)verticesAmount;

	//wyznaczanie punktów
	for (int t = 0; t < verticesAmount; t++)
	{

		XMVECTOR pos = XMLoadFloat3(
			&XMFLOAT3(
			circleRadius * cos(t * delta),
			circleRadius * sin(t * delta),
			0));

		pos = XMVector3Transform(pos, XMMatrixRotationY(XM_PIDIV2));
		pos = XMVector3Transform(pos, XMMatrixRotationZ(XM_PI / 6.0f));
		pos = XMVector3Transform(pos, XMMatrixTranslation(circleCenter.x, circleCenter.y, 0.0f));

		circleVertices[t].Pos = XMFLOAT3(XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));
		circleVertices[t].Normal = XMFLOAT3(sqrt(3) / 2.0f, 0.5f, 0.0f);
	}

	m_vbCircle = m_device.CreateVertexBuffer(circleVertices, verticesAmount);

	int indicesAmount = verticesAmount * 2;
	unsigned short* indices = new unsigned short[indicesAmount];
	int counter = 0;
	for (int i = 0; i < indicesAmount - 2; i += 2)
	{
		indices[i] = counter++;
		indices[i + 1] = counter;
	}
	indices[indicesAmount - 2] = counter;
	indices[indicesAmount - 1] = 0;

	m_ibCircle = m_device.CreateIndexBuffer(indices, indicesAmount);

	//delete[] vertices;
}

struct Quaternion
{
	float q[4];
	static Quaternion IDENTITY;
};
struct Vector3
{
	float v[3];
	static Vector3 ZERO;
};
struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	unsigned int AmbientColor;
};

// Scene globals
Quaternion g_SpongeRotation;                 // model rotation, set by InitScene
int x_mesh = 150;
int y_mesh = 50;
int z_mesh = 150;
int precision_mesh = 100;
int animation_speed = 100;
int max_speed = 101;
bool isMaterialActive = false;
bool g_SpongeAO = true;                      // apply ambient occlusion
unsigned int g_SpongeIndicesCount = 0;       // set by BuildSponge
Vector3 g_LightDir = { -0.5f, -0.2f, 1 };      // light direction vector
float g_CamDistance = 0.7f;                  // camera distance
float g_BackgroundColor[] = { 0, 0, 0.5f, 1 }; // background color
bool g_Animate = true;                       // enable animation
float g_AnimationSpeed = 0.2f;               // animation speed


void TW_CALL SetPrecisionMesh(const void *value, void * a)
{
	precision_mesh = *static_cast<const int *>(value);
	container* c = static_cast<container*>(a);
	c->heightMap->materialHeight = precision_mesh;
	c->heightMap->materialWidth = precision_mesh;

	c->heightMap->Initialize();
	c->miller->Reset(c->heightMap);
}

void TW_CALL SetAnimationSpeed(const void *value, void * a)
{
	animation_speed = *static_cast<const int *>(value);
}


void TW_CALL SetXMesh(const void *value, void * a)
{
	x_mesh = *static_cast<const int *>(value);
	container* c = static_cast<container*>(a);
	c->heightMap->map_x = x_mesh;

	c->heightMap->Initialize();
	c->miller->Reset(c->heightMap);
}

void TW_CALL SetYMesh(const void *value, void * a)
{
	y_mesh = *static_cast<const int *>(value);
	container* c = static_cast<container*>(a);
	c->heightMap->map_y = y_mesh;

	c->heightMap->Initialize();
	c->miller->Reset(c->heightMap);
}

void TW_CALL SetZMesh(const void *value, void * a)
{
	z_mesh = *static_cast<const int *>(value);
	container* c = static_cast<container*>(a);
	c->heightMap->map_z = z_mesh;

	c->heightMap->Initialize();
	c->miller->Reset(c->heightMap);
}


// Callback function called by AntTweakBar to get the sponge recursion level
void TW_CALL GetXMesh(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = x_mesh;
}

void TW_CALL GetYMesh(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = y_mesh;
}

void TW_CALL GetZMesh(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = z_mesh;
}

void TW_CALL GetPrecisionMesh(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = precision_mesh;
}

void TW_CALL GetAnimationSpeed(void *value, void * /*clientData*/)
{
	*static_cast<int *>(value) = animation_speed;
}


// Callback function called by AntTweakBar to enable/disable ambient occlusion
void TW_CALL SetSpongeAOCB(const void *value, void * /*clientData*/)
{
	g_SpongeAO = *static_cast<const bool *>(value);
	//BuildSponge(g_SpongeLevel, g_SpongeAO);
}


// Callback function called by AntTweakBar to get ambient occlusion state
void TW_CALL GetSpongeAOCB(void *value, void * /*clientData*/)
{
	*static_cast<bool *>(value) = g_SpongeAO;
}


void TW_CALL RunCB(void * a)
{
	container* c = static_cast<container*>(a);
	Window* mainWindow = c->mainWindow;
	//Window* mainWindow = static_cast<Window*>(a);
	OPENFILENAME ofn = { 0 };
	//TCHAR szFilters[] = TEXT("Model geometry files (*.mg1)\0*.mg1\0\0");
	TCHAR szFilePathName[_MAX_PATH] = TEXT("");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = mainWindow->getHandle();
	//ofn.lpstrFilter = szFilters;
	ofn.lpstrFile = szFilePathName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = TEXT("Open File");
	ofn.Flags = OFN_FILEMUSTEXIST;
	GetOpenFileName(&ofn);
	wstring s = (wstring)(ofn.lpstrFile);
	if (s.size() <= 3)
		return;
	c->reader->LoadPaths(s);
	c->miller->m_radius = (c->reader->millerSize) / c->heightMap->def_map_x;
	c->miller->type = c->reader->millerType;
	c->miller->Initialize();
	int breakpoint = 10;
}


void TW_CALL ToogleAnimation(void * a)
{
	bool* toogle = static_cast<bool*>(a);
	if (*toogle)
		*toogle = false;
	else
		*toogle = true;
}

void Scene::SetShaders()
{
	m_context->VSSetShader(m_vertexShader.get(), 0, 0);
	m_context->PSSetShader(m_pixelShader.get(), 0, 0);
	m_context->IASetInputLayout(m_inputLayout.get());
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Scene::SetConstantBuffers()
{
	ID3D11Buffer* vsb[] = { m_cbWorld->getBufferObject().get(), m_cbView->getBufferObject().get(), m_cbProj->getBufferObject().get(), m_cbLightPos.get() };
	m_context->VSSetConstantBuffers(0, 4, vsb);
	ID3D11Buffer* psb[] = { m_cbLightColors.get(), m_cbSurfaceColor.get() };
	m_context->PSSetConstantBuffers(0, 2, psb);

}

bool Scene::LoadContent()
{

	InitializeShaders();
	InitializeConstantBuffers();
	InitializeRenderStates();
	InitializeCamera();

	InitializeRoom();
	InitializeScene();
	InitializePlane();
	InitializeCircle();
	InitializeCyllinder();
	InitializeShadowEffects();

	Service service;
	service.Context = m_context;
	service.Device = m_device;
	service.cbLightColors = m_cbLightColors;
	service.cbLightPos = m_cbLightPos;
	service.cbSurfaceColor = m_cbSurfaceColor;



	m_coordinateSystem = new CoordinateSystem(service);
	m_miller = new Miller(service);
	m_HeightMap = new HeightMap(service);
	m_miller->Reset(m_HeightMap);

	wstring file = L"C:\\Users\\Qba\\Documents\\Studia magisterskie\\semestr_2\\PUSN\\SymulatorFrezarki3C\\paths\\t1.k16";
	m_reader.LoadPaths(file);
	m_miller->m_radius = m_reader.millerSize / m_HeightMap->def_map_x;
	m_miller->type = m_reader.millerType;
	m_miller->SetPosition(0, 0.2, 0);
	m_miller->Initialize();

	/*m_miller->m_radius = 0.1f;
	m_miller->type = millerType::Sphere;
	m_miller->Initialize();*/
	animationPending = true;

	x_mesh = m_HeightMap->map_x;
	y_mesh = m_HeightMap->map_y;
	z_mesh = m_HeightMap->map_z;
	precision_mesh = m_HeightMap->materialWidth;



	m_particles.reset(new ParticleSystem(m_device, XMFLOAT3(-1.0f, -1.1f, 0.46f)));
	m_particles->SetViewMtxBuffer(m_cbView);
	m_particles->SetProjMtxBuffer(m_cbProj);

	SetShaders();
	SetConstantBuffers();




	// Create a tweak bar
	TwBar *bar = TwNewBar("TweakBar");
	TwDefine(" GLOBAL help='No help :)' "); // Message added to the help bar.
	int barSize[2] = { 300, 320 };
	TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);

	// Add variables to the tweak bar
	//getMainWindow()
	//container* c = new container();
	//c->mainWindow = getMainWindow();
	//c->reader = &m_reader;
	TwAddButton(bar, "Run", RunCB, (new container(getMainWindow(), &m_reader, m_miller, m_HeightMap)), " label='Otwórz plik' ");

	TwAddVarCB(bar, "Szerokosc siatki (X)", TW_TYPE_INT32, SetXMesh, GetXMesh, new container(getMainWindow(), &m_reader, m_miller, m_HeightMap), "min=10 max=300 group='Wymiary siatki'");
	TwAddVarCB(bar, "Szerokosc siatki (Y)", TW_TYPE_INT32, SetYMesh, GetYMesh, new container(getMainWindow(), &m_reader, m_miller, m_HeightMap), "min=10 max=300 group='Wymiary siatki'");
	TwAddVarCB(bar, "Szerokosc siatki (Z)", TW_TYPE_INT32, SetZMesh, GetZMesh, new container(getMainWindow(), &m_reader, m_miller, m_HeightMap), "min=10 max=300 group='Wymiary siatki'");

	TwAddVarCB(bar, "Dokladnosc siatki", TW_TYPE_INT32, SetPrecisionMesh, GetPrecisionMesh, new container(getMainWindow(), &m_reader, m_miller, m_HeightMap), "min=10 max=180 group='Dokladnosc siatki'");

	TwAddButton(bar, "Animation", ToogleAnimation, &animationPending, " label='Start/Pauza' group='Sterowanie aplikacji'");
	TwAddVarCB(bar, "Szybkosc animacji", TW_TYPE_INT32, SetAnimationSpeed, GetAnimationSpeed, &animation_speed, "min=10 max=100 group='Sterowanie aplikacji'");
	TwAddVarRW(bar, "Wyswietlanie materialu", TW_TYPE_BOOLCPP, &isMaterialActive, "group='Sterowanie aplikacji");

	//TwAddVarCB(bar, "Ambient Occlusion", TW_TYPE_BOOLCPP, SetSpongeAOCB, GetSpongeAOCB, NULL, "group=Sponge key=o");
	//TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &g_SpongeRotation, "opened=true axisz=-z group=Sponge");
	//TwAddVarRW(bar, "Animation", TW_TYPE_BOOLCPP, &g_Animate, "group=Sponge key=a");
	//TwAddVarRW(bar, "Animation speed", TW_TYPE_FLOAT, &g_AnimationSpeed, "min=-10 max=10 step=0.1 group=Sponge keyincr=+ keydecr=-");
	//TwAddVarRW(bar, "Light direction", TW_TYPE_DIR3F, &g_LightDir, "opened=true axisz=-z showval=true");
	//TwAddVarRW(bar, "Camera distance", TW_TYPE_FLOAT, &g_CamDistance, "min=0 max=4 step=0.01 keyincr=PGUP keydecr=PGDOWN");
	//TwAddVarRW(bar, "Background", TW_TYPE_COLOR4F, &g_BackgroundColor, "colormode=hls");


	return true;
}

void Scene::UnloadContent()
{
	m_vertexShader.reset();
	m_pixelShader.reset();
	m_inputLayout.reset();



	m_vbRoom.reset();
	m_ibRoom.reset();


	m_cbWorld.reset();
	m_cbView.reset();
	m_cbProj.reset();
	m_cbLightPos.reset();
	m_cbLightColors.reset();
	m_cbSurfaceColor.reset();
	TwTerminate();
}

void Scene::UpdateCamera(const XMMATRIX& view)
{
	XMMATRIX viewMtx[2];
	viewMtx[0] = view;
	XMVECTOR det;
	viewMtx[1] = XMMatrixInverse(&det, viewMtx[0]);
	//m_context->UpdateSubresource(m_cbView->getBufferObject().get(), 0, 0, viewMtx, 0, 0);
	m_cbView->Update(m_context, view);
}

void Scene::SetLight0()
//Setup one positional light at the camera
{
	XMFLOAT4 positions[3];
	ZeroMemory(positions, sizeof(XMFLOAT4) * 3);
	positions[0] = lightPos;// m_camera.GetPosition();
	//positions[1] = XMFLOAT4(-2, -2, -2, 1);//m_camera.GetPosition();
	//positions[2] = XMFLOAT4(0, 0, -10, 1);//m_camera.GetPosition();
	m_context->UpdateSubresource(m_cbLightPos.get(), 0, 0, positions, 0, 0);

	XMFLOAT4 colors[5];
	ZeroMemory(colors, sizeof(XMFLOAT4) * 5);
	colors[0] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); //ambient color
	colors[1] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); //surface [ka, kd, ks, m]
	colors[2] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	//colors[3] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	//colors[4] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); //light0 color
	m_context->UpdateSubresource(m_cbLightColors.get(), 0, 0, colors, 0, 0);
}

void Scene::DrawCoordinateSystem()
{
	m_coordinateSystem->Draw();
}


void Scene::DrawMiller()
{
	//XMVECTOR s = XMVectorSet(m_miller->StartMillerPosition.x, m_miller->StartMillerPosition.y, 0, 1);
	//XMVECTOR e = XMVectorSet(m_miller->EndMillerPosition.x, m_miller->EndMillerPosition.y, 0, 1);
	//XMVECTOR offset = (e - s) / 1000;
	//m_miller->Translate(XMFLOAT4(XMVectorGetX(offset), 0, XMVectorGetY(offset), 1));
	XMMATRIX worldMtx;
	m_miller->GetModelMatrix(worldMtx);
	m_cbWorld->Update(m_context, worldMtx);
	m_miller->Draw();
	m_cbWorld->Update(m_context, XMMatrixIdentity());
}

void Scene::DrawRoom()
{
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	const XMMATRIX worldMtx = XMMatrixIdentity();
	m_cbWorld->Update(m_context, worldMtx);
	ID3D11Buffer* b = m_vbRoom.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibRoom.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->DrawIndexed(36, 0, 0);
}

void Scene::DrawPlane(bool val)
{
	const XMMATRIX worldMtx = XMMatrixIdentity();
	m_cbWorld->Update(m_context, worldMtx);

	m_context->UpdateSubresource(m_cbSurfaceColor.get(), 0, 0, &XMFLOAT4(1, 1, 1, 1.0), 0, 0);

	ID3D11Buffer* b = m_vbPlane.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibPlane.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->DrawIndexed(12, 0, 0);
}
void Scene::DrawPuma()
{
	for (int i = 0; i < 6; i++)
	{
		const XMMATRIX worldMtx = m_SceneMtx[i];
		m_cbWorld->Update(m_context, worldMtx);
		ID3D11Buffer* b = m_vbScene[i].get();
		m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
		m_context->IASetIndexBuffer(m_ibScene[i].get(), DXGI_FORMAT_R16_UINT, 0);
		m_context->DrawIndexed(SceneIndicesCount[i], 0, 0);
	}
}

void Scene::DrawCube()
{
	//const XMMATRIX worldMtx = XMMatrixIdentity();
	//m_cbWorld->Update(m_context, XMMatrixTranslation(-1.0, 0, 0));
	m_HeightMap->Draw(isMaterialActive);
}

void Scene::DrawCircle()
{
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	const XMMATRIX worldMtx = XMMatrixIdentity();
	m_cbWorld->Update(m_context, worldMtx);
	ID3D11Buffer* b = m_vbCircle.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibCircle.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->DrawIndexed(720, 0, 0);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Scene::DrawCyllinder()
{
	m_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	const XMMATRIX worldMtx = XMMatrixIdentity();
	m_cbWorld->Update(m_context, worldMtx);
	ID3D11Buffer* b = m_vbCyllinder.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibCyllinder.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->DrawIndexed(244, 0, 0);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Scene::DrawMirroredWorld()
{
	//Setup render state for writing to the stencil buffer
	m_context->OMSetDepthStencilState(m_dssWrite.get(), 1);
	//Draw the i-th face
	m_cbWorld->Update(m_context, &XMMatrixIdentity());
	ID3D11Buffer* b = m_vbPlane.get();
	m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_context->IASetIndexBuffer(m_ibPlane.get(), DXGI_FORMAT_R16_UINT, 0);
	m_context->DrawIndexed(12, 0, 0);

	//Setup render state and view matrix for rendering the mirrored world
	m_context->OMSetDepthStencilState(m_dssTest.get(), 1);


	XMMATRIX scale = XMMatrixScaling(1, 1, -1);
	XMVECTOR det;
	XMMATRIX m_mirrorMtx;
	XMMATRIX m = XMMatrixRotationY(XM_PIDIV2) * XMMatrixRotationZ(XM_PI / 6) * XMMatrixTranslation(-1.48, 0, 0);// *XMMatrixRotationZ(-XM_PI / 6);
	m_mirrorMtx = XMMatrixInverse(&det,
		m)
		* scale *
		m;

	XMMATRIX viewMtx;
	m_camera.GetViewMatrix(viewMtx);
	XMMATRIX mirrorViewMtx = XMMatrixMultiply(m_mirrorMtx, viewMtx);
	UpdateCamera(mirrorViewMtx);
	m_context->RSSetState(m_rsCounterClockwise.get());

	SetLight0();
	DrawRoom();
	//DrawPlane(true);
	DrawPuma();
	DrawCircle();
	DrawCyllinder();
	m_particles->Render(m_context, XMFLOAT3());
	SetShaders();
	SetConstantBuffers();
	m_context->RSSetState(NULL);

	//Restore rendering state to it's original values
	UpdateCamera(viewMtx);
	m_context->OMSetDepthStencilState(NULL, 0);
}

void Scene::inverse_kinematics(XMFLOAT3 pos, XMFLOAT3 normal, float &a1, float &a2,
	float &a3, float &a4, float &a5)
{
	float l1 = .91f, l2 = .81f, l3 = .33f, dy = .27f, dz = .26f;
	float normalizationFactor = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	normal = XMFLOAT3(normal.x / normalizationFactor, normal.y / normalizationFactor, normal.z / normalizationFactor);
	XMFLOAT3 pos1 = XMFLOAT3(pos.x + normal.x * l3, pos.y + normal.y * l3, pos.z + normal.z * l3);
	float e = sqrtf(pos1.z*pos1.z + pos1.x*pos1.x - dz*dz);
	a1 = atan2(pos1.z, -pos1.x) + atan2(dz, e);
	XMFLOAT3 pos2(e, pos1.y - dy, .0f);
	a3 = -acosf(min(1.0f, (pos2.x*pos2.x + pos2.y*pos2.y - l1*l1 - l2*l2)
		/ (2.0f*l1*l2)));
	float k = l1 + l2 * cosf(a3), l = l2 * sinf(a3);
	a2 = -atan2(pos2.y, sqrtf(pos2.x*pos2.x + pos2.z*pos2.z)) - atan2(l, k);
	XMFLOAT3 normal1;
	XMVECTOR a = XMVector3Transform(XMVectorSet(normal.x, normal.y, normal.z, 0.0f), XMMatrixRotationY(-a1));
	normal1 = XMFLOAT3(XMVectorGetX(a), XMVectorGetY(a), XMVectorGetZ(a));


	XMVECTOR b = XMVector3Transform(XMVectorSet(normal1.x, normal1.y, normal1.z, 0.0f), XMMatrixRotationZ(-(a2 + a3)));
	normal1 = XMFLOAT3(XMVectorGetX(b), XMVectorGetY(b), XMVectorGetZ(b));


	a5 = acosf(normal1.x);
	a4 = atan2(normal1.z, normal1.y);
}


void Scene::UpdateScene(float dt)
{
	static float counter = 0;
	static float lap = 0.0f;
	lap += dt;
	while (lap > LAP_TIME)
		lap -= LAP_TIME;
	float t = 2 * lap / LAP_TIME;
	t *= XM_2PI;

	XMFLOAT3 norm = XMFLOAT3(sqrtf(3) / 2.0f, 0.5f, 0.0f);

	float a1, a2, a3, a4, a5;
	XMFLOAT3 p = circleVertices[((int)counter) % 120].Pos;
	counter += 0.1f;
	m_particles.get()->m_emitterPos = p;
	XMVECTOR rVec = XMLoadFloat3(&p) - XMLoadFloat3(&XMFLOAT3(circleCenter.x, circleCenter.y, 0.0f));
	m_particles.get()->m_perpendicularToPlane = rVec;
	m_particles.get()->m_startPosition = p;
	inverse_kinematics(p, norm, a1, a2, a3, a4, a5);
	//a1 = 0;
	vector<VertexPosNormal> newVertices[6];
	for (int i = 1; i < 6; i++)
	{
		XMMATRIX tr;
		switch (i)
		{
		case 1:
			m_SceneMtx[i] = XMMatrixRotationY(a1);
			break;
		case 2:
			m_SceneMtx[i] = XMMatrixTranslation(0, -0.27f, 0) * XMMatrixRotationZ(a2) * XMMatrixTranslation(0, 0.27f, 0) * XMMatrixRotationY(a1);
			break;
		case 3:
			m_SceneMtx[i] = XMMatrixTranslation(0, -0.27f, 0) *XMMatrixTranslation(0.91f, 0, 0) * XMMatrixRotationZ(a3) *
				XMMatrixTranslation(-0.91f, 0, 0)  * XMMatrixRotationZ(a2) *XMMatrixTranslation(0, 0.27f, 0) * XMMatrixRotationY(a1);
			break;
		case 4:
			m_SceneMtx[i] = XMMatrixTranslation(0, -0.27f, 0) *
				XMMatrixTranslation(0, 0, 0.26f) *XMMatrixRotationX(a4) * XMMatrixTranslation(0, 0, -0.26f) *XMMatrixTranslation(+0.91f, 0, 0)
				*XMMatrixRotationZ(a3) * XMMatrixTranslation(-0.91f, 0, 0) *XMMatrixRotationZ(a2) *XMMatrixTranslation(0, 0.27f, 0) *
				XMMatrixRotationY(a1);
			break;
		case 5:
			m_SceneMtx[i] = XMMatrixTranslation(0, -0.27f, 0)* XMMatrixTranslation(1.72f, 0, 0) *XMMatrixRotationZ(a5) * XMMatrixTranslation(-1.72f, 0, 0) *
				XMMatrixTranslation(0, 0, 0.26f) * XMMatrixRotationX(a4) *XMMatrixTranslation(0, 0, -0.26f) * XMMatrixTranslation(+0.91f, 0, 0)
				*	XMMatrixRotationZ(a3)* XMMatrixTranslation(-0.91f, 0, 0) *XMMatrixRotationZ(a2)*
				XMMatrixTranslation(0, 0.27f, 0) *XMMatrixRotationY(a1);
			break;
		}
	}
}

void Scene::UpdateInput()
{
	static KeyboardState state;
	if (!m_keyboard->GetState(state))
		return;
	float factor = 0.01f;
	if (state.isKeyDown(DIK_W))
	{
		m_camera.UpdatePosition(XMVector3Normalize(-m_camera.camTarget));
	}
	if (state.isKeyDown(DIK_S))
	{
		m_camera.UpdatePosition(XMVector3Normalize(m_camera.camTarget));
	}
	if (state.isKeyDown(DIK_A))
	{
		m_camera.UpdatePosition(XMVector3Normalize(XMVector3Cross(XMVector3Normalize(m_camera.camTarget), m_camera.camUp)));
	}
	if (state.isKeyDown(DIK_D))
	{
		m_camera.UpdatePosition(XMVector3Normalize(XMVector3Cross(m_camera.camUp, XMVector3Normalize(m_camera.camTarget))));
	}
	if (state.isKeyDown(DIK_Z))
	{
		m_camera.UpdatePosition(XMVector3Normalize(m_camera.camUp));
	}
	if (state.isKeyDown(DIK_X))
	{
		m_camera.UpdatePosition(XMVector3Normalize(-m_camera.camUp));
	}
}
// Procedura dialogowa
BOOL CALLBACK DlgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_COMMAND:
	{
		// reakcja na przyciski
		switch (LOWORD(wParam))
		{
		case IDOK: EndDialog(hwnd, IDOK); break;
		case IDCANCEL: EndDialog(hwnd, IDCANCEL); break;
		}
	}
	break;

	default: return FALSE;
	}

	return TRUE;
}

void Scene::ShowPopup()
{
	MessageBox(NULL, L"Frez próbuje przekroczyæ wyskoœæ podstawki.", NULL, NULL);
}

bool once = false;
int timer = 0;
int max_time = 1000;
XMFLOAT3 StartMillerPosition, EndMillerPosition, CoreEndMillerPosition;
XMVECTOR s, e, step;
void Scene::UpdateCube(float dt)
{
	if (m_reader.pathIndex >= m_reader.paths.size())
		return;


	if (timer == 0)
	{
		if (m_reader.pathIndex == 0)
		{
			int index = m_reader.pathIndex;
			StartMillerPosition = m_reader.paths[index].Pos;
			CoreEndMillerPosition = m_reader.paths[index + 1].Pos;
		}
		else
		{
			int index = m_reader.pathIndex;
			StartMillerPosition = m_reader.paths[index - 1].Pos;
			CoreEndMillerPosition = m_reader.paths[index].Pos;
		}
		s = XMVectorSet(StartMillerPosition.x, StartMillerPosition.y, StartMillerPosition.z, 1);
		e = XMVectorSet(CoreEndMillerPosition.x, CoreEndMillerPosition.y, CoreEndMillerPosition.z, 1);
		step = (e - s) / (max_speed - animation_speed);
		if (abs(XMVectorGetZ(e - s)) > 0 && m_miller->type == millerType::Cone)
			m_miller->isMovingVertically = true;
		else
			m_miller->isMovingVertically = false;

		float val = m_HeightMap->def_map_y;
		float left = -((val - XMVectorGetZ(e)) / (m_HeightMap->def_map_y / 2));
		float right = -(m_HeightMap->map_y / m_HeightMap->def_map_y);
		//float right = (m_HeightMap->low_y * m_HeightMap->criticalMaterialYTolerance);
		if (left < right)
		{
			int error = 10;
			ShowPopup();
			m_reader.pathIndex++;
			return;
		}
		m_miller->Reset(m_HeightMap);
	}
	s = XMVectorSet(StartMillerPosition.x, StartMillerPosition.y, StartMillerPosition.z, 1);
	XMVECTOR target = s + step;
	EndMillerPosition = XMFLOAT3(XMVectorGetX(target), XMVectorGetY(target), XMVectorGetZ(target));
	m_miller->SetupPosition(StartMillerPosition, EndMillerPosition);



	long long a = GetTickCount64();
	m_miller->UpdateActivePoints();

	StartMillerPosition = EndMillerPosition;

	long long b = GetTickCount64() - a;
	timer++;
	if (timer >= (max_speed - animation_speed))
	{
		timer = 0;
		m_reader.pathIndex++;
	}

	//while (++m_reader.pathIndex < m_reader.paths.size())
	//	UpdateCube(dt);
	//int d = 0;
}

void Scene::Update(float dt)
{
	UpdateInput();
	static MouseState prevState;
	MouseState currentState;
	if (!m_mouse->GetState(currentState))
		return;
	bool change = true;
	if (prevState.isButtonDown(0))
	{
		POINT d = currentState.getMousePositionChange();
		m_camera.Rotate(d.x / 300.f, d.y / 300.f);
	}
	else
		change = false;
	prevState = currentState;
	//if (change)
	UpdateCamera(m_camera.GetViewMatrix());
	//UpdateScene(dt);
	if (animationPending)
		UpdateCube(dt);

	//m_particles->Update(m_context, dt, m_camera.GetPosition());
}

XMFLOAT3 Scene::ComputeNormalVectorForTriangle(int elementNumber, int triangle)
{
	XMFLOAT3 p0 = vertices[elementNumber][triangles[elementNumber][triangle][0]].Pos;
	XMFLOAT3 p1 = vertices[elementNumber][triangles[elementNumber][triangle][1]].Pos;
	XMFLOAT3 p2 = vertices[elementNumber][triangles[elementNumber][triangle][2]].Pos;
	//
	XMVECTOR pos = XMLoadFloat3(
		&XMFLOAT3(
		p0.x,
		p0.y,
		p0.z));

	pos = XMVector3Transform(pos, m_SceneMtx[elementNumber]);

	p0.x = XMVectorGetX(pos);
	p0.y = XMVectorGetY(pos);
	p0.z = XMVectorGetZ(pos);

	pos = XMLoadFloat3(
		&XMFLOAT3(
		p1.x,
		p1.y,
		p1.z));

	pos = XMVector3Transform(pos, m_SceneMtx[elementNumber]);

	p1.x = XMVectorGetX(pos);
	p1.y = XMVectorGetY(pos);
	p1.z = XMVectorGetZ(pos);

	pos = XMLoadFloat3(
		&XMFLOAT3(
		p2.x,
		p2.y,
		p2.z));

	pos = XMVector3Transform(pos, m_SceneMtx[elementNumber]);

	p2.x = XMVectorGetX(pos);
	p2.y = XMVectorGetY(pos);
	p2.z = XMVectorGetZ(pos);
	//
	XMFLOAT3 p1_minus_p0 = XMFLOAT3(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z);
	XMFLOAT3 p2_minus_p0 = XMFLOAT3(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z);

	return Scalar(p1_minus_p0, p2_minus_p0);
}

XMFLOAT3 Scene::Normalize(XMFLOAT3 v)
{
	float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return XMFLOAT3(v.x / length, v.y / length, v.z / length);
}

float Scene::Dot(XMFLOAT3 v, XMFLOAT3 t)
{
	return v.x * t.x + v.y * t.y + v.z * t.z;
}

XMFLOAT3 Scene::Scalar(XMFLOAT3 v, XMFLOAT3 t)
{
	return XMFLOAT3(v.y * t.z - t.y * v.z,
		+v.z * t.x - t.z * v.x,
		v.x * t.y - t.x * v.y);
}

void Scene::ComputeShadowVolume()
{
	vector<vector<int>> bordersOfTheLightenedArea[6];
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < edges[i].size(); j++)
		{
			int v1 = edges[i][j][0];
			int v2 = edges[i][j][1];
			int t1 = edges[i][j][2];
			int t2 = edges[i][j][3];
			XMFLOAT3 normal1 = ComputeNormalVectorForTriangle(i, t1);
			XMFLOAT3 normal2 = ComputeNormalVectorForTriangle(i, t2);
			normal1 = Normalize(normal1);
			normal2 = Normalize(normal2);
			//
			XMFLOAT3 v1_Pos;
			XMFLOAT3 v2_Pos;

			XMVECTOR pos = XMLoadFloat3(
				&XMFLOAT3(
				vertexes[i][v1].x,
				vertexes[i][v1].y,
				vertexes[i][v1].z));

			pos = XMVector3Transform(pos, m_SceneMtx[i]);

			v1_Pos.x = XMVectorGetX(pos);
			v1_Pos.y = XMVectorGetY(pos);
			v1_Pos.z = XMVectorGetZ(pos);

			pos = XMLoadFloat3(
				&XMFLOAT3(
				vertexes[i][v2].x,
				vertexes[i][v2].y,
				vertexes[i][v2].z));

			pos = XMVector3Transform(pos, m_SceneMtx[i]);

			v2_Pos.x = XMVectorGetX(pos);
			v2_Pos.y = XMVectorGetY(pos);
			v2_Pos.z = XMVectorGetZ(pos);
			//
			XMFLOAT3 vectorBetweenLightAndTheTriangle = XMFLOAT3(lightPos.x - v1_Pos.x,
				lightPos.y - v1_Pos.y,
				lightPos.z - v1_Pos.z
				);

			float dotProductForTriangle1 = Dot(normal1, vectorBetweenLightAndTheTriangle);
			float dotProductForTriangle2 = Dot(normal2, vectorBetweenLightAndTheTriangle);
			if ((dotProductForTriangle1 > 0 && dotProductForTriangle2 <= 0)
				||
				(dotProductForTriangle1 <= 0 && dotProductForTriangle2 > 0))
			{
				//krawedz v1v2 znajduje siê na granicy oœwietlenia
				bordersOfTheLightenedArea[i].push_back(vector < int > {v1, v2});
			}
		}
	}

	vector<VertexPosNormal> verticesForShadowVolumes[6];
	vector<unsigned short> indicesForShadowVolumes[6];

	//tworzenie czworok¹tów
	for (int i = 0; i < 6; i++)
	{
		int counter = 0;
		for (int j = 0; j < bordersOfTheLightenedArea[i].size(); j++)
		{
			int v1 = bordersOfTheLightenedArea[i][j][0];
			int v2 = bordersOfTheLightenedArea[i][j][1];
			//
			XMFLOAT3 v1_Pos;
			XMFLOAT3 v2_Pos;

			XMVECTOR pos = XMLoadFloat3(
				&XMFLOAT3(
				vertexes[i][v1].x,
				vertexes[i][v1].y,
				vertexes[i][v1].z));

			pos = XMVector3Transform(pos, m_SceneMtx[i]);

			v1_Pos.x = XMVectorGetX(pos);
			v1_Pos.y = XMVectorGetY(pos);
			v1_Pos.z = XMVectorGetZ(pos);

			pos = XMLoadFloat3(
				&XMFLOAT3(
				vertexes[i][v2].x,
				vertexes[i][v2].y,
				vertexes[i][v2].z));

			pos = XMVector3Transform(pos, m_SceneMtx[i]);

			v2_Pos.x = XMVectorGetX(pos);
			v2_Pos.y = XMVectorGetY(pos);
			v2_Pos.z = XMVectorGetZ(pos);
			//
			XMFLOAT3 v1_prim = XMFLOAT3(v1_Pos.x - lightPos.x,
				v1_Pos.y - lightPos.y,
				v1_Pos.z - lightPos.z);


			XMFLOAT3 v2_prim = XMFLOAT3(v2_Pos.x - lightPos.x,
				v2_Pos.y - lightPos.y,
				v2_Pos.z - lightPos.z);

			//dodanie 4 wierzcho³ków tworz¹cych czworok¹t
			VertexPosNormal v;
			v.Pos = v1_Pos;
			verticesForShadowVolumes[i].push_back(v);
			v.Pos = v2_Pos;
			verticesForShadowVolumes[i].push_back(v);
			v.Pos = v1_prim;
			verticesForShadowVolumes[i].push_back(v);
			v.Pos = v2_prim;
			verticesForShadowVolumes[i].push_back(v);

			indicesForShadowVolumes[i].push_back(counter);
			indicesForShadowVolumes[i].push_back(counter + 1);
			indicesForShadowVolumes[i].push_back(counter + 2);
			indicesForShadowVolumes[i].push_back(counter + 1);
			indicesForShadowVolumes[i].push_back(counter + 3);
			indicesForShadowVolumes[i].push_back(counter + 2);

			indicesForShadowVolumes[i].push_back(counter + 2);
			indicesForShadowVolumes[i].push_back(counter + 1);
			indicesForShadowVolumes[i].push_back(counter);
			indicesForShadowVolumes[i].push_back(counter + 2);
			indicesForShadowVolumes[i].push_back(counter + 3);
			indicesForShadowVolumes[i].push_back(counter + 1);

			counter += 3;

		}

		m_vbSceneShadowVolume[i] = m_device.CreateVertexBuffer(&verticesForShadowVolumes[i][0], verticesForShadowVolumes[i].size());
		m_ibSceneShadowVolume[i] = m_device.CreateIndexBuffer(&indicesForShadowVolumes[i][0], indicesForShadowVolumes[i].size());
		SceneShadowVolumeIndicesCount[i] = indicesForShadowVolumes[i].size();
	}
}


void Scene::DrawShadowVolumes()
{
	for (int i = 0; i < 6; i++)
	{
		const XMMATRIX worldMtx = XMMatrixIdentity();// m_SceneMtx[i];
		m_cbWorld->Update(m_context, worldMtx);
		ID3D11Buffer* b = m_vbSceneShadowVolume[i].get();
		m_context->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
		m_context->IASetIndexBuffer(m_ibSceneShadowVolume[i].get(), DXGI_FORMAT_R16_UINT, 0);
		m_context->DrawIndexed(SceneShadowVolumeIndicesCount[i], 0, 0);
	}
}

void Scene::DrawScene(bool mirrored)
{
	//if (mirrored)
	//{
	//	DrawMirroredWorld();
	//}

	m_context->OMSetBlendState(m_bsAlpha.get(), 0, BS_MASK);

	SetLight0();

	DrawPlane();
	m_context->OMSetBlendState(0, 0, BS_MASK);

	DrawCoordinateSystem();
	DrawMiller();
	//SetLight0();
	DrawRoom();
	//DrawScene();
	DrawCircle();
	//DrawCyllinder();
	DrawCube();
}


void Scene::Render()
{
	if (m_context == nullptr)
		return;

	//m_lightShadowEffect->SetupShadow(m_context);
	//m_phongEffect->Begin(m_context);
	//DrawScene(false);
	//m_phongEffect->End();
	//m_lightShadowEffect->EndShadow();

	//ResetRenderTarget();
	//m_cbProj->Update(m_context, m_projMtx);
	//UpdateCamera(m_camera.GetViewMatrix());


	float clearColor[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
	m_context->ClearRenderTargetView(m_backBuffer.get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	SetShaders();
	SetConstantBuffers();

	//ComputeShadowVolume();
	///*DrawShadowVolumes();*/

	//DrawScene(false);

	//m_context->OMSetDepthStencilState(m_dssShadowsOne.get(), 0);
	//m_context->RSSetState(m_rsCullBack.get());
	//DrawShadowVolumes();
	//m_context->OMSetDepthStencilState(m_dssShadowsSecond.get(), 0);
	//m_context->RSSetState(m_rsCullFace.get());
	//DrawShadowVolumes();
	//m_context->OMSetDepthStencilState(nullptr, 0);
	//m_context->RSSetState(NULL);


	//m_lightShadowEffect->Begin(m_context);
	DrawScene(true);
	//m_lightShadowEffect->End();

	// Draw tweak bars
	TwDraw();


	//m_context->OMSetBlendState(m_bsAlpha.get(), nullptr, BS_MASK);
	//m_context->OMSetDepthStencilState(m_dssNoWrite.get(), 0);
	//m_particles->Render(m_context, XMFLOAT3());
	//m_context->OMSetDepthStencilState(nullptr, 0);
	//m_context->OMSetBlendState(nullptr, nullptr, BS_MASK);

	m_swapChain->Present(0, 0);
}
#include "pusn_scene.h"
#include "pusn_utils.h"
#include "pusn_vertices.h"
#include "pusn_window.h"
#include <fstream>
#include <iostream>

using namespace std;
using namespace pusn;

#define TEST_READY_PATHS

#define RESOURCES_PATH L"resources/"
const wstring Scene::ShaderFile = RESOURCES_PATH L"shaders/Puma.hlsl";

XMFLOAT4 Scene::lightPos = XMFLOAT4(-4, 4, 4, 1);
const unsigned int Scene::VB_STRIDE = sizeof(pusn::VertexPosNormal);
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
	InitializePlane();
	InitializeShadowEffects();

	PusnService service;
	service.Context = m_context;
	service.Device = m_device;
	service.cbLightColors = m_cbLightColors;
	service.cbLightPos = m_cbLightPos;
	service.cbSurfaceColor = m_cbSurfaceColor;



	m_coordinateSystem = new CoordinateSystem(service);
	m_miller = new Miller(service);

	m_HeightMap = new HeightMap(service);
	#ifndef TEST_READY_PATHS
		m_pathsDrawer = new PathsDrawer(service);
		m_HeightMap->PrepareSurface(m_modelsManager);
	#endif
	m_miller->Reset(m_HeightMap);
	PusnMenuService* menuService = new PusnMenuService(getMainWindow(), &m_reader, &m_writer, m_miller, m_HeightMap);
	m_menu = new Menu(menuService);
	#ifndef TEST_READY_PATHS
		//m_miller->GeneratePaths();
		m_miller->GeneratePathsSecondPart(m_HeightMap->orderedContourPoints);
		/*m_miller->GeneratePathsThirdPart(m_HeightMap->orderedPrecisePoints);
		m_miller->GeneratePathsThirdPart(m_HeightMap->orderedPrecisePointsBackHandle);
		m_miller->GeneratePathsThirdPart(m_HeightMap->orderedPrecisePointsFrontHandle);*/
		m_pathsDrawer->Feed(m_miller->vertices_container, m_miller->indices_container);
		m_menu->WritePaths(menuService);
	#endif

	m_particles.reset(new ParticleSystem(m_device, XMFLOAT3(-1.0f, -1.1f, 0.46f)));
	m_particles->SetViewMtxBuffer(m_cbView);
	m_particles->SetProjMtxBuffer(m_cbProj);

	SetShaders();
	SetConstantBuffers();

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

void Scene::DrawPaths()
{
	XMMATRIX worldMtx;
	m_pathsDrawer->GetModelMatrix(worldMtx);
	m_cbWorld->Update(m_context, worldMtx);
	m_pathsDrawer->Draw();
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

void Scene::DrawCube()
{
	//const XMMATRIX worldMtx = XMMatrixIdentity();
	//m_cbWorld->Update(m_context, XMMatrixTranslation(-1.0, 0, 0));
	m_HeightMap->Draw(m_menu-> IsMaterialActive());
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
	//m_particles->Render(m_context, XMFLOAT3());
	SetShaders();
	SetConstantBuffers();
	m_context->RSSetState(NULL);

	//Restore rendering state to it's original values
	UpdateCamera(viewMtx);
	m_context->OMSetDepthStencilState(NULL, 0);
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
	///////WSTAWKA/////
	//m_miller->FollowPaths();
	//return;
	/////////////////


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
		step = (e - s) / (m_menu->GetMenuMaxSpeed() - m_menu->GetMenuAnimationSpeed() );
		if (abs(XMVectorGetZ(e - s)) > 0 && m_miller->type == millerType::Cone)
			m_miller->isMovingVertically = true;
		else
			m_miller->isMovingVertically = false;

		float val = m_HeightMap->map_y;
		float left = -((val - XMVectorGetZ(e)) / (m_HeightMap->map_y / 2));
		float right = -(m_HeightMap->map_y / m_HeightMap->map_y);
		//float right = (m_HeightMap->low_y * m_HeightMap->criticalMaterialYTolerance);

		//PRZYWRÓc
		/*if (left < right)
		{
			int error = 10;
			ShowPopup();
			m_reader.pathIndex++;
			return;
		}*/
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
	if (timer >= (m_menu->GetMenuMaxSpeed() - m_menu->GetMenuAnimationSpeed() ) )
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
	if (m_menu->animationPending)
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
	#ifndef TEST_READY_PATHS
		DrawPaths();
	#endif
	//SetLight0();
	DrawRoom();
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
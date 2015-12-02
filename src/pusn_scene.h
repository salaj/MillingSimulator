#pragma once

#include "pusn_applicationBase.h"
#include "pusn_camera.h"
#include <xnamath.h>
#include <vector>
#include <mutex>
#include "pusn_vertices.h"
#include "pusn_lightShadowEffect.h"
#include "pusn_phongEffect.h"
#include "pusn_coordinateSystem.h"
#include "pusn_miller.h"
#include "pusn_service.h"
#include "pusn_heightMap.h"
#include "pusn_fileReader.h"
#include "pusn_particles.h"
#include "pusn_menu.h"
#include "pusn_pathDrawer.h"
//#include "bezierSurface.h"

using namespace std;
namespace pusn
{
	class Scene : public pusn::ApplicationBase
	{
	public:
		Scene(HINSTANCE hInstance);
		virtual ~Scene();
		static void* operator new(size_t size);
		static void operator delete(void* ptr);
	protected:
		virtual bool LoadContent();
		virtual void UnloadContent();

		virtual void Update(float dt);
		void UpdateCube(float dt);
		virtual void Render();
		void ShowPopup();
	private:

		//bool animationPending = true;

		static const unsigned int VB_STRIDE;
		static const unsigned int VB_OFFSET;
		static const unsigned int BS_MASK;

		static const float LAP_TIME;

		Camera m_camera;
		CoordinateSystem* m_coordinateSystem;
		Miller* m_miller;
		PathsDrawer* m_pathsDrawer;

		XMMATRIX m_projMtx;
		XMMATRIX m_SceneMtx[6];

		Menu* m_menu;

		HeightMap* m_HeightMap;

		FileReader m_reader;
		FileWriter m_writer;


		std::shared_ptr<ID3D11VertexShader> m_vertexShader;
		std::shared_ptr<ID3D11PixelShader> m_pixelShader;
		std::shared_ptr<ID3D11InputLayout> m_inputLayout;

		std::shared_ptr<ID3D11Buffer> m_vbRoom;
		std::shared_ptr<ID3D11Buffer> m_ibRoom;


		std::shared_ptr<ID3D11Buffer> m_vbPlane;
		std::shared_ptr<ID3D11Buffer> m_ibPlane;

		std::shared_ptr<ID3D11Buffer> m_vbScene[6];
		std::shared_ptr<ID3D11Buffer> m_ibScene[6];
		std::shared_ptr<ID3D11Buffer> m_vbSceneShadowVolume[6];
		std::shared_ptr<ID3D11Buffer> m_ibSceneShadowVolume[6];
		vector<VertexPosNormal> vertices[6];


		int SceneIndicesCount[6];
		int SceneShadowVolumeIndicesCount[6];
		vector<XMFLOAT3> vertexes[6];
		vector<unsigned short> indices[6];

		vector<vector<int>> triangles[6];
		vector<vector<int>> edges[6];

		std::shared_ptr<ID3D11DepthStencilState> m_dssWrite;
		std::shared_ptr<ID3D11DepthStencilState> m_dssTest;
		std::shared_ptr<ID3D11RasterizerState> m_rsCounterClockwise;
		std::shared_ptr<ID3D11RasterizerState> m_rsCullFace;
		std::shared_ptr<ID3D11RasterizerState> m_rsCullBack;
		std::shared_ptr<ID3D11BlendState> m_bsAlpha;
		std::shared_ptr<ID3D11BlendState> m_bsAdd;

		std::shared_ptr<ID3D11DepthStencilState> m_dssNoWrite;

		std::shared_ptr<ID3D11DepthStencilState> m_dssShadowsOne;
		std::shared_ptr<ID3D11DepthStencilState> m_dssShadowsSecond;
		
		std::shared_ptr<LightShadowEffect> m_lightShadowEffect;
		std::shared_ptr<pusn::PhongEffect> m_phongEffect;

		//dane okrêgu
		XMFLOAT2 circleCenter = XMFLOAT2(-0.9f - 1.3f / 2.0f, -1.0f + 1.3f / 2.0f* sqrt(3));
		float circleRadius = 0.5f;

		std::shared_ptr<ID3D11Buffer> m_vbCircle;
		std::shared_ptr<ID3D11Buffer> m_ibCircle;
		VertexPosNormal *circleVertices;


		std::shared_ptr<ID3D11Buffer> m_vbCyllinder;
		std::shared_ptr<ID3D11Buffer> m_ibCyllinder;
		VertexPosNormal *cyllinderVertices;

		void inverse_kinematics(XMFLOAT3 pos, XMFLOAT3 normal, float &a1, float &a2,
			float &a3, float &a4, float &a5);

		std::shared_ptr<CBMatrix> m_cbWorld;
		std::shared_ptr<CBMatrix> m_cbView;
		std::shared_ptr<CBMatrix> m_cbProj;
		std::shared_ptr<pusn::ConstantBuffer<XMFLOAT4>> m_cameraPosCB;
		std::shared_ptr<pusn::ConstantBuffer<XMFLOAT4>> m_lightPosCB;
		std::shared_ptr<pusn::ConstantBuffer<XMFLOAT4>> m_surfaceColorCB;
		std::shared_ptr<ID3D11Buffer> m_cbLightPos;
		std::shared_ptr<ID3D11Buffer> m_cbLightColors;
		std::shared_ptr<ID3D11Buffer> m_cbSurfaceColor;


		std::shared_ptr<ID3D11InputLayout> m_layout;


		std::shared_ptr<pusn::ParticleSystem> m_particles;

		static const std::wstring ShaderFile;
		static const std::wstring SceneFiles[6];

		void InitializeShaders();
		void InitializeConstantBuffers();
		void InitializeRenderStates();
		void InitializeCamera();
		void InitializeShadowEffects();

		void InitializeRoom();
		void InitializePlane();

		void UpdateCamera(const XMMATRIX& view);
		void UpdateInput();

		void SetShaders();
		void SetConstantBuffers();

		void SetLight0();

		void DrawScene(bool mirrored = false);
		void DrawRoom();
		void DrawCoordinateSystem();
		void DrawMiller();
		void DrawPaths();
		void DrawPlane(bool val = false);
		void DrawCube();
		void DrawMirroredWorld();

		void ComputeShadowVolume();
		XMFLOAT3 Normalize(XMFLOAT3 v); 
		float Dot(XMFLOAT3 v, XMFLOAT3 t);
		XMFLOAT3 Scalar(XMFLOAT3 v, XMFLOAT3 t);
		XMFLOAT3 ComputeNormalVectorForTriangle(int elementNumber, int triangle);
		static XMFLOAT4 lightPos;
	};

}

#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include <map>
#include "pusn_modelclass.h"
#include "pusn_constantBuffer.h"
#include "pusn_marchingSquares.h"
//#include "gk2_vertices.h"

#include "modelsManager.h"

using namespace std;

namespace pusn{
	class HeightMap
	{
	public:
		HeightMap(PusnService &service);
		HeightMap(const HeightMap&);
		~HeightMap();
		VertexPosNormal& GetHeightMapValue(int x, int y);
		bool& GetVerticeActiveValue(int x, int y);
		void Initialize();
		void PrepareSurface(ModelsManager* m_modelsManager);
		//void CheckNeighbours();
		void Draw(bool isMaterialActive);
		VertexPosNormal* cubeVertices;
		map<XMFLOAT2*, XMFLOAT2*> activePoints;
		int materialWidth = 150;
		int materialHeight = 150;

		bool* activeVertices;

		//float def_map_x = 150;
		//float def_map_y = 50;
		//float def_map_z = 150;

		float def_map_len = 2;
		float def_map_number = 150;

		//REAL DIMENSIONS in mm
		float map_x = 150;
		float map_y = 50;
		float map_z = 150;

		float low_y = -1.0;
		float criticalMaterialYTolerance = 0.7;

		vector<XMFLOAT2> orderedContourPoints;
		vector<XMFLOAT3> orderedPrecisePoints;
		vector<XMFLOAT3> orderedPrecisePointsBackHandle;
		vector<XMFLOAT3> orderedPrecisePointsFrontHandle;

	private:
		static const unsigned int VB_STRIDE;
		static const unsigned int VB_OFFSET;

		PusnService m_service;
		XMFLOAT3* m_heightMap;
		XMFLOAT3* m_lowMap;

		void InitializeMap(int n, int m);
		void fillIndices(int &index, int index1, int index2, int index3, int index4);
		void probeSurface(BSplineSurface* surface, XMFLOAT2& min, XMFLOAT2& max, XMFLOAT2& parMin, XMFLOAT2& parMax);
		void fixSurfaceHoles(int x_len, int y_len, int x_start, int y_start, float x_offset, float y_offset);
		void checkIntersections(IntersectionSurfaceBSpline* intersectionSurface, BSplineSurface* surface);
		void prepareEvolutePoints();
		void preparePrecisePoints(ModelsManager* m_modelsManager, BSplineSurface* surface, vector<XMFLOAT2>& collisions, vector<XMFLOAT3>& orderedPoints, int iter_u, int iter_v, float tolerance, bool forceStopOnCollisiondetected);

		vector<XMFLOAT2> collisionPoints;
		vector<XMFLOAT2> collisionPointsFrontHandle;
		vector<XMFLOAT2> collisionPointsBackHandle;

		VertexPosNormal* lowVertices;
		unsigned short* cubeIndicesMesh;
		unsigned short* cubeIndicesMaterial;
		shared_ptr<ID3D11Buffer> indicesPointerMesh;
		shared_ptr<ID3D11Buffer> indicesPointerMaterial;
		std::shared_ptr<DBVertexes> m_dbVertexes;
		int indicesMeshCount;
		int indicesMaterialCount;
	};
}
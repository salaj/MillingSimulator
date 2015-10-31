#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include <map>
#include "modelclass.h"
#include "gk2_constantBuffer.h"
//#include "gk2_vertices.h"


using namespace std;

namespace gk2{
	class HeightMap
	{
	public:
		HeightMap(Service &service);
		HeightMap(const HeightMap&);
		~HeightMap();
		VertexPosNormal& GetHeightMapValue(int x, int y);
		bool& GetVerticeActiveValue(int x, int y);
		void Initialize();
		//void CheckNeighbours();
		void Draw(bool isMaterialActive);
		VertexPosNormal* cubeVertices;
		map<XMFLOAT2*, XMFLOAT2*> activePoints;
		int materialWidth = 150;
		int materialHeight = 150;

		bool* activeVertices;

		float def_map_x = 150;
		float def_map_y = 50;
		float def_map_z = 150;


		float map_x = 150;
		float map_y = 50;
		float map_z = 150;

		float low_y = -1.0;
		float criticalMaterialYTolerance = 0.7;

	private:
		static const unsigned int VB_STRIDE;
		static const unsigned int VB_OFFSET;

		Service m_service;
		XMFLOAT3* m_heightMap;
		XMFLOAT3* m_lowMap;

		void InitializeMap(int n, int m);
		void fillIndices(int &index, int index1, int index2, int index3, int index4);

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
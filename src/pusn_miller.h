#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include "pusn_modelclass.h"
#include "pusn_heightMap.h"
#include "pusn_fileReader.h"

using namespace std;

namespace pusn{
	class Miller : public PusnModelClass{
	public:
		Miller(PusnService &service);
		Miller(std::shared_ptr<ID3D11DeviceContext>);
		Miller(const Miller&);
		~Miller();
		virtual void Initialize();
		virtual void preDraw();
		virtual void onDraw();
		virtual void afterDraw();
		bool CheckIfPointIsInside(XMVECTOR& pos);
		void UpdateActivePoints();
		XMFLOAT3 StartMillerPosition, EndMillerPosition;
		virtual void Reset(HeightMap* heightMap);
		void SetupPosition(XMFLOAT3 startMillerPosition, XMFLOAT3 endMillerPosition);
		float m_radius = 0.0f;
		millerType type;
		bool isMovingVertically = false;
		void updateNormals(XMFLOAT2*);
	private:
		float millerStep = 1.0f;

		virtual void setTriangleTopology();
		virtual void setLineTopology();
		void calculateOffset(XMVECTOR& pos, XMVECTOR& miller_center);

		//void updateActivePoints();
		void InitializeCyllinder();
		void prepareBresenham(float x0, float y0, float x1, float y1);
		bool calculateBresenham();
		void calculateNormal(XMFLOAT2& index, XMFLOAT2& index1, XMFLOAT2& index2, XMVECTOR& f, int& counter);
		//dane kuli
		int verticesCount;
		int indicesCount;
		//XMFLOAT3 circleCenter = XMFLOAT3(0.0f, -0.02f, 0.0f);

		//dane cylindra
		int verticesCyllinderCount;
		int indicesCyllinderCount;


		HeightMap* m_heightMap;
		int n, m;
		//vector<XMFLOAT2*> activePoints;
		int dx, sx;
		int dy, sy;
		int err, e2;
		int start_x, start_y;
		int end_x, end_y;
		float cell_width;
	};
}
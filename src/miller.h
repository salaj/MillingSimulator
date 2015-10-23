#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include "modelclass.h"
#include "heightMap.h"
#include "fileReader.h"

using namespace std;

namespace gk2{
	class Miller : public ModelClass{
	public:
		Miller(Service &service);
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
		bool ShouldContinueBresenham();
		virtual void Reset(HeightMap* heightMap);
		void SetupPosition(XMFLOAT3 startMillerPosition, XMFLOAT3 endMillerPosition);
		void CheckNeighbours();
		float m_radius = 0.0f;
		millerType type;
		bool isMovingVertically = false;
	private:
		virtual void setTriangleTopology();
		virtual void setLineTopology();
		void calculateOffset(XMVECTOR& pos, XMVECTOR& miller_center);

		//void updateActivePoints();
		void InitializeCyllinder();
		void prepareBresenham(float x0, float y0, float x1, float y1);
		bool calculateBresenham();
		void searchRecursively(int x, int y, map<XMFLOAT2*, XMFLOAT2*>*);
		void updateNormals(XMFLOAT2*);
		void calculateNormal(XMFLOAT2& index, XMFLOAT2& index1, XMFLOAT2& index2, XMVECTOR& f, int& counter);
		//dane kuli
		int verticesCount;
		int indicesCount;
		//XMFLOAT3 circleCenter = XMFLOAT3(0.0f, -0.02f, 0.0f);

		//dane cylindra
		int verticesCyllinderCount;
		int indicesCyllinderCount;


		HeightMap* m_heightMap;

		int iteration = 0;
		int n, m;
		//vector<XMFLOAT2*> activePoints;
		float dx, sx;
		float dy, sy;
		float err, e2;
		float start_x, start_y;
		float end_x, end_y;
		float cell_width;

		bool algorithmContinue = true;
	};
}
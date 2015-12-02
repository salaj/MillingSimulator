#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include "pusn_modelclass.h"
#include "pusn_heightMap.h"
#include "pusn_fileReader.h"
#include "pusn_fileWriter.h"
#include "pusn_path.h"

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
		void GeneratePaths();
		void GeneratePathsSecondPart(vector<XMFLOAT2>& orderedContourPoints);
		void GeneratePathsThirdPart(vector<XMFLOAT3>& orderedPrecisePoints);
		void UpdateActivePoints();
		XMFLOAT3 StartMillerPosition, EndMillerPosition;
		virtual void Reset(HeightMap* heightMap);
		void SetupPosition(XMFLOAT3 startMillerPosition, XMFLOAT3 endMillerPosition);
		float m_radius = 0.0f;
		millerType type;
		bool isMovingVertically = false;
		void updateNormals(XMFLOAT2*);
		void FollowPaths();

		vector<Path> paths;
		vector<VertexPosNormal> vertices_container;
		vector<unsigned short> indices_container;
	private:
		float millerStep = 1.0f;

		virtual void setTriangleTopology();
		virtual void setLineTopology();
		void calculateOffset(XMVECTOR& pos, XMVECTOR& miller_center);

		void generateSinglePaths(XMFLOAT3& lastPos, XMFLOAT2& previous, float currentZ);
		void generateUpperMovement(XMFLOAT2& first, XMFLOAT2& second, XMFLOAT3& lastPos);
		void moveMill(bool isMovingRight, int iterarion, int MilledDist, int columnStep, int radius, int miller_diameter, XMFLOAT2& last, XMFLOAT2& previous, XMFLOAT3& lastPos, float& currentZ, int left, int right);
		void upperLayerMilling(bool isMovingRight, int radius, int left, int right, int last_y, int MilledDist, float offsetZ);
		void returnToSafePosition();
		void startFromSafePosition(XMFLOAT3 targetPos);
		void startFromSafePositionFlatMiller(XMFLOAT3 targetPos);
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

		XMFLOAT3 safePosition;

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
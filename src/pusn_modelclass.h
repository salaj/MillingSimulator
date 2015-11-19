#pragma once


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <xnamath.h>
#include <memory>
#include <algorithm>
#include "pusn_vertices.h"
#include "pusn_deviceHelper.h"
#include "pusn_applicationBase.h"
#include "pusn_service.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////

//using namespace gk2;
using namespace std;

namespace pusn
{

	class PusnModelClass
	{

	public:

		static int m_counter;
		static int m_fakeCounter;

		PusnModelClass(PusnService &service);
		PusnModelClass(const PusnModelClass&);
		//ModelClass(Service& service);
		~PusnModelClass();

		//static void* operator new(size_t size);
		//static void operator delete(void* ptr);

		//called when object is initialized
		virtual void Initialize() = 0;
		void Draw();
		

		void Shutdown();
		void Render(std::shared_ptr<ID3D11DeviceContext>*);
		void SetModelMatrix(XMMATRIX&);
		void GetModelMatrix(XMMATRIX&);
		int GetIndexCount();

		///TRANSFORMATIONS

		void RotateX(double);
		void RotateY(double);
		void RotateZ(double);
		void Translate(XMFLOAT4&);
		void Scale(float);
		void ScaleDown();
		void ScaleUp();
		void ScaleToDefault();
		void ScaleBackToDefault();

		XMFLOAT4 GetPosition();
		XMFLOAT3 GetPosition3();
		XMFLOAT4 GetNormalizedPosition();
		XMFLOAT4 GetTranslatedPosition(PusnModelClass* cursor);
		void SetPosition(XMFLOAT4 position);
		void SetPosition(float x, float y, float z);
		void SetPosition(XMVECTOR pos);
		static float GetSquareDistanceBetweenModels(PusnModelClass*, PusnModelClass*);
		static XMFLOAT4 GetRelativeScaleVector(PusnModelClass*, PusnModelClass*);

		XMMATRIX m_modelMatrix;
		bool m_selected;
		int m_id;
		//ModelType m_Type = ModelType::Undecided;
		bool m_isGenuine;
	protected:
		PusnService m_service;
		std::shared_ptr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;
		std::shared_ptr<ID3D11Buffer> m_vertexBuffer_SquareFirst, m_indexBuffer_SquareFirst;
		std::shared_ptr<ID3D11Buffer> m_vertexBuffer_SquareSecond, m_indexBuffer_SquareSecond;
		std::shared_ptr<ID3D11Buffer> m_vertexBuffer_ParametrizationFirst, m_indexBuffer_ParametrizationFirst;
		std::shared_ptr<ID3D11Buffer> m_vertexBuffer_ParametrizationSecond, m_indexBuffer_ParametrizationSecond;
		int m_vertexCount, m_indexCount, m_parametrizationFirst_indexCount, m_parametrizationSecond_indexCount;
		//std::shared_ptr<ID3D11DeviceContext> m_context;
		//ShaderBase* m_shader_base;
		//InputClass* m_input;

		XMMATRIX createStereoscopicProjMatrixLeft();
		XMMATRIX createStereoscopicProjMatrixRight();

		static const unsigned int VB_STRIDE_WITH_NORMAL;
		static const unsigned int VB_STRIDE;
		static const unsigned int VB_OFFSET;

		virtual void setTriangleTopology() = 0;
		virtual void setLineTopology() = 0;
		virtual void preDraw() = 0;
		virtual void onDraw() = 0;
		virtual void afterDraw() = 0;

	private:
		static int counter;
		//const float rotation = (float)XM_PI * 0.05f;

		//gk2::Camera m_camera;

		const float diametryE = 1.f;

		XMMATRIX CreateXAxisRotationMatrix(float angle);
		XMMATRIX CreateYAxisRotationMatrix(float angle);
		XMMATRIX CreateZAxisRotationMatrix(float angle);
		XMMATRIX CreateTranslationMatrix(XMFLOAT4 offset);
		XMMATRIX CreateScaleMatrix(float s);
		
		XMFLOAT4 m_position;

		bool isScaledUp = false;
		float scaleFactor = 2.0f;
	};
}
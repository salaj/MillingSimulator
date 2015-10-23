#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include "modelclass.h"

namespace gk2
{
	class CoordinateSystem : public ModelClass
	{

	private:

	public:
		CoordinateSystem(Service &service);
		CoordinateSystem(std::shared_ptr<ID3D11DeviceContext>);
		CoordinateSystem(const CoordinateSystem&);
		~CoordinateSystem();

		//static void* operator new(size_t size);
		//static void operator delete(void* ptr);

		virtual void Initialize();
		virtual void preDraw();
		virtual void onDraw();
		virtual void afterDraw();


		void setStereoscopy(bool);

		CoordinateSystem& operator=(const CoordinateSystem& element);

	private:
		virtual void setTriangleTopology();
		virtual void setLineTopology();
		void initializeAxis(int axis);
		int verticesCount;
		int indicesCount;

		bool m_isStereoscopic = true;
	};
}
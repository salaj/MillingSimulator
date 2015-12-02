#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include "pusn_modelclass.h"

namespace pusn
{
	class PathsDrawer : public PusnModelClass
	{

	private:

	public:
		PathsDrawer(PusnService &service);
		PathsDrawer(std::shared_ptr<ID3D11DeviceContext>);
		PathsDrawer(const PathsDrawer&);
		~PathsDrawer();

		//static void* operator new(size_t size);
		//static void operator delete(void* ptr);

		virtual void Initialize();
		virtual void preDraw();
		virtual void onDraw();
		virtual void afterDraw();


		void setStereoscopy(bool);

		PathsDrawer& operator=(const PathsDrawer& element);
		void Feed(vector<VertexPosNormal>& vertices_container, vector<unsigned short>& indices_container);
	private:
		virtual void setTriangleTopology();
		virtual void setLineTopology();
		void initializeAxis(int axis);
		int verticesCount;
		int indicesCount;

		bool m_isStereoscopic = true;
	};
}
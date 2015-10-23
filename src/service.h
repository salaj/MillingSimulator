#pragma once

#include "gk2_applicationBase.h"
//#include "guiUpdater.h"

using namespace std;

namespace gk2{

	class Service
	{
	public:

		Service();
		Service(const Service&);
		virtual ~Service();
		shared_ptr<ID3D11DeviceContext> Context;
		shared_ptr<ID3D11Buffer> cbLightPos;
		shared_ptr<ID3D11Buffer> cbLightColors;
		shared_ptr<ID3D11Buffer> cbSurfaceColor;
		gk2::DeviceHelper Device;
	};
}

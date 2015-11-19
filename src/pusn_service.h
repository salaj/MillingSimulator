#pragma once

#include "pusn_applicationBase.h"

using namespace std;

namespace pusn{

	class PusnService
	{
	public:

		PusnService();
		PusnService(const PusnService&);
		virtual ~PusnService();
		shared_ptr<ID3D11DeviceContext> Context;
		shared_ptr<ID3D11Buffer> cbLightPos;
		shared_ptr<ID3D11Buffer> cbLightColors;
		shared_ptr<ID3D11Buffer> cbSurfaceColor;
		pusn::DeviceHelper Device;
	};
}

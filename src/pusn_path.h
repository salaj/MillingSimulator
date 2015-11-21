#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include <xnamath.h>

using namespace std;

namespace pusn{
	class Path {
	public:
		XMFLOAT3 StartPosition;
		XMFLOAT3 EndPosition;
	};
}
#pragma once
#include "pusn_service.h"

using namespace pusn;

namespace pusn{
	PusnService::PusnService()
	{
	}

	PusnService::PusnService(const PusnService& other)
	{
		this->Context = other.Context;
		this->Device = other.Device;
		this->cbLightPos = other.cbLightPos;
		this->cbLightColors = other.cbLightColors;
		this->cbSurfaceColor = other.cbSurfaceColor;
	}

	PusnService::~PusnService()
	{
	}
}

#pragma once

#include "pusn_effectBase.h"

namespace pusn
{
	class PhongEffect : public EffectBase
	{
	public:
		PhongEffect(pusn::DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
					std::shared_ptr<ID3D11DeviceContext> context = nullptr);

		void SetLightPosBuffer(const std::shared_ptr<pusn::ConstantBuffer<XMFLOAT4>>& lightPos);
		void SetSurfaceColorBuffer(const std::shared_ptr<pusn::ConstantBuffer<XMFLOAT4>>& surfaceColor);

	protected:
		virtual void SetVertexShaderData();
		virtual void SetPixelShaderData();

	private:
		static const std::wstring ShaderFile;

		std::shared_ptr<pusn::ConstantBuffer<XMFLOAT4>> m_lightPosCB;
		std::shared_ptr<pusn::ConstantBuffer<XMFLOAT4>> m_surfaceColorCB;
	};
}
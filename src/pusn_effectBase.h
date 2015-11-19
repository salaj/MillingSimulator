#pragma once

#include <d3d11.h>
#include "pusn_deviceHelper.h"
#include "pusn_constantBuffer.h"
#include <memory>
#include <string>

namespace pusn
{
	class EffectBase
	{
	public:
		void SetWorldMtxBuffer(const std::shared_ptr<pusn::CBMatrix>& world);
		void SetViewMtxBuffer(const std::shared_ptr<pusn::CBMatrix>& view);
		void SetProjMtxBuffer(const std::shared_ptr<pusn::CBMatrix>& proj);

		void Begin(std::shared_ptr<ID3D11DeviceContext> context = nullptr);
		void End();

	protected:
		EffectBase(std::shared_ptr<ID3D11DeviceContext> context = nullptr);

		virtual void SetVertexShaderData() = 0;
		virtual void SetPixelShaderData() = 0;

		std::shared_ptr<pusn::CBMatrix> m_worldCB;
		std::shared_ptr<pusn::CBMatrix> m_viewCB;
		std::shared_ptr<pusn::CBMatrix> m_projCB;
		std::shared_ptr<ID3D11DeviceContext> m_context;

		void Initialize(pusn::DeviceHelper& device, std::shared_ptr<ID3D11InputLayout>& layout,
						const std::wstring& shaderFile);

	private:
		std::shared_ptr<ID3D11VertexShader> m_vs;
		std::shared_ptr<ID3D11PixelShader> m_ps;
		std::shared_ptr<ID3D11InputLayout> m_layout;
	};
}

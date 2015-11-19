#pragma once

#include <d3d11.h>
#include <D3DX11.h>
#include <dinput.h>
#include "pusn_input.h"
#include "pusn_deviceHelper.h"
//#include "pusn_exceptions.h"
#include <AntTweakBar.h>

#include "modelsManager.h"

namespace pusn
{
	class Window;

	class ApplicationBase
	{
	public:
		ApplicationBase(HINSTANCE hInstance);
		virtual ~ApplicationBase();

		int Run(pusn::Window* window, int cmdShow);

		inline HINSTANCE getHandle() const { return m_hInstance; }
		inline pusn::Window* getMainWindow() const { return m_mainWindow; }
		void SetModelsManager(gk2::ModelsManager* modelsManager);

	protected:
		bool Initialize();
		int MainLoop();
		void Shutdown();

		virtual bool LoadContent();
		virtual void UnloadContent();
		virtual void Update(float dt) = 0;
		virtual void Render() = 0;

		void ResetRenderTarget();

		D3D_DRIVER_TYPE m_driverType;
		D3D_FEATURE_LEVEL m_featureLevel;

		pusn::DeviceHelper m_device;
		std::shared_ptr<ID3D11DeviceContext> m_context;
		std::shared_ptr<IDXGISwapChain> m_swapChain;
		std::shared_ptr<ID3D11Texture2D> m_backBufferTexture;
		std::shared_ptr<ID3D11RenderTargetView> m_backBuffer;
		std::shared_ptr<ID3D11Texture2D> m_depthStencilTexture;
		std::shared_ptr<ID3D11DepthStencilView> m_depthStencilView;
		pusn::InputHelper m_input;
		std::shared_ptr<Keyboard> m_keyboard;
		std::shared_ptr<Mouse> m_mouse;
		gk2::ModelsManager* m_modelsManager;
	private:
		HINSTANCE m_hInstance;
		pusn::Window* m_mainWindow;

		void FillSwapChainDesc(DXGI_SWAP_CHAIN_DESC& desc, int width, int height);
		void CreateDeviceAndSwapChain(SIZE windowSize);
		void CreateBackBuffers(SIZE windowSize);
		void InitializeDirectInput();
		void SetViewPort(SIZE windowSize);


	};
}

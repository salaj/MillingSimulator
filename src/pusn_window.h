#pragma once

#include <string>
#include <Windows.h>
#include <AntTweakBar.h>

namespace pusn
{
	class Window
	{
	public:
		Window(HINSTANCE hInstance, int width, int height);
		Window(HINSTANCE hInstance, int width, int height, const std::wstring& title);
		virtual ~Window();

		virtual void Show(int cmdShow);
		SIZE getClientSize() const;
		inline HWND getHandle() const { return m_hWnd; }

	protected:
		virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		static bool IsWindowClassRegistered(HINSTANCE hInstance);
		static void RegisterWindowClass(HINSTANCE hInstance);
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static std::wstring m_windowClassName;
		static int m_defaultWindowWidth;
		static int m_defaultWindowHeight;

		Window(const pusn::Window& other) { /* Do not use!*/ }

		void CreateWindowHandle(int width, int height, const std::wstring& title);

		HWND m_hWnd;
		HINSTANCE m_hInstance;
	};
}

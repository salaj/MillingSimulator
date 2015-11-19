#pragma once

#include <Windows.h>
#include <string>
#include <DxErr.h>
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#define STRINGIFY(x) #x
#define TOWSTRING(x) WIDEN(STRINGIFY(x))
#define __AT__ __WFILE__ L":" TOWSTRING(__LINE__)

namespace pusn
{
	class Exception
	{
	public:
		Exception(const wchar_t* location) { m_location = location; }
		virtual std::wstring getMessage() const = 0;
		virtual int getExitCode() const = 0;
		const wchar_t* getErrorLocation() const { return m_location; }
	private:
		const wchar_t* m_location;
	};

	class WinAPIException : public pusn::Exception
	{
	public:
		WinAPIException(const wchar_t* location, DWORD errorCode = GetLastError());
		virtual int getExitCode() const { return getErrorCode(); }
		inline DWORD getErrorCode() const { return m_code; }
		virtual std::wstring getMessage() const;

	private:
		DWORD m_code;
	};

	class Dx11Exception : public pusn::Exception
	{
	public:
		Dx11Exception(const wchar_t* location, HRESULT result);
		virtual int getExitCode() const { return getResultCode(); }
		inline HRESULT getResultCode() const { return m_result; }
		virtual std::wstring getMessage() const;

	private:
		HRESULT m_result;
	};
}

#define PUSN_THROW_WINAPI throw pusn::WinAPIException(__AT__)
#define PUSN_THROW_DX11(hr) throw pusn::Dx11Exception(__AT__, hr)


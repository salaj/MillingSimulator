#pragma once

#include <d3d11.h>
#include <memory>
#include <xnamath.h>
#include "pusn_deviceHelper.h"
#include "pusn_vertices.h"

namespace pusn
{
	class ConstantBufferBase
	{
	public:
		const std::shared_ptr<ID3D11Buffer>& getBufferObject() const { return m_bufferObject; }

	protected:
		ConstantBufferBase(pusn::DeviceHelper& device, unsigned int dataSize, unsigned int dataCount);

		void Update(const std::shared_ptr<ID3D11DeviceContext>& context, const void* dataPtr, unsigned int dataCount);
		
		void Map(const std::shared_ptr<ID3D11DeviceContext>& context);
		void* get();
		void Unmap(const std::shared_ptr<ID3D11DeviceContext>& context);

		int m_mapped;
		unsigned int m_dataSize;
		unsigned int m_dataCount;
		std::shared_ptr<ID3D11Buffer> m_bufferObject;
		D3D11_MAPPED_SUBRESOURCE m_resource;

	private:
		ConstantBufferBase(const ConstantBufferBase& right) { }
		ConstantBufferBase& operator=(const ConstantBufferBase& right) { return *this; }
	};

	template<typename T, unsigned int N = 1>
	class ConstantBuffer : public ConstantBufferBase
	{
	public:
		ConstantBuffer(pusn::DeviceHelper& device)
			: ConstantBufferBase(device, sizeof(T), N)
		{ }

		void Update(const std::shared_ptr<ID3D11DeviceContext>& context, const T& data)
		{
			return ConstantBufferBase::Update(context, reinterpret_cast<const void*>(&data), 1);
		}

		void Update(const std::shared_ptr<ID3D11DeviceContext>& context, const T* data)
		{
			return ConstantBufferBase::Update(context, reinterpret_cast<const void*>(data), N);
		}

		void Map(const std::shared_ptr<ID3D11DeviceContext>& context) { ConstantBufferBase::Map(context); }
		T* get() { return reinterpret_cast<T*>(ConstantBufferBase::get()); }
		void Unmap(const std::shared_ptr<ID3D11DeviceContext>& context) { ConstantBufferBase::Unmap(context); }

	private:
		ConstantBuffer(const ConstantBuffer<T, N>& right) { }
		ConstantBuffer<T, N>& operator=(const ConstantBuffer<T, N>& right) { return *this; }
	};
	
	typedef pusn::ConstantBuffer<XMMATRIX> CBMatrix;

	template<typename T>
	class DynamicBuffer : public ConstantBufferBase
	{
		public:
			DynamicBuffer(pusn::DeviceHelper& device, int dataCount) : m_dataCount(dataCount),
				ConstantBufferBase(device, sizeof(T), dataCount)
			{ }
			void Update(const std::shared_ptr<ID3D11DeviceContext>& context, const T* data)
			{
				return ConstantBufferBase::Update(context, reinterpret_cast<const void*>(data), m_dataCount);
			}
			void Map(const std::shared_ptr<ID3D11DeviceContext>& context) { ConstantBufferBase::Map(context); }
			T* get() { return reinterpret_cast<T*>(ConstantBufferBase::get()); }
			void Unmap(const std::shared_ptr<ID3D11DeviceContext>& context) { ConstantBufferBase::Unmap(context); }
	private:
		int m_dataCount;
		DynamicBuffer(const DynamicBuffer<T>& right) { }
		DynamicBuffer<T>& operator=(const DynamicBuffer<T>& right) { return *this; }
	};

	typedef pusn::DynamicBuffer<VertexPosNormal> DBVertexes;
}


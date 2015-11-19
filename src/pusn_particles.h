#pragma once

#include <d3d11.h>
#include <xnamath.h>
#include <list>
#include <memory>
#include "pusn_deviceHelper.h"
#include "pusn_constantBuffer.h"

namespace pusn
{
	struct ParticleVertex
	{
		XMFLOAT3 Pos;
		float Age;
		float Angle;
		float Size;
		static const unsigned int LayoutElements = 4;
		static const D3D11_INPUT_ELEMENT_DESC Layout[LayoutElements];

		ParticleVertex() : Pos(0.0f, 0.0f, 0.0f), Age(0.0f), Angle(0.0f), Size(0.0f) { }
	};
	
	struct ParticleVelocities
	{
		XMFLOAT3 Velocity;
		float AngleVelocity;
		XMFLOAT3 StartVelocity;
		XMFLOAT3 StartPos;
		ParticleVelocities() : Velocity(0.0f, 0.0f, 0.0f), AngleVelocity(0.0f) { }
	};

	struct Particle
	{
		ParticleVertex Vertex;
		ParticleVelocities Velocities;
		float time;
	};

	class ParticleComparer
	{
	public:
		ParticleComparer(XMFLOAT4 camDir, XMFLOAT4 camPos) : m_camDir(camDir), m_camPos(camPos) { }

		bool operator()(const pusn::ParticleVertex& p1, const pusn::ParticleVertex& p2);

	private:
		XMFLOAT4 m_camDir, m_camPos;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem(pusn::DeviceHelper& device, XMFLOAT3 emitterPos);

		void SetViewMtxBuffer(const std::shared_ptr<pusn::CBMatrix>& view);
		void SetProjMtxBuffer(const std::shared_ptr<pusn::CBMatrix>& proj);

		void Update(std::shared_ptr<ID3D11DeviceContext>& context, float dt, XMFLOAT4 cameraPos);
		void Render(std::shared_ptr<ID3D11DeviceContext>& context, XMFLOAT3 emitterPos);
		XMFLOAT3 m_emitterPos;
		static XMVECTOR m_perpendicularToPlane;
		static XMFLOAT3 m_startPosition;
	private:
		static const XMFLOAT3 EMITTER_DIR;	//mean direction of particles' velocity
		static const float TIME_TO_LIVE;	//time of particle's life in seconds
		static const float EMISSION_RATE;	//number of particles to be born per second
		static const float MAX_ANGLE;		//maximal angle declination from mean direction
		static const float MIN_VELOCITY;	//minimal value of particle's velocity
		static const float MAX_VELOCITY;	//maximal value of particle's velocity
		static const float PARTICLE_SIZE;	//initial size of a particle
		static const float PARTICLE_SCALE;	//size += size*scale*dtime
		static const float MIN_ANGLE_VEL;	//minimal rotation speed
		static const float MAX_ANGLE_VEL;	//maximal rotation speed
		static const int MAX_PARTICLES;		//maximal number of particles in the system
		
		static const unsigned int OFFSET;
		static const unsigned int STRIDE;

		
		float m_particlesToCreate;
		unsigned int m_particlesCount;
		
		std::list<Particle> m_particles;

		std::shared_ptr<ID3D11Buffer> m_vertices;
		
		std::shared_ptr<pusn::CBMatrix> m_viewCB;
		std::shared_ptr<pusn::CBMatrix> m_projCB;
		
		std::shared_ptr<ID3D11SamplerState> m_samplerState;
		std::shared_ptr<ID3D11ShaderResourceView> m_cloudTexture;
		std::shared_ptr<ID3D11ShaderResourceView> m_opacityTexture;

		std::shared_ptr<ID3D11VertexShader> m_vs;
		std::shared_ptr<ID3D11GeometryShader> m_gs;
		std::shared_ptr<ID3D11PixelShader> m_ps;
		std::shared_ptr<ID3D11InputLayout> m_layout;

		static XMFLOAT3 RandomVelocity();
		void AddNewParticle();
		void UpdateParticle(pusn::Particle& p, float dt);
		void UpdateVertexBuffer(std::shared_ptr<ID3D11DeviceContext>& context, XMFLOAT4 cameraPos);
	};
}

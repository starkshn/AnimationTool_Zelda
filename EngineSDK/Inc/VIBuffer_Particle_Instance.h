#pragma once

#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Particle_Instance abstract : public CVIBuffer
{
public:
	typedef struct
	{
		_uint			iNumInstances = { };
		_float3			vCenter;
		_float3			vRange;
		_float3			vPivot;
		_float2			vSpeed;		
		_float2			vLifeTime;
		_float2			vSize;
		_bool			isLoop;
	}PARTICLE_DESC;

protected:
	CVIBuffer_Particle_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Particle_Instance(const CVIBuffer_Particle_Instance& Prototype);
	virtual ~CVIBuffer_Particle_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render();
	virtual HRESULT Bind_BufferDesc();	

public:
	void Drop(_float fTimeDelta);
	void Spread(_float fTimeDelta);

protected:
	ID3D11Buffer*					m_pVBInstance = { nullptr };
	D3D11_BUFFER_DESC				m_InstanceBufferDesc = {};
	D3D11_SUBRESOURCE_DATA			m_InstanceInitialDesc = {};
	_uint							m_iNumInstances = {};
	_uint							m_iNumIndexPerInstance = {};
	_uint							m_iInstanceStride = {};
	VTXINSTANCE*					m_pInstanceVertices = {};
	_float3							m_vPivot = {};
	_bool							m_isLoop = {};
	_float*							m_pSpeeds = {};


public:
	
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END


#pragma once

#include "VIBuffer_Particle_Instance.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Particle_Point final : public CVIBuffer_Particle_Instance
{
private:
	CVIBuffer_Particle_Point(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Particle_Point(const CVIBuffer_Particle_Point& Prototype);
	virtual ~CVIBuffer_Particle_Point() = default;

public:
	virtual HRESULT Initialize_Prototype(const PARTICLE_DESC& ParticleDesc);
	virtual HRESULT Initialize(void* pArg) override;

public:
	static CVIBuffer_Particle_Point* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const PARTICLE_DESC& ParticleDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};


END
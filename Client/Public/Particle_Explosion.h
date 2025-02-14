#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Particle_Point;
class CTexture;
class CShader;
END


BEGIN(Client)

class CParticle_Explosion final : public CGameObject
{
private:
	CParticle_Explosion(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticle_Explosion(const CParticle_Explosion& Prototype);
	virtual ~CParticle_Explosion() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:	
	CShader*					m_pShaderCom = { nullptr };	
	CTexture*					m_pTextureCom = { nullptr };
	CVIBuffer_Particle_Point*	m_pVIBufferCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CParticle_Explosion* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;


};

END
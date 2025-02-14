#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)

class CCollider;
class CShader;
class CVIBuffer_Square;
class CTexture;

END

BEGIN(Client)

class CAT_Mouse final : public CGameObject
{
private:
	CAT_Mouse(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAT_Mouse(const CAT_Mouse& Prototype);
	virtual ~CAT_Mouse() override = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;

private:
	HRESULT					Ready_Components();
	HRESULT					Bind_ShaderResources();

private:
	CShader*				m_pShaderCom = nullptr;
	CVIBuffer_Square*		m_pVIBufferCom = nullptr;
	CTexture*				m_pTextureCom = nullptr;
	CCollider*				m_pColliderCom = nullptr;


public:
	static CAT_Mouse*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

END
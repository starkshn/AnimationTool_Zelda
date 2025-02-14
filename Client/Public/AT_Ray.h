#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)

class CShader;
class CVIBuffer_Line;
class CTexture;

END

BEGIN(Client)

class CAT_Ray final : public CGameObject
{
private:
	CAT_Ray(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAT_Ray(const CAT_Ray& Prototype);
	virtual ~CAT_Ray() override = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;

public:
	void					Set_Color(_float3 vColor)
	{
		m_vColor = vColor;
	}

	_bool					Get_Use() { return m_bUse; }
	void					Set_Use(_bool bUse) { m_bUse = bUse; }

private:
	HRESULT					Ready_Components();
	HRESULT					Bind_ShaderResources();

private:
	CShader*				m_pShaderCom = nullptr;
	CVIBuffer_Line*			m_pVIBufferCom = nullptr;
	CTexture*				m_pTextureCom = nullptr;

private:
	_float3					m_vColor = { 1.f, 0.f, 0.f };
	_bool					m_bUse = false;

public:
	static CAT_Ray*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

END
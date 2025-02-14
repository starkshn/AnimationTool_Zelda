#pragma once

#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CAlphaObject abstract : public CGameObject
{

protected:
	CAlphaObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAlphaObject(const CAlphaObject& Prototype);
	virtual ~CAlphaObject() = default;

public:
	_float Get_ViewZ() const {
		return m_fViewZ;
	}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;	

protected:
	_float						m_fViewZ = { };

protected:
	void Compute_ViewZ();

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;

};

END
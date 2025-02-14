#pragma once

#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CPartObj abstract : public CGameObject
{
public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		const _float4x4* pParentMatrix;
	}PARTOBJ_DESC;
protected:
	CPartObj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPartObj(const CPartObj& Prototype);
	virtual ~CPartObj() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	
protected:
	const _float4x4*		m_pParentMatrix = {};
	_float4x4				m_WorldMatrix = {};		

public:
	virtual void Free() override;
};

END
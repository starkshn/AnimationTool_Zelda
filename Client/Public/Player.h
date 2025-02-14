#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CCollider;
class CNavigation;
END


BEGIN(Client)

class CPlayer final : public CGameObject
{
public:
	enum PARTOBJ { PART_BODY, PART_WEAPON, PART_EFFECT, PART_END };
	enum STATE {
		STATE_RESET = 0x00000000,
		STATE_IDLE = 0x00000001, 
		STATE_WALK = 0x00000002, 
		STATE_ATTACK = 0x00000004,
	};
private:
	CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayer(const CPlayer& Prototype);
	virtual ~CPlayer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_uint				m_iState = {};

	CCollider*			m_pColliderCom = { nullptr };
	CNavigation*		m_pNavigationCom = { nullptr };

private:
	HRESULT Ready_Components();	
	HRESULT Ready_PartObjects();

public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
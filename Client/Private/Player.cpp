#include "stdafx.h"
#include "..\Public\Player.h"

#include "GameInstance.h"
#include "Body_Player.h"
#include "Navigation.h"
#include "Weapon.h"


CPlayer::CPlayer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{

}

CPlayer::CPlayer(const CPlayer & Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CPlayer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer::Initialize(void * pArg)
{
	GAMEOBJECT_DESC		Desc{};

	
	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = XMConvertToRadians(90.f);

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_PartObjects()))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(5.f, 0.f, 5.f, 1.f));	

	return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{

	__super::Priority_Update(fTimeDelta);
}

void CPlayer::Update(_float fTimeDelta)
{
	/*if (GetKeyState(VK_LBUTTON) & 0x8000)
	{
		_float4			vPickPos = {};
		if (true == m_pGameInstance->isPicked(&vPickPos))
		{
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&vPickPos));
		}
	}*/

	if (GetKeyState(VK_DOWN) & 0x8000)
		m_pTransformCom->Go_Backward(fTimeDelta);

	if (GetKeyState(VK_LEFT) & 0x8000)
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * -1.f);

	if (GetKeyState(VK_RIGHT) & 0x8000)
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);

	if (GetKeyState(VK_UP) & 0x8000)
	{
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);

		m_iState = STATE_WALK;
	}
	else
	{
		if(m_iState & STATE_WALK)
			m_iState ^= STATE_WALK;

		m_iState |= STATE_IDLE;
	}

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, 
	//	m_pNavigationCom->SetUp_OnNavigation(m_pTransformCom->Get_State(CTransform::STATE_POSITION)));

	__super::Update(fTimeDelta);



	m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix());
}

void CPlayer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	_vector		vWorldPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vWorldPos, m_pGameInstance->Compute_Height(vWorldPos)));

	m_pGameInstance->Add_RenderObject(CRenderer::RG_NONPICKED, this);

#ifdef _DEBUG
	m_pGameInstance->Add_DebugComponent(m_pColliderCom);
#endif
}

HRESULT CPlayer::Render()
{


	return S_OK;
}

HRESULT CPlayer::Ready_Components()
{
	/* Com_Collider */
	CBounding_AABB::BOUND_AABB_DESC		ColliderDesc{};
	ColliderDesc.vExtents = _float3(0.3f, 0.7f, 0.3f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vExtents.y, 0.f);

	if (FAILED(Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		return E_FAIL;

	/* Com_Navigation */

	CNavigation::NAVIGATION_DESC		NaviDesc{};

	NaviDesc.iStartIndex = 0;

	if (FAILED(Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom), &NaviDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayer::Ready_PartObjects()
{
	CBody_Player::BODY_PLAYER_DESC			BodyDesc{};

	BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	BodyDesc.pState = &m_iState;
	Add_Child(static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Body_Player"), &BodyDesc)));

	if (nullptr == m_vecChildren.back())
		return E_FAIL;


	CWeapon::WEAPON_DESC			WeaponDesc{};

	WeaponDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	WeaponDesc.pState = &m_iState;
	WeaponDesc.pSocketMatrix = static_cast<CBody_Player*>(m_vecChildren.back())->Get_BoneMatrix("SWORD");
	Add_Child(static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Weapon"), &WeaponDesc)));
	
	if (nullptr == m_vecChildren.back())
		return E_FAIL;

	return S_OK;
}

CPlayer * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayer*	pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayer::Clone(void * pArg)
{
	CPlayer*	pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPlayer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();

	Safe_Release(m_pNavigationCom);
	Safe_Release(m_pColliderCom);
}

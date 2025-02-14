#include "stdafx.h"
#include "AT_Square.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Transform.h"

CAT_Square::CAT_Square(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:
	CGameObject(pDevice, pContext)
{
}

CAT_Square::CAT_Square(const CAT_Square& Prototype)
	:
	CGameObject(Prototype)
{
}

HRESULT CAT_Square::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAT_Square::Initialize(void* pArg)
{
	if (__super::Initialize(pArg))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CAT_Square::Priority_Update(_float fTimeDelta)
{
}

void CAT_Square::Update(_float fTimeDelta)
{
	m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix());
}

void CAT_Square::Late_Update(_float fTimeDelta)
{
	// m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);


	m_pGameInstance->Add_DebugComponent(m_pColliderCom);
}

HRESULT CAT_Square::Render()
{

	return S_OK;
}

HRESULT CAT_Square::Ready_Components()
{
	/* Com_Collider */
	//CBounding_Sphere::BOUND_SPHERE_DESC		ColliderDesc{};
	//ColliderDesc.fRadius = 0.03f;
	//ColliderDesc.vCenter = _float3(0.f, ColliderDesc.fRadius, 0.f);


	CBounding_AABB::BOUND_AABB_DESC		ColliderDesc{};
	ColliderDesc.vExtents = _float3(0.025f, 0.025f, 0.025f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vExtents.y, 0.f);

	/*ColliderDesc.vRotation = _float3(XMConvertToRadians(0.f), XMConvertToRadians(45.f), XMConvertToRadians(0.f));*/

	if (FAILED(Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Collider_AABB"), TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		return E_FAIL;

	return S_OK;

}

HRESULT CAT_Square::Bind_ShaderResources()
{
	return S_OK;
}

CAT_Square* CAT_Square::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAT_Square* pInstance = new CAT_Square(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAT_Square");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAT_Square::Clone(void* pArg)
{
	CAT_Square* pInstance = new CAT_Square(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAT_Square");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAT_Square::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
}

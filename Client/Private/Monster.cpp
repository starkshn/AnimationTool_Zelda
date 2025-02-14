#include "stdafx.h"
#include "..\Public\Monster.h"

#include "GameInstance.h"
#include "Player.h"


CMonster::CMonster(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject { pDevice, pContext }
{
}

CMonster::CMonster(const CMonster & Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CMonster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster::Initialize(void * pArg)
{
	if (FAILED(__super::Initialize(nullptr)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pModelCom->SetUp_Animation(rand() % 20, true);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_pGameInstance->Compute_Random(0.f, 10.f), 0.f,m_pGameInstance->Compute_Random(0.f, 10.f),  1.f));	

	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(m_pGameInstance->Compute_Random(0.f, 360.f)));

	return S_OK;
}

void CMonster::Priority_Update(_float fTimeDelta)
{
	int a = 10;
}

void CMonster::Update(_float fTimeDelta)
{	
	if (m_pModelCom->Play_Animation(fTimeDelta))
	{
		int a = 10;
	}
	for (auto& pColliderCom : m_pColliderCom)
		pColliderCom->Update(m_pTransformCom->Get_WorldMatrix());

}

void CMonster::Late_Update(_float fTimeDelta)
{
	/*CCollider*		pTargetCollider = static_cast<CCollider*>(m_pGameInstance->Get_Component(LEVEL_GAMEPLAY, TEXT("Layer_Player"), CPlayer::PART_WEAPON, TEXT("Com_Collider")));

	_vector		vWorldPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vWorldPos, m_pGameInstance->Compute_Height(vWorldPos)));

	m_pColliderCom[COLL_OBB]->Intersect(pTargetCollider);	

	if(true == m_pGameInstance->isIn_Frustum_InWorldSpace(vWorldPos, 2.f))
	{
		m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);

#ifdef _DEBUG
		for (auto& pColliderCom : m_pColliderCom)
			m_pGameInstance->Add_DebugComponent(pColliderCom);
#endif

	}*/


}

HRESULT CMonster::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}


	

	return S_OK;
}

HRESULT CMonster::Ready_Components()
{
	/* Com_Collider_Sphere */
	CBounding_Sphere::BOUND_SPHERE_DESC		ColliderSphereDesc{};


	ColliderSphereDesc.fRadius = 0.7f;
	ColliderSphereDesc.vCenter = _float3(0.f, ColliderSphereDesc.fRadius, 0.f);
	/*ColliderDesc.vRotation = _float3(XMConvertToRadians(0.f), XMConvertToRadians(45.f), XMConvertToRadians(0.f));*/

	if (FAILED(Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider_Sphere"), reinterpret_cast<CComponent**>(&m_pColliderCom[COLL_SPHERE]), &ColliderSphereDesc)))
		return E_FAIL;

	/* Com_Collider_OBB */
	CBounding_OBB::BOUND_OBB_DESC		ColliderOBBDesc{};

	ColliderOBBDesc.vExtents = _float3(0.5f, 0.5f, 0.5f);
	ColliderOBBDesc.vCenter = _float3(0.f, ColliderOBBDesc.vExtents.y, 0.f);
	ColliderOBBDesc.vRotation = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));

	if (FAILED(Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider_OBB"), reinterpret_cast<CComponent**>(&m_pColliderCom[COLL_OBB]), &ColliderOBBDesc)))
		return E_FAIL;

	/* Com_Collider_AABB */
	CBounding_AABB::BOUND_AABB_DESC		ColliderAABBDesc{};


	ColliderAABBDesc.vExtents = _float3(0.3f, 0.7f, 0.3f);
	ColliderAABBDesc.vCenter = _float3(0.f, ColliderAABBDesc.vExtents.y, 0.f);

	if (FAILED(Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider_AABB"), reinterpret_cast<CComponent**>(&m_pColliderCom[COLL_AABB]), &ColliderAABBDesc)))
		return E_FAIL;
	/* Com_Shader */
	if (FAILED(Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Fiona"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMonster::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CMonster * CMonster::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CMonster*	pInstance = new CMonster(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMonster");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CMonster::Clone(void * pArg)
{
	CMonster*	pInstance = new CMonster(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster::Free()
{
	__super::Free();

	for (auto& pColliderCom : m_pColliderCom)
		Safe_Release(pColliderCom);
	
	Safe_Release(m_pShaderCom);	
	Safe_Release(m_pModelCom);
}

#include "stdafx.h"
#include "AT_Mouse.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Transform.h"

#include "Texture.h"
#include "VIBuffer_Square.h"

CAT_Mouse::CAT_Mouse(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:
	CGameObject(pDevice, pContext)
{
}

CAT_Mouse::CAT_Mouse(const CAT_Mouse& Prototype)
	:
	CGameObject(Prototype)
{
}

HRESULT CAT_Mouse::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAT_Mouse::Initialize(void* pArg)
{
	if (__super::Initialize(pArg))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Scaling(0.2f, 0.2f, 0.2f);

	return S_OK;
}

void CAT_Mouse::Priority_Update(_float fTimeDelta)
{
}

void CAT_Mouse::Update(_float fTimeDelta)
{
	m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix());
}

void CAT_Mouse::Late_Update(_float fTimeDelta)
{
	// m_pGameInstance->Add_RenderObject()

	m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);


	m_pGameInstance->Add_DebugComponent(m_pColliderCom);
}

HRESULT CAT_Mouse::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	m_pShaderCom->Begin(0);

	m_pVIBufferCom->Bind_BufferDesc();

	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CAT_Mouse::Ready_Components()
{
	/* Com_Collider */
	CBounding_Sphere::BOUND_SPHERE_DESC		ColliderDesc{};
	ColliderDesc.fRadius = 0.2f;
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.fRadius, 0.f);

	if (FAILED(Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Collider_Sphere"), TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		return E_FAIL;

	if (Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Texture_Test"), TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr) != S_OK)
		return E_FAIL;
		
	if (Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_VIBuffer_Square"), TEXT("Com_Buffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr) != S_OK)
		return E_FAIL;

	if (Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Shader_VtxPosNorTex"), TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr) != S_OK)
		return E_FAIL;

	return S_OK;
}

HRESULT CAT_Mouse::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(m_pShaderCom, "g_DepthTexture", TEXT("Target_Depth"))))
		return E_FAIL;

	return S_OK;
}

CAT_Mouse* CAT_Mouse::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAT_Mouse* pInstance = new CAT_Mouse(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAT_Mouse");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAT_Mouse::Clone(void* pArg)
{
	CAT_Mouse* pInstance = new CAT_Mouse(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAT_Mouse");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAT_Mouse::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pTextureCom);
}

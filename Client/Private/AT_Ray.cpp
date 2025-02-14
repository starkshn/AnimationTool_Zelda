#include "stdafx.h"
#include "AT_Ray.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Transform.h"
#include "VIBuffer_Line.h"


CAT_Ray::CAT_Ray(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:
	CGameObject(pDevice, pContext)
{
}

CAT_Ray::CAT_Ray(const CAT_Ray& Prototype)
	:
	CGameObject(Prototype)
{
}

HRESULT CAT_Ray::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAT_Ray::Initialize(void* pArg)
{
	if (__super::Initialize(pArg))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Scaling(5.f, 5.f, 5.f);

	return S_OK;
}

void CAT_Ray::Priority_Update(_float fTimeDelta)
{
}

void CAT_Ray::Update(_float fTimeDelta)
{
}

void CAT_Ray::Late_Update(_float fTimeDelta)
{
	if (Get_Use())
		m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
}

HRESULT CAT_Ray::Render()
{
   	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	m_pShaderCom->Begin(0);

	m_pVIBufferCom->Bind_BufferDesc();

	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CAT_Ray::Ready_Components()
{
	if (Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_VIBuffer_Line"), TEXT("Com_Buffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr) != S_OK)
		return E_FAIL;

	if (Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Shader_VtxPosColor"), TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr) != S_OK)
		return E_FAIL;

	return S_OK;
}

HRESULT CAT_Ray::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof _float3)))
		return E_FAIL;

	return S_OK;
}

CAT_Ray* CAT_Ray::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAT_Ray* pInstance = new CAT_Ray(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAT_Ray");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAT_Ray::Clone(void* pArg)
{
	CAT_Ray* pInstance = new CAT_Ray(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAT_Ray");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAT_Ray::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pTextureCom);
}

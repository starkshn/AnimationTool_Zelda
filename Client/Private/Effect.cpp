#include "stdafx.h"
#include "..\Public\Effect.h"

#include "GameInstance.h"


CEffect::CEffect(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CAlphaObject{ pDevice, pContext }
{
}

CEffect::CEffect(const CEffect & Prototype)
	: CAlphaObject{ Prototype }
{
}

HRESULT CEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEffect::Initialize(void * pArg)
{
    m_preview = pArg && static_cast<PREVIEW_DESC*>(pArg)->preview;
    m_previewSpark = m_preview && static_cast<PREVIEW_DESC*>(pArg)->spark;
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;	


	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_pGameInstance->Compute_Random(0.f, 20.f), 2.f, m_pGameInstance->Compute_Random(0.f, 20.f), 1.f));

	return S_OK;
}

void CEffect::Priority_Update(_float fTimeDelta)
{
	int a = 10;
}

void CEffect::Trigger_Preview(_vector position)
{
    m_previewActive = true;
    m_fFrame = 0.f;
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, position);
    m_pTransformCom->Scaling(0.7f, 0.7f, 0.7f);
}

void CEffect::Update(_float fTimeDelta)
{
    if (m_preview)
    {
        if (!m_previewActive) { return; }
        m_fFrame += 200.f * fTimeDelta;
        if (m_fFrame >= (m_previewSpark ? 50.f : 90.f)) { m_previewActive = false; return; }
        if (m_previewSpark)
        {
            const float size = 0.9f - 0.7f * (m_fFrame / 50.f);
            m_pTransformCom->Scaling(size, size, size);
        }
        m_pTransformCom->LookAt(XMLoadFloat4(m_pGameInstance->Get_CamPosition()));
        Compute_ViewZ();
        return;
    }
	m_fFrame += 90.f * fTimeDelta;

	if (m_fFrame >= 90.f)
		m_fFrame = 0.f;

	Compute_ViewZ();
}

void CEffect::Late_Update(_float fTimeDelta)
{
    if (m_preview && !m_previewActive) { return; }
	m_pGameInstance->Add_RenderObject(CRenderer::RG_BLEND, this);
}

HRESULT CEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	m_pShaderCom->Begin(m_preview ? 2 : 1);

	m_pVIBufferCom->Bind_BufferDesc();

	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CEffect::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_VIBuffer */
	if (FAILED(Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	/* Com_Texture */
	if (FAILED(Add_Component(m_preview ? LEVEL_ANIMTOOL : LEVEL_GAMEPLAY, m_previewSpark ? TEXT("Prototype_Component_Texture_EventSpark") : TEXT("Prototype_Component_Texture_Effect"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	return S_OK;	
}

HRESULT CEffect::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;	

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_previewSpark ? 0 : static_cast<_uint>(m_fFrame))))
		return E_FAIL;

	if (!m_preview && FAILED(m_pGameInstance->Bind_RT_ShaderResource(m_pShaderCom, "g_DepthTexture", TEXT("Target_Depth"))))
		return E_FAIL;

	return S_OK;
}

CEffect * CEffect::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CEffect*	pInstance = new CEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CEffect::Clone(void * pArg)
{
	CEffect*	pInstance = new CEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEffect::Free()
{
	__super::Free();

	
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}

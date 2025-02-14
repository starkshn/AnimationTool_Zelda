#include "stdafx.h"
#include "AT_Stair.h"
#include "GameInstance.h"

CAT_Stair::CAT_Stair(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :
    CGameObject(pDevice, pContext)
{

}

CAT_Stair::CAT_Stair(const CAT_Stair& Prototype)
    :
    CGameObject(Prototype)
{

}

HRESULT CAT_Stair::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAT_Stair::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    Get_Transform()->Scaling(0.01f, 0.01f, 0.01f);
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, _fvector{ 0.f, 1.f, -10.f, 1.f });

    return S_OK;
}

void CAT_Stair::Priority_Update(_float fTimeDelta)
{

}

void CAT_Stair::Update(_float fTimeDelta)
{
   
}

void CAT_Stair::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
}

HRESULT CAT_Stair::Render()
{
      if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

     _uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
            return E_FAIL;

        m_pShaderCom->Begin(0);
        m_pModelCom->Render(i);
    }

    return S_OK;
}


HRESULT CAT_Stair::Ready_Components()
{
    if (FAILED(Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Shader_VtxMesh"), TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    if (FAILED(Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Model_AT_Stair"), TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;

    return S_OK;
}

HRESULT CAT_Stair::Bind_ShaderResources()
{
    //if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
    //    return E_FAIL;
    //if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
    //    return E_FAIL;
    //if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
    //    return E_FAIL;


    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    _float4x4 viewMatrix = m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW);
    _float4x4 projMatrix = m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ);

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &viewMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &projMatrix)))
        return E_FAIL;

    return S_OK;


    return S_OK;
}

CAT_Stair* CAT_Stair::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAT_Stair* pInstance = new CAT_Stair(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAT_Stair");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CAT_Stair::Clone(void* pArg)
{
    CAT_Stair* pInstance = new CAT_Stair(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CAT_Stair");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAT_Stair::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}

#include "stdafx.h"
#include "AT_Terrain.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Transform.h"
#include "Model.h"

CAT_Terrain::CAT_Terrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :
    CGameObject(pDevice, pContext)
{
}

CAT_Terrain::CAT_Terrain(const CAT_Terrain& Prototype)
    :
    CGameObject(Prototype)
{
}

HRESULT CAT_Terrain::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAT_Terrain::Initialize(void* pArg)
{
    if (__super::Initialize(pArg))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pTransformCom->Set_State(CTransform::STATE_POSITION, _fvector{ 0.f, 0.f, 0.f, 1.f });

    return S_OK;
}

void CAT_Terrain::Priority_Update(_float fTimeDelta)
{
}

void CAT_Terrain::Update(_float fTimeDelta)
{
}

void CAT_Terrain::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
}

HRESULT CAT_Terrain::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        // i 번째에 해당하는 텍스쳐를 Bind 해준다.
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
            return E_FAIL;

        m_pShaderCom->Begin(0);

        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CAT_Terrain::Ready_Components()
{
    // Com_Shader
    {
        if (FAILED(Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Shader_VtxMesh"), TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
            return E_FAIL;
    }

    // Com_Model
    {
        if (FAILED(Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Model_AT_Terrain"), TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
            return E_FAIL;
    }

    return S_OK; 
}

HRESULT CAT_Terrain::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    _float4x4 viewMatrix = m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW);
    _float4x4 projMatrix = m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ);

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &viewMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &projMatrix)))
        return E_FAIL;

    return S_OK;
}

CAT_Terrain* CAT_Terrain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAT_Terrain* pInst = new CAT_Terrain(pDevice, pContext);

    if (FAILED(pInst->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAT_Terrain");
        Safe_Release(pInst);
    }

    return pInst;
}

CGameObject* CAT_Terrain::Clone(void* pArg)
{
    CAT_Terrain* pInst = new CAT_Terrain(*this);

    if (FAILED(pInst->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CAT_Terrain");
        Safe_Release(pInst);
    }

    return pInst;
}

void CAT_Terrain::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}

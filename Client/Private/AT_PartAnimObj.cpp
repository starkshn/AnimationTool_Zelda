#include "stdafx.h"
#include "AT_PartAnimObj.h"
#include "GameInstance.h"
#include "AT_Obj.h"
#include "Animation.h"

#include <iostream>
#include <filesystem>
using namespace std;

CAT_PartAnimObj::CAT_PartAnimObj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :
    CGameObject(pDevice, pContext)
{
}

CAT_PartAnimObj::CAT_PartAnimObj(const CAT_PartAnimObj& Prototype)
    :
    CGameObject(Prototype)
{
}

const _float4x4* CAT_PartAnimObj::Get_BoneMatrix(const _char* pBoneName) const
{
    return m_pModelCom->Get_BoneMatrix(pBoneName);
}

HRESULT CAT_PartAnimObj::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAT_PartAnimObj::Initialize(void* pArg)
{
    if (pArg)
    {
        AT_PARTANIMOBJ_DESC* pDesc = static_cast<AT_PARTANIMOBJ_DESC*>(pArg);
        m_strModelProtoName = pDesc->strModelProtoName;
    }

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    // 0 번 애니매이션 플레이 리스트에 추가
    // Add_AnimIn_AnimPlayList(0);

    // 꺼내와서 재생하는 애니매이션 설정
    // auto& Pair = m_AnimPlayLists.front();
    // Set_CurAnimIdx(Pair.first, Pair.second);

    Set_CurAnimIdx(56, 1);

    return S_OK;
}

void CAT_PartAnimObj::Priority_Update(_float fTimeDelta)
{

}

void CAT_PartAnimObj::Update(_float fTimeDelta)
{
    // 보간중인경우
    if (m_pModelCom->Get_Interpolationing())
    {
        
    }
    // 보간중이 아닌 경우
    else
    {
        // 기본 재생 애니매이션이 끝난 경우 애니매이션이 끝난다면
        if (m_pModelCom->Get_CurPlayAnimEnd())
        {
            if (Get_AnimPlayType() == PartAnimObj_Type::ANIM_PLAY_TYPE::DEFAULT)
            {
                Update_AnimLoop_Default(fTimeDelta);
            }
            else if (Get_AnimPlayType() == PartAnimObj_Type::ANIM_PLAY_TYPE::MULTI_DEFAULT)
            {
                Update_AnimLoop_NotInterpolation(fTimeDelta);
            }
            // 애니매이션 재생 타입이 보간 테스트인 경우
            else if (Get_AnimPlayType() == PartAnimObj_Type::ANIM_PLAY_TYPE::MULTI_INTERPOLATION)
            {
                Update_AnimLoop_Interpolation(fTimeDelta);
            }
        }

        // m_pModelCom->Set_Loop(false);
        // m_pModelCom->Set_BlendGroup(13, 67);
    }
    
    // 애니매이션 일반 재생의 경우
    if (!m_bIsPlayAnimBlend)
        m_pModelCom->Play_Animation(fTimeDelta);
    // 애니매이션 블렌딩 재생의 경우 (플레이어는 아래 함수 써야한다)
    else
    {
        m_pModelCom->Play_Animation(fTimeDelta, true);
    }


    // 루트 본 따라가는 경우
    if (m_bFollowRootBone)
    {
        m_MatRootBoneCombindedTransformationMatrix = m_pModelCom->Get_RootBoneCombinedTransfomationMatrix();
    }

    // 최종 월드 행렬 업데이트
    XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));
}

void CAT_PartAnimObj::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
}

HRESULT CAT_PartAnimObj::Render()
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

HRESULT CAT_PartAnimObj::Render_Shadow()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;


    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Shadow_Transform(CShadow::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Shadow_Transform(CShadow::D3DTS_PROJ))))
        return E_FAIL;

    _uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        m_pShaderCom->Begin(2);
        m_pModelCom->Render(i);
    }

    return S_OK;
}

void CAT_PartAnimObj::Update_AnimLoop_Default(_float fTimeDelta)
{
    if (m_AnimPlayLists.size() == 1)
    {
        {
            // m_pModelCom->Reset_CurAnim();
            // m_pModelCom->SetUp_Animation(_uint(m_AnimPlayLists.front().first), true);
        }

        // m_pModelCom->SetUp_Animation(_uint(m_AnimPlayLists.front().first), true);
    }
}

void CAT_PartAnimObj::Update_AnimLoop_NotInterpolation(_float fTimeDelta)
{
    // 단일이든 루프 이든 현재 애니매이션의 한번의 재생이 끝나면 호출 된다.

    // 애니매이션 재생을 순환 시킨다.
    if (!m_AnimPlayLists.empty())
    {
        auto Cur = m_AnimPlayLists.front();
        m_AnimPlayLists.pop_front();

        auto Next = m_AnimPlayLists.front();
        m_AnimPlayLists.push_back(Cur);             // 가장 뒤에 넣는다.

        m_pModelCom->Reset_CurAnim();
        m_pModelCom->SetUp_Animation(Next.first, Next.second);
        m_iCurAnimIdx = Next.first;
    }

    // Level_AnimTool에 애니매이션이 끝났다는 로그를 남긴다.
    Log_AnimEnd();
}

void CAT_PartAnimObj::Update_AnimLoop_Interpolation(_float fTimeDelta)
{   
    // 애니매이션이 끝난다면
    // Log_AnimEnd("anim end start interpolation to dst anim");
        
    // 애니매이션 재생을 순환 시킨다.
    // - 본인은 애니매이션이 끝난 시점에서 다음 애니매이션의 처음으로 보간해준다.
    if (!m_AnimPlayLists.empty())
    {
        auto Cur = m_AnimPlayLists.front();
        m_AnimPlayLists.pop_front();

        auto Next = m_AnimPlayLists.front();

        m_AnimPlayLists.push_back(Cur);             // 재생이 끝난 애니매이션을 가장 뒤에 넣는다.

        // 현재 애니매이션을 다음 애니매이션으로 변경한다.
        m_iCurAnimIdx = Next.first;

        // 덱의 앞부분 애니매이션 인덱스를 보간할 목적지 인덱스로 넣어준다.
        m_pModelCom->Change_Anim(Next.first, true);
    }
}

void CAT_PartAnimObj::Update_AnimLoop_Interpolation_End(_float fTimeDelta)
{
    // Log_AnimEnd("anim interpolation end");
    
    auto Cur = m_AnimPlayLists.front();
    m_pModelCom->SetUp_Animation(Cur.first, Cur.second);
}


_int CAT_PartAnimObj::Get_CurAnimIdx()
{
    return m_iCurAnimIdx;
}

void CAT_PartAnimObj::Set_CurAnimIdx(_int iCurAnimIdx, _bool bLoop)
{
    if (iCurAnimIdx < 0)
        return;

    m_iCurAnimIdx = iCurAnimIdx; 
    bLoop == 1 ? m_bCurAnimLoop = 1 : m_bCurAnimLoop = 0;
    m_pModelCom->SetUp_Animation(m_iCurAnimIdx, bLoop);
}


_float CAT_PartAnimObj::Get_Anim_TickPerSecond()
{
    return m_pModelCom->Get_Anim_TickPerSecond();
}

_float CAT_PartAnimObj::Get_Anim_Duration()
{
    return m_pModelCom->Get_Anim_Duration();
}

_float CAT_PartAnimObj::Get_Anim_CurTrackPos()
{
    return m_pModelCom->Get_Anim_CurTrackPos();
}

void CAT_PartAnimObj::StopAndPlay_CurAnim()
{
    static _bool bPlay = true;
    
    if (bPlay)
    {
        bPlay = false;
        m_pModelCom->Stop_CurAnim();
    }
    else
    {
        bPlay = true;
        m_pModelCom->Play_CurAnim();
    }
}

void CAT_PartAnimObj::TestFunc()
{
    static _int a = 0;
    cout << "Test" << ++a << endl;
}

void CAT_PartAnimObj::Log_AnimEnd()
{
    // cout << "Anim End" << endl;
    string strLog = "AnimEnd | ";
    strLog += m_pModelCom->Get_CurAnim()->Get_AnimName();
    strLog += "Duration : " + to_string(m_pModelCom->Get_CurAnim()->Get_AnimDuration()) + " | ";
    strLog += "TickPerSecond : " + to_string(m_pModelCom->Get_CurAnim()->Get_TickPerSecond()) + " | ";
    strLog += "TrackPos : " + to_string(m_pModelCom->Get_CurAnim()->Get_CurTrackPos());
    // m_Logs.push_back(strLog);
}

void CAT_PartAnimObj::Log_AnimEnd(string strLog)
{
    m_Logs.push_back(strLog);
}


HRESULT CAT_PartAnimObj::Ready_Components()
{
    if (FAILED(Add_Component(LEVEL_ANIMTOOL, TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
        return E_FAIL;

    if (FAILED(Add_Component(LEVEL_ANIMTOOL, m_strModelProtoName, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
        return E_FAIL;

    CModel* pModelCom = static_cast<CModel*>(this->Find_Component(TEXT("Com_Model")));

    // _wstring strRelativeFilePath = L"..\\Bin\\DataFiles\\AnimData\\FionaInterpolation.xml";



    _wstring strRelativeFilePath = L"..\\Bin\\DataFiles\\AnimData\\LinkAnimInterpolationData.xml";
    


    filesystem::path absolutePath = filesystem::absolute(strRelativeFilePath);

    // 보간 데이터 로드
    if (FAILED(pModelCom->Load_Anim_InterXML(absolutePath.wstring().c_str())))
    {
        MSG_BOX("XML Load_Anim_InterXML 실패");
        return E_FAIL;
    }

    // _wstring strRootMotionFilePath = L"..\\Bin\\DataFiles\\AnimData\\FionaRootMotionBoneName.xml";

    _wstring strRootMotionFilePath = L"..\\Bin\\DataFiles\\AnimData\\LinkRootMotionBoneName.xml";
    if (FAILED(pModelCom->Load_Anim_RootMotionBoneXML(strRootMotionFilePath.c_str())))
    {
        MSG_BOX("XML Load_Anim_RootMotionBoneXML Fail");
        return E_FAIL;
    }

    _wstring strblendBonesFilePath = L"..\\Bin\\DataFiles\\AnimData\\LinkBlendBones.xml";
    if (FAILED(pModelCom->Load_BlendBones(strblendBonesFilePath.c_str())))
        return E_FAIL;

    _wstring strAnimEventFilePath = L"..\\Bin\\DataFiles\\AnimData\\LinkAnimationEvents.xml";
    if (FAILED(pModelCom->Load_AnimEvents(strAnimEventFilePath.c_str())))
    {
        return E_FAIL;
    }

    function<void()> callback = bind(&CAT_PartAnimObj::TestFunc, this);
    if (pModelCom->Bind_Event("Armature|Nml_Move_Dash", "Sound", "Left", callback) != S_OK)
        return E_FAIL;

    if (pModelCom->Bind_Event("Armature|Nml_Move_Dash", "Sound", "Right", callback) != S_OK)
        return E_FAIL;

    if (pModelCom->Bind_Event("Armature|Nml_Move_Dash", "Sound", "1", callback) != S_OK)
        return E_FAIL;

    if (pModelCom->Bind_Event("Armature|Nml_Move_Dash", "Sound", "11", callback) != S_OK)
        return E_FAIL;

    if (pModelCom->Bind_Event("Armature|Nml_Move_Dash", "Sound", "111", callback) != S_OK)
        return E_FAIL;

    if (pModelCom->Bind_Event("Armature|Nml_Move_Dash", "Sound", "1111", callback) != S_OK)
        return E_FAIL;

    if (pModelCom->Bind_Event("Armature|Nml_Move_Dash", "Sound", "11111", callback) != S_OK)
        return E_FAIL;

    return S_OK;
}

HRESULT CAT_PartAnimObj::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    return S_OK;
}

CAT_PartAnimObj* CAT_PartAnimObj::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAT_PartAnimObj* pInstance = new CAT_PartAnimObj(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAT_PartAnimObj");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CAT_PartAnimObj::Clone(void* pArg)
{
    CAT_PartAnimObj* pInstance = new CAT_PartAnimObj(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CAT_PartAnimObj");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAT_PartAnimObj::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}

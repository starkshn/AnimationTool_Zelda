#include "stdafx.h"
#include "AT_Cam.h"
#include "GameInstance.h"
#include "imgui.h"

CAnimTool_Camera::CAnimTool_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :
    CCamera(pDevice, pContext)
{
}

CAnimTool_Camera::CAnimTool_Camera(const CAnimTool_Camera& Prototype)
    :
    CCamera(Prototype)
{
}

HRESULT CAnimTool_Camera::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAnimTool_Camera::Initialize(void* pArg)
{
    CAMERA_FREE_DESC* pDesc = static_cast<CAMERA_FREE_DESC*>(pArg);

    m_fMouseSensor = pDesc->fMouseSensor;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CAnimTool_Camera::Priority_Update(_float fTimeDelta)
{
    RECT clientArea{};
    GetClientRect(g_hWnd, &clientArea);
    if (clientArea.right > 0 && clientArea.bottom > 0)
    {
        m_fAspect = static_cast<float>(clientArea.right) / clientArea.bottom;
    }
    const bool active = GetForegroundWindow() == g_hWnd;
    const bool captureKeyboard = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard;
    const bool captureMouse = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse;
    if (active && !captureKeyboard)
    {
        if (KEY_PRESSING(KEY::W)) { m_pTransformCom->Go_Straight(fTimeDelta); }
        if (KEY_PRESSING(KEY::S)) { m_pTransformCom->Go_Backward(fTimeDelta); }
        if (KEY_PRESSING(KEY::A)) { m_pTransformCom->Go_Left(fTimeDelta); }
        if (KEY_PRESSING(KEY::D)) { m_pTransformCom->Go_Right(fTimeDelta); }
        if (KEY_PRESSING(KEY::Q)) { m_pTransformCom->Go_Down(fTimeDelta); }
        if (KEY_PRESSING(KEY::E)) { m_pTransformCom->Go_Up(fTimeDelta); }
    }
    if (active && !captureMouse && (GetAsyncKeyState(VK_RBUTTON) & 0x8000))
    {
        const _long mouseX = MOUSE_MOVE(MOUSE_MOVE::X);
        const _long mouseY = MOUSE_MOVE(MOUSE_MOVE::Y);
        m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), mouseX * fTimeDelta * m_fMouseSensor);
        m_pTransformCom->Turn(Get_Transform()->Get_State(CTransform::STATE_RIGHT), mouseY * fTimeDelta * m_fMouseSensor);
    }
    __super::Compute_PipeLineMatrices();
}

void CAnimTool_Camera::Update(_float fTimeDelta)
{
}

void CAnimTool_Camera::Late_Update(_float fTimeDelta)
{
}

HRESULT CAnimTool_Camera::Render()
{
    return S_OK;
}

CAnimTool_Camera* CAnimTool_Camera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimTool_Camera* pInstance = new CAnimTool_Camera(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAnimTool_Camera");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CAnimTool_Camera::Clone(void* pArg)
{
    CAnimTool_Camera* pInstance = new CAnimTool_Camera(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CAnimTool_Camera");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAnimTool_Camera::Free()
{
    __super::Free();
}

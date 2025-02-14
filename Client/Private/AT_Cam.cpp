#include "stdafx.h"
#include "AT_Cam.h"
#include "GameInstance.h"

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

	if (KEY_DOWN(KEY::TAB))
	{
		m_bBlockCamMove == false ? m_bBlockCamMove = true : m_bBlockCamMove = false;
	}

	if (KEY_PRESSING(KEY::W))
	{
		if (m_bBlockCamMove) return;

		m_pTransformCom->Go_Straight(fTimeDelta);
	}

	if (KEY_PRESSING(KEY::S))
	{
		if (m_bBlockCamMove) return;

		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	if (KEY_PRESSING(KEY::A))
	{
		if (m_bBlockCamMove) return;

		m_pTransformCom->Go_Left(fTimeDelta);
	}

	if (KEY_PRESSING(KEY::D))
	{
		if (m_bBlockCamMove) return;

		m_pTransformCom->Go_Right(fTimeDelta);
	}

	if (KEY_PRESSING(KEY::E))
	{
		if (m_bBlockCamMove) return;

		m_pTransformCom->Go_Up(fTimeDelta);
	}

	if (KEY_PRESSING(KEY::Q))
	{
		if (m_bBlockCamMove) return;

		m_pTransformCom->Go_Down(fTimeDelta);
	}

	_long MouseMove = {};

	if (MouseMove = MOUSE_MOVE(MOUSE_MOVE::X))
	{
		if (m_bBlockCamMove) 
			return;

		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), MouseMove * fTimeDelta * m_fMouseSensor);
	}

	if (MouseMove = MOUSE_MOVE(MOUSE_MOVE::Y))
	{
		if (m_bBlockCamMove) 
			return;

		m_pTransformCom->Turn(Get_Transform()->Get_State(CTransform::STATE_RIGHT), MouseMove * fTimeDelta * m_fMouseSensor);
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

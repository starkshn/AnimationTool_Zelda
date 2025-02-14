#include "stdafx.h"
#include "..\Public\Level_GamePlay.h"

#include "GameInstance.h"
#include "Camera_Free.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_GamePlay::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_HeightCamera()))
		return E_FAIL;

	if (FAILED(Ready_ShadowCamera()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camara"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Player(TEXT("Layer_Player"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Effect(TEXT("Layer_Effect"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Monster(TEXT("Layer_Monster"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	m_fTime += fTimeDelta;
	++m_iFrameCount;
#endif
}

HRESULT CLevel_GamePlay::Render()
{
#ifdef _DEBUG

	if (1.f <= m_fTime)
	{
		wstring strFrame;
		strFrame = TEXT("게임플레이레벨입니다 / Frame : ") + to_wstring(m_iFrameCount);
		SetWindowText(g_hWnd, strFrame.c_str());
		m_iFrameCount = 0;
		m_fTime = 0.f;
	}

#endif
	
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	ZeroMemory(&LightDesc, sizeof LightDesc);

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTOINAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.7f, 0.7f, 0.7f, 1.f);
	LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	ZeroMemory(&LightDesc, sizeof LightDesc);

	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(10.f, 3.f, 10.f, 0.f);
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(1.f, 0.f, 0.f, 1.f);
	LightDesc.vAmbient = _float4(0.5f, 0.2f, 0.2f, 1.f);
	LightDesc.vSpecular = LightDesc.vDiffuse;

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	ZeroMemory(&LightDesc, sizeof LightDesc);

	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(20.f, 3.f, 10.f, 0.f);
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(0.f, 1.f, 0.f, 1.f);
	LightDesc.vAmbient = _float4(0.2f, 0.5f, 0.2f, 1.f);
	LightDesc.vSpecular = LightDesc.vDiffuse;

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_HeightCamera()
{
	/*XMMatrixLookAtLH(XMVectorSet(65.f, 20.f, 65.f, 1.f), XMVectorSet(65.f, 0.f, 65.f, 1.f), XMVectorSet(0.f, 1.f, 0.f, 0.f));*/

	_matrix			CamWorldMatrix = XMMatrixIdentity();
	CamWorldMatrix.r[0] = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	CamWorldMatrix.r[1] = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	CamWorldMatrix.r[2] = XMVectorSet(0.f, -1.f, 0.f, 0.f);
	CamWorldMatrix.r[3] = XMVectorSet(65.f, 20.f, 65.f, 1.f);

	m_pGameInstance->SetUp_Height_TransformMatrix(CHeight::D3DTS_VIEW, XMMatrixInverse(nullptr, CamWorldMatrix));
	m_pGameInstance->SetUp_Height_TransformMatrix(CHeight::D3DTS_PROJ, XMMatrixOrthographicLH(200.f, 200.f, 0.f, 100.f));

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_ShadowCamera()
{	
	m_pGameInstance->SetUp_Shadow_TransformMatrix(CShadow::D3DTS_VIEW, XMMatrixLookAtLH(XMVectorSet(-10.f, 50.f, -10.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	m_pGameInstance->SetUp_Shadow_TransformMatrix(CShadow::D3DTS_PROJ, XMMatrixPerspectiveFovLH(XMConvertToRadians(60.f), static_cast<_float>(g_iWinSizeX) / g_iWinSizeY, 0.1f, 1000.f));

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Camera(const _wstring & strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC		Desc{};

	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = XMConvertToRadians(180.f);
	Desc.fMouseSensor = 0.1f;
	Desc.fFovy = XMConvertToRadians(60.f);
	Desc.fAspect = static_cast<_float>(g_iWinSizeX) / g_iWinSizeY;
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;
	Desc.vEye = _float3(0.f, 10.f, -15.f);
	Desc.vAt = _float3(0.f, 0.f, 0.f);

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Camera_Free"),
		LEVEL_GAMEPLAY, strLayerTag, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Player(const _wstring & strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Player"),
		LEVEL_GAMEPLAY, strLayerTag)))
		return E_FAIL;
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Effect(const _wstring & strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Particle_Explosion"),
		LEVEL_GAMEPLAY, strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Particle_Snow"),
		LEVEL_GAMEPLAY, strLayerTag)))
		return E_FAIL;

	for (size_t i = 0; i < 40; i++)
	{
		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Effect"),
			LEVEL_GAMEPLAY, strLayerTag, nullptr, TEXT("Character"))))
			return E_FAIL;		
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Monster(const _wstring & strLayerTag)
{
	/*for (size_t i = 0; i < 20; i++)
	{
		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Monster"),
			LEVEL_GAMEPLAY, strLayerTag)))
			return E_FAIL;
	}*/
	

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_BackGround(const _wstring & strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Terrain"), LEVEL_GAMEPLAY, strLayerTag, nullptr)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Prototype_GameObject_Sky"), LEVEL_GAMEPLAY, strLayerTag, nullptr)))
		return E_FAIL;

	return S_OK;
}

CLevel_GamePlay * CLevel_GamePlay::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CLevel_GamePlay*	pInstance = new CLevel_GamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_GamePlay");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_GamePlay::Free()
{
	__super::Free();
}

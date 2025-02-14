#include "..\Public\Picking.h"

#include "GameInstance.h"

CPicking::CPicking(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPicking::Initialize(HWND hWnd)
{
	m_hWnd = hWnd;

	_uint					iNumViewports = { 1 };
	D3D11_VIEWPORT			ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	D3D11_TEXTURE2D_DESC	TextureDesc{};

	TextureDesc.Width = m_iWidth = ViewportDesc.Width;
	TextureDesc.Height = m_iHeight = ViewportDesc.Height;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_STAGING;
	TextureDesc.BindFlags = 0;
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
		return E_FAIL;

	return S_OK;
}

void CPicking::Update()
{
	if (FAILED(m_pGameInstance->Copy_RT_Resource(TEXT("Target_PickWorld"), m_pTexture2D)))
		return;
}

_bool CPicking::isPicked(_float4 * pWorldPos)
{
	_bool			isPicked = { false };

	POINT			ptMouse = {};
	GetCursorPos(&ptMouse);

	/* ºäÆ÷Æ® »óÀÇ ¸¶¿ì½º ÁÂÇ¥·Î º¯È¯ÇÏ³®. */
	ScreenToClient(m_hWnd, &ptMouse);

	_uint		iPixelIndex = ptMouse.y * m_iWidth + ptMouse.x;

	D3D11_MAPPED_SUBRESOURCE			SubResource{};

	m_pContext->Map(m_pTexture2D, 0, D3D11_MAP_READ_WRITE, 0, &SubResource);

	_float4*		pPixels = static_cast<_float4*>(SubResource.pData);

	if (0.f < pPixels[iPixelIndex].w)
	{
		isPicked = true;
		*pWorldPos = pPixels[iPixelIndex];
	}

	m_pContext->Unmap(m_pTexture2D, 0);

	return isPicked;
}

CPicking * CPicking::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, HWND hWnd)
{
	CPicking*		pInstance = new CPicking(pDevice, pContext);

	if (FAILED(pInstance->Initialize(hWnd)))
	{
		MSG_BOX("Failed to Created : CPicking");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CPicking::Free()
{
	__super::Free();

	Safe_Release(m_pTexture2D);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}

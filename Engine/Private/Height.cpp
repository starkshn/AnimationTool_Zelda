#include "..\Public\Height.h"

#include "GameInstance.h"

/* 16384 */

CHeight::CHeight(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CHeight::Initialize()
{	
	_uint					iNumViewports = { 1 };
	D3D11_VIEWPORT			ViewportDesc{};

	/*m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);*/

	D3D11_TEXTURE2D_DESC	TextureDesc{};

	TextureDesc.Width = m_iWidth = /*ViewportDesc.Width*/g_iMaxWidth;
	TextureDesc.Height = m_iHeight = /*ViewportDesc.Height*/g_iMaxHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32_FLOAT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_STAGING;
	TextureDesc.BindFlags = 0;
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
		return E_FAIL;

	m_pHeights = new _float[g_iMaxWidth * g_iMaxHeight];
	ZeroMemory(m_pHeights, sizeof(_float) * g_iMaxWidth * g_iMaxHeight);

	return S_OK;
}

void CHeight::Update()
{
	if (FAILED(m_pGameInstance->Copy_RT_Resource(TEXT("Target_Height"), m_pTexture2D)))
		return;

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pTexture2D, 0, D3D11_MAP_READ_WRITE, 0, &SubResource);

	memcpy(m_pHeights, SubResource.pData, sizeof(_float) * (g_iMaxWidth * g_iMaxHeight));

	m_pContext->Unmap(m_pTexture2D, 0);

}

_float CHeight::Compute_Height(_fvector vWorldPos)
{
	_vector		vPosition = XMVector3TransformCoord(vWorldPos, XMLoadFloat4x4(&m_TransformMatrices[D3DTS_VIEW]));
	vPosition = XMVector3TransformCoord(vPosition, XMLoadFloat4x4(&m_TransformMatrices[D3DTS_PROJ]));

	_float2		vViewportPos = {};

	vViewportPos.x = vPosition.m128_f32[0] * (g_iMaxWidth * 0.5f) + g_iMaxWidth * 0.5f;
	vViewportPos.y = vPosition.m128_f32[1] * (g_iMaxHeight * -0.5f) + g_iMaxHeight * 0.5f;
	
	_uint			iPixelIndex = _uint(vViewportPos.y) * g_iMaxWidth + _uint(vViewportPos.x);

	_uint			iIndices[9] = {
		iPixelIndex - g_iMaxWidth - 1, 
		iPixelIndex - g_iMaxWidth,
		iPixelIndex - g_iMaxWidth + 1,
		iPixelIndex - 1, 
		iPixelIndex, 
		iPixelIndex + 1,
		iPixelIndex + g_iMaxWidth - 1,
		iPixelIndex + g_iMaxWidth,
		iPixelIndex + g_iMaxWidth + 1,	
	};
	
	_float		fHeight = {};

	for (size_t i = 0; i < 9; i++)
		fHeight += m_pHeights[iIndices[i]];

	return fHeight / 9.f;	
}

CHeight * CHeight::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CHeight*		pInstance = new CHeight(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CHeight");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHeight::Free()
{
	__super::Free();

	Safe_Delete_Array(m_pHeights);

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pTexture2D);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}

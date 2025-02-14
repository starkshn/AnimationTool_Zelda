#include "..\Public\Texture.h"
#include "Shader.h"

CTexture::CTexture(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent { pDevice, pContext }
{
}

CTexture::CTexture(const CTexture & Prototype)
	: CComponent{ Prototype }
	, m_iNumSRVs { Prototype.m_iNumSRVs }
	, m_SRVs { Prototype.m_SRVs }
{
	for (auto& pSRV : m_SRVs)
		Safe_AddRef(pSRV);
}

HRESULT CTexture::Initialize_Prototype(const _tchar * pTextureFilePath, _uint iNumTexture)
{
	m_iNumSRVs = iNumTexture;

	_tchar			szTextureFilePath[MAX_PATH] = TEXT("");

	for (size_t i = 0; i < m_iNumSRVs; i++)
	{
		wsprintf(szTextureFilePath, pTextureFilePath, i);

		/* 드라이브경로, 디렉토리경로, 파일네임, 파일확장자. */
		_tchar		szEXT[MAX_PATH] = TEXT("");

		/* D:\정의훈\147\3d\Framework\Client\Bin\Resources\Textures\Default.jpg */
		_wsplitpath_s(szTextureFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szEXT, MAX_PATH);

		HRESULT		hr = {};

		ID3D11ShaderResourceView*		pSRV = { nullptr };

		if (false == lstrcmpW(szEXT, TEXT(".dds")))
		{
			hr = DirectX::CreateDDSTextureFromFile(m_pDevice, szTextureFilePath, nullptr, &pSRV);
		}
		else if (false == lstrcmpW(szEXT, TEXT(".tga")))
		{

		}
		else
		{
			hr = DirectX::CreateWICTextureFromFile(m_pDevice, szTextureFilePath, nullptr, &pSRV);
		}
		

		if (FAILED(hr))
			return E_FAIL;

		m_SRVs.push_back(pSRV);
	}


	return S_OK;
}

HRESULT CTexture::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CTexture::Bind_ShaderResource(CShader * pShader, const _char * pConstantName, _uint iSRVIndex)
{
	if (iSRVIndex >= m_iNumSRVs)
		return E_FAIL;

	return pShader->Bind_SRV(pConstantName, m_SRVs[iSRVIndex]);	
}

HRESULT CTexture::Bind_ShaderResources(CShader * pShader, const _char * pConstantName)
{
	return pShader->Bind_SRVs(pConstantName, &m_SRVs.front(), m_SRVs.size());
}

CTexture * CTexture::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _tchar * pTextureFilePath, _uint iNumTextures)
{
	CTexture*	pInstance = new CTexture(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pTextureFilePath, iNumTextures)))
	{
		MSG_BOX("Failed to Created : CTexture");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CComponent * CTexture::Clone(void * pArg)
{
	CTexture*	pInstance = new CTexture(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTexture");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTexture::Free()
{
	__super::Free();

	for (auto& pSRV : m_SRVs)
		Safe_Release(pSRV);

	m_SRVs.clear();

}

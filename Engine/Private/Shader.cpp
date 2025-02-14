#include "..\Public\Shader.h"

CShader::CShader(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent { pDevice, pContext }
{
}

CShader::CShader(const CShader & Prototype)
	: CComponent{ Prototype }
	, m_pEffect { Prototype.m_pEffect }
	, m_InputLayouts{ Prototype.m_InputLayouts }
	, m_iNumLayouts{ Prototype.m_iNumLayouts }
{
	Safe_AddRef(m_pEffect);
	for (auto& pInputLayout : m_InputLayouts)
		Safe_AddRef(pInputLayout);
}

HRESULT CShader::Initialize_Prototype(const _tchar* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
	_uint		iHlslFlag = { 0 };

#ifdef _DEBUG
	iHlslFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	iHlslFlag = D3DCOMPILE_OPTIMIZATION_LEVEL1;

#endif

	ID3DBlob* pBuffer = { nullptr };

	if (FAILED(D3DX11CompileEffectFromFile(pShaderFilePath, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, iHlslFlag, 0, m_pDevice, &m_pEffect, &pBuffer)))
		return E_FAIL;

	ID3DX11EffectTechnique*	pTechnique = m_pEffect->GetTechniqueByIndex(0);
	if (nullptr == pTechnique)
		return E_FAIL;

	D3DX11_TECHNIQUE_DESC	TechniqueDesc{};

	pTechnique->GetDesc(&TechniqueDesc);

	m_iNumLayouts = TechniqueDesc.Passes;	

	for (size_t i = 0; i < m_iNumLayouts; i++)
	{
		ID3DX11EffectPass*		pPass = pTechnique->GetPassByIndex(i);
		if (nullptr == pPass)
			return E_FAIL;

		D3DX11_PASS_DESC		PassDesc{};

		pPass->GetDesc(&PassDesc);

		ID3D11InputLayout*		pInputLayout = { nullptr };

		if (FAILED(m_pDevice->CreateInputLayout(pElements, iNumElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayout)))
			return E_FAIL;

		m_InputLayouts.push_back(pInputLayout);
	}	

	return S_OK;
}

HRESULT CShader::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CShader::Begin(_uint iPassIndex)
{
	if (iPassIndex >= m_iNumLayouts)
		return E_FAIL;

	ID3DX11EffectPass*	pPass = m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(iPassIndex);

	pPass->Apply(0, m_pContext);

	/* 버퍼를 드로우하기전에 반드시 호출되어야하는 함수다. */
	m_pContext->IASetInputLayout(m_InputLayouts[iPassIndex]);

	return S_OK;
}

HRESULT CShader::Bind_RawValue(const _char * pConstantName, const void * pData, _uint iLength)
{
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	return pVariable->SetRawValue(pData, 0, iLength);	
}

HRESULT CShader::Bind_Matrix(const _char* pConstantName, const _float4x4 * pMatrix)
{
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectMatrixVariable*	pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;

	return pMatrixVariable->SetMatrix(reinterpret_cast<const _float*>(pMatrix));	
}

HRESULT CShader::Bind_Matrices(const _char * pConstantName, const _float4x4 * pMatrix, _uint iNumMatrices)
{
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectMatrixVariable*	pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;
	return pMatrixVariable->SetMatrixArray(reinterpret_cast<const _float*>(pMatrix), 0, iNumMatrices);
}

HRESULT CShader::Bind_SRV(const _char * pConstantName, ID3D11ShaderResourceView * pSRV)
{
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable*	pSRVVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVVariable)
		return E_FAIL;

	return pSRVVariable->SetResource(pSRV);	
}

HRESULT CShader::Bind_SRVs(const _char * pConstantName, ID3D11ShaderResourceView ** ppSRV, _uint iNumSRVs)
{
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable*	pSRVVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVVariable)
		return E_FAIL;

	return pSRVVariable->SetResourceArray(ppSRV, 0, iNumSRVs);	
}

CShader * CShader::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _tchar * pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
	CShader*	pInstance = new CShader(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pShaderFilePath, pElements, iNumElements)))
	{
		MSG_BOX("Failed to Created : CShader");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CComponent * CShader::Clone(void * pArg)
{
	CShader*	pInstance = new CShader(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CShader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CShader::Free()
{
	__super::Free();

	for (auto& pInputLayOut : m_InputLayouts)
		Safe_Release(pInputLayOut);

	m_InputLayouts.clear();

	Safe_Release(m_pEffect);
}

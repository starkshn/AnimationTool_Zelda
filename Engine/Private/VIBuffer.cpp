#include "..\Public\VIBuffer.h"

CVIBuffer::CVIBuffer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent { pDevice, pContext }
{

}

CVIBuffer::CVIBuffer(const CVIBuffer & Prototype)
	: CComponent{ Prototype }
	, m_pVB { Prototype.m_pVB }
	, m_pIB{ Prototype.m_pIB }
	, m_iVertexStride{ Prototype.m_iVertexStride }
	, m_iNumVertices{ Prototype.m_iNumVertices }
	, m_iNumVertexBuffers{ Prototype.m_iNumVertexBuffers }
	, m_iIndexStride{ Prototype.m_iIndexStride }
	, m_iNumIndices{ Prototype.m_iNumIndices }
	, m_eIndexFormat{ Prototype.m_eIndexFormat }
	, m_ePrimitiveTopology{ Prototype.m_ePrimitiveTopology }
	, m_pVertexPositions { Prototype.m_pVertexPositions }
{

	Safe_AddRef(m_pIB);
	Safe_AddRef(m_pVB);
}

HRESULT CVIBuffer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CVIBuffer::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CVIBuffer::Render()
{
	m_pContext->DrawIndexed(m_iNumIndices, 0, 0);

	//

	return S_OK;
}

HRESULT CVIBuffer::Bind_BufferDesc()
{
	ID3D11Buffer*			pVertexBuffers[] = {
		m_pVB,		 
	};

	_uint					iVertexStrides[] = {
		m_iVertexStride,

	};

	_uint					iOffsets[] = {
		0,
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	return S_OK;
}


HRESULT CVIBuffer::Create_Buffer(ID3D11Buffer ** ppOut)
{
	m_pDevice->CreateBuffer(&m_BufferDesc, &m_SubResourceDesc, ppOut);

	return S_OK;
}

void CVIBuffer::Free()
{
	__super::Free();

	if (false == m_isCloned)
		Safe_Delete_Array(m_pVertexPositions);

	Safe_Release(m_pVB);
	Safe_Release(m_pIB);
}

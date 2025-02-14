#include "VIBuffer_Line.h"

CVIBuffer_Line::CVIBuffer_Line(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:
	CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Line::CVIBuffer_Line(const CVIBuffer_Line& Prototype)
	:
	CVIBuffer(Prototype)
{
}

HRESULT CVIBuffer_Line::Initialize_Prototype()
{
	// Vertex Buffer 초기화
	{
		m_iNumVertexBuffers = 1;
		m_iVertexStride = sizeof(VTX_POSCOLOR);
		m_iNumVertices = 2;
	}

	VTX_POSCOLOR* pVertices = new VTX_POSCOLOR[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTX_POSCOLOR) * m_iNumVertices);

	// Index Buffer 초기화
	{
		m_iIndexStride = sizeof(_uint);
		m_iNumIndices = 2;
		m_eIndexFormat = DXGI_FORMAT_R32_UINT;
		m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	}

	_uint* pIndices = new _uint[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);


	pVertices[0].vPosition = XMFLOAT3(0.f, 0.f, -0.5f);
	pVertices[0].vColor = XMFLOAT3(1.f, 0.f, 0.f);

	pVertices[1].vPosition = XMFLOAT3(0.f, 0.f, 0.5f);
	pVertices[1].vColor = XMFLOAT3(1.f, 0.f, 0.f);

	ZeroMemory(&m_SubResourceDesc, sizeof m_SubResourceDesc);
	m_SubResourceDesc.pSysMem = pVertices;

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.StructureByteStride = m_iVertexStride;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	pIndices[0] = 0;
	pIndices[1] = 1;

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.StructureByteStride = 0;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;

	ZeroMemory(&m_SubResourceDesc, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceDesc.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
	Safe_Delete_Array(pIndices);

	return S_OK;
}

HRESULT CVIBuffer_Line::Initialize(void* pArg)
{
	return S_OK;
}

CVIBuffer_Line* CVIBuffer_Line::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVIBuffer_Line* pInstance = new CVIBuffer_Line(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Line");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Line::Clone(void* pArg)
{
	CVIBuffer_Line* pInstance = new CVIBuffer_Line(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Line");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Line::Free()
{
	__super::Free();
}

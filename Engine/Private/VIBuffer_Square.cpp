#include "VIBuffer_Square.h"

CVIBuffer_Square::CVIBuffer_Square(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:
	CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Square::CVIBuffer_Square(const CVIBuffer_Square& Prototype)
	:
	CVIBuffer(Prototype)
{
}

HRESULT CVIBuffer_Square::Initialize_Prototype()
{
	// Vertex Buffer 초기화
	{
		m_iNumVertexBuffers = 1;
		m_iVertexStride = sizeof(VTX_POSNORTEX);
		m_iNumVertices = 4 * 6; // 한 면당 정점의 개수 * 6면

	}

	VTX_POSNORTEX* pVertices = new VTX_POSNORTEX[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTX_POSNORTEX) * m_iNumVertices);


	// 인덱스 버퍼 초기화
	{
		m_iIndexStride = sizeof(_uint);
		m_iNumIndices = 6 * 6;
		m_eIndexFormat = DXGI_FORMAT_R32_UINT;
		m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	_uint* pIndices = new _uint[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);

	// 앞면 정점
	{
		pVertices[0].vPosition = XMFLOAT3(-0.5f, 0.5f, -0.5f);
		pVertices[0].vTexcoord = XMFLOAT2(0.f, 0.f);
		pVertices[0].vNormal = XMFLOAT3(0.f, 0.f, -1.f);

		pVertices[1].vPosition = XMFLOAT3(0.5f, 0.5f, -0.5f);
		pVertices[1].vTexcoord = XMFLOAT2(1.f, 0.f);
		pVertices[1].vNormal = XMFLOAT3(0.f, 0.f, -1.f);

		pVertices[2].vPosition = XMFLOAT3(0.5f, -0.5f, -0.5f);
		pVertices[2].vTexcoord = XMFLOAT2(1.f, 1.f);
		pVertices[2].vNormal = XMFLOAT3(0.f, 0.f, -1.f);

		pVertices[3].vPosition = XMFLOAT3(-0.5f, -0.5f, -0.5f);
		pVertices[3].vTexcoord = XMFLOAT2(0.f, 1.f);
		pVertices[3].vNormal = XMFLOAT3(0.f, 0.f, -1.f);

	}

	// 오른쪽 면 정점
	{
		pVertices[4].vPosition = XMFLOAT3(0.5f, 0.5f, -0.5f);
		pVertices[4].vTexcoord = XMFLOAT2(0.f, 0.f);
		pVertices[4].vNormal = XMFLOAT3(1.f, 0.f, 0.f);


		pVertices[5].vPosition = XMFLOAT3(0.5f, 0.5f, 0.5f);
		pVertices[5].vTexcoord = XMFLOAT2(1.f, 0.f);
		pVertices[5].vNormal = XMFLOAT3(1.f, 0.f, 0.f);


		pVertices[6].vPosition = XMFLOAT3(0.5f, -0.5f, 0.5f);
		pVertices[6].vTexcoord = XMFLOAT2(1.f, 1.f);
		pVertices[6].vNormal = XMFLOAT3(1.f, 0.f, 0.f);


		pVertices[7].vPosition = XMFLOAT3(0.5f, -0.5f, -0.5f);
		pVertices[7].vTexcoord = XMFLOAT2(0.f, 1.f);
		pVertices[7].vNormal = XMFLOAT3(1.f, 0.f, 0.f);

	}


	// 뒷 면 정점
	{
		pVertices[8].vPosition = XMFLOAT3(0.5f, 0.5f, 0.5f);
		pVertices[8].vTexcoord = XMFLOAT2(0.f, 0.f);
		pVertices[8].vNormal = XMFLOAT3(0.f, 0.f, 1.f);


		pVertices[9].vPosition = XMFLOAT3(-0.5f, 0.5f, 0.5f);
		pVertices[9].vTexcoord = XMFLOAT2(1.f, 0.f);
		pVertices[9].vNormal = XMFLOAT3(0.f, 0.f, 1.f);


		pVertices[10].vPosition = XMFLOAT3(-0.5f, -0.5f, 0.5f);
		pVertices[10].vTexcoord = XMFLOAT2(1.f, 1.f);
		pVertices[10].vNormal = XMFLOAT3(0.f, 0.f, 1.f);


		pVertices[11].vPosition = XMFLOAT3(0.5f, -0.5f, 0.5f);
		pVertices[11].vTexcoord = XMFLOAT2(0.f, 1.f);
		pVertices[11].vNormal = XMFLOAT3(0.f, 0.f, 1.f);
	}


	// 왼쪽 면 정점
	{
		pVertices[12].vPosition = XMFLOAT3(-0.5f, 0.5f, 0.5f);
		pVertices[12].vTexcoord = XMFLOAT2(0.f, 0.f);
		pVertices[12].vNormal = XMFLOAT3(-1.f, 0.f, 0.f);


		pVertices[13].vPosition = XMFLOAT3(-0.5f, 0.5f, -0.5f);
		pVertices[13].vTexcoord = XMFLOAT2(1.f, 0.f);
		pVertices[13].vNormal = XMFLOAT3(-1.f, 0.f, 0.f);


		pVertices[14].vPosition = XMFLOAT3(-0.5f, -0.5f, -0.5f);
		pVertices[14].vTexcoord = XMFLOAT2(1.f, 1.f);
		pVertices[14].vNormal = XMFLOAT3(-1.f, 0.f, 0.f);


		pVertices[15].vPosition = XMFLOAT3(-0.5f, -0.5f, 0.5f);
		pVertices[15].vTexcoord = XMFLOAT2(0.f, 1.f);
		pVertices[15].vNormal = XMFLOAT3(-1.f, 0.f, 0.f);
	}


	// 위쪽 면 정점
	{
		pVertices[16].vPosition = XMFLOAT3(-0.5f, 0.5f, 0.5f);
		pVertices[16].vTexcoord = XMFLOAT2(0.f, 0.f);
		pVertices[16].vNormal = XMFLOAT3(0.f, 1.f, 0.f);


		pVertices[17].vPosition = XMFLOAT3(0.5f, 0.5f, 0.5f);
		pVertices[17].vTexcoord = XMFLOAT2(1.f, 0.f);
		pVertices[17].vNormal = XMFLOAT3(0.f, 1.f, 0.f);


		pVertices[18].vPosition = XMFLOAT3(0.5f, 0.5f, -0.5f);
		pVertices[18].vTexcoord = XMFLOAT2(1.f, 1.f);
		pVertices[18].vNormal = XMFLOAT3(0.f, 1.f, 0.f);


		pVertices[19].vPosition = XMFLOAT3(-0.5f, 0.5f, -0.5f);
		pVertices[19].vTexcoord = XMFLOAT2(0.f, 1.f);
		pVertices[19].vNormal = XMFLOAT3(0.f, 1.f, 0.f);
	}


	// 아랫 면 정점
	{
		pVertices[20].vPosition = XMFLOAT3(-0.5f, -0.5f, -0.5f);
		pVertices[20].vTexcoord = XMFLOAT2(0.f, 0.f);
		pVertices[20].vNormal = XMFLOAT3(0.f, -1.f, 0.f);


		pVertices[21].vPosition = XMFLOAT3(0.5f, -0.5f, -0.5f);
		pVertices[21].vTexcoord = XMFLOAT2(1.f, 0.f);
		pVertices[21].vNormal = XMFLOAT3(0.f, -1.f, 0.f);


		pVertices[22].vPosition = XMFLOAT3(0.5f, -0.5f, 0.5f);
		pVertices[22].vTexcoord = XMFLOAT2(1.f, 1.f);
		pVertices[22].vNormal = XMFLOAT3(0.f, -1.f, 0.f);


		pVertices[23].vPosition = XMFLOAT3(-0.5f, -0.5f, 0.5f);
		pVertices[23].vTexcoord = XMFLOAT2(0.f, 1.f);
		pVertices[23].vNormal = XMFLOAT3(0.f, -1.f, 0.f);
	}

	// Create Vertex Buffer
	{

		ZeroMemory(&m_SubResourceDesc, sizeof m_SubResourceDesc);
		m_SubResourceDesc.pSysMem = pVertices;

		ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
		m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
		m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_BufferDesc.StructureByteStride = m_iVertexStride;
		m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		m_BufferDesc.MiscFlags = 0;

		if (FAILED(__super::Create_Buffer(&m_pVB)))
			return E_FAIL;
	}

	// 정면
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 0;
	pIndices[4] = 2;
	pIndices[5] = 3;

	// 오른쪽 면
	pIndices[6] = 4;
	pIndices[7] = 5;
	pIndices[8] = 6;
	pIndices[9] = 4;
	pIndices[10] = 6;
	pIndices[11] = 7;

	// 뒷 면
	pIndices[12] = 8;
	pIndices[13] = 9;
	pIndices[14] = 10;
	pIndices[15] = 8;
	pIndices[16] = 10;
	pIndices[17] = 11;

	// 왼쪽 면
	pIndices[18] = 12;
	pIndices[19] = 13;
	pIndices[20] = 14;
	pIndices[21] = 12;
	pIndices[22] = 14;
	pIndices[23] = 15;

	// 아랫 면
	pIndices[24] = 16;
	pIndices[25] = 17;
	pIndices[26] = 18;
	pIndices[27] = 16;
	pIndices[28] = 18;
	pIndices[29] = 19;

	// 왼쪽 면
	pIndices[30] = 20;
	pIndices[31] = 21;
	pIndices[32] = 22;
	pIndices[33] = 20;
	pIndices[34] = 22;
	pIndices[35] = 23;


	// Create Index Buffer
	{
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
	}



	Safe_Delete_Array(pVertices);
	Safe_Delete_Array(pIndices);


	return S_OK;
}

HRESULT CVIBuffer_Square::Initialize(void* pArg)
{
	return S_OK;
}

CVIBuffer_Square* CVIBuffer_Square::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVIBuffer_Square* pInstance = new CVIBuffer_Square(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Square");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Square::Clone(void* pArg)
{
	CVIBuffer_Square* pInstance = new CVIBuffer_Square(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Square");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Square::Free()
{
	__super::Free();
}

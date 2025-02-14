#include "..\Public\VIBuffer_Particle_Rect.h"

#include "GameInstance.h"

CVIBuffer_Particle_Rect::CVIBuffer_Particle_Rect(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer_Particle_Instance { pDevice, pContext }
{
}

CVIBuffer_Particle_Rect::CVIBuffer_Particle_Rect(const CVIBuffer_Particle_Rect & Prototype)
	: CVIBuffer_Particle_Instance { Prototype }
{
}

HRESULT CVIBuffer_Particle_Rect::Initialize_Prototype(const PARTICLE_DESC& ParticleDesc)
{
	m_iNumVertexBuffers = 2;
	m_iNumInstances = ParticleDesc.iNumInstances;
	

	m_iVertexStride = sizeof(VTXPOSTEX);
	m_iNumVertices = 4;

	m_iIndexStride = sizeof(_ushort);
	m_iNumIndices = 6 * m_iNumInstances;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#pragma region VERTEX_BUFFER	
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼를 몇 바이트 할당할까요? */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.StructureByteStride = m_iVertexStride;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;

	VTXPOSTEX*		pVertices = new VTXPOSTEX[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXPOSTEX) * m_iNumVertices);

	pVertices[0].vPosition = XMFLOAT3(-0.5f, 0.5f, 0.f);
	pVertices[0].vTexcoord = XMFLOAT2(0.f, 0.f);

	pVertices[1].vPosition = XMFLOAT3(0.5f, 0.5f, 0.f);
	pVertices[1].vTexcoord = XMFLOAT2(1.f, 0.f);

	pVertices[2].vPosition = XMFLOAT3(0.5f, -0.5f, 0.f);
	pVertices[2].vTexcoord = XMFLOAT2(1.f, 1.f);

	pVertices[3].vPosition = XMFLOAT3(-0.5f, -0.5f, 0.f);
	pVertices[3].vTexcoord = XMFLOAT2(0.f, 1.f);

	ZeroMemory(&m_SubResourceDesc, sizeof m_SubResourceDesc);
	m_SubResourceDesc.pSysMem = pVertices;
	

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

#pragma endregion

#pragma region INDEX_BUFFER
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼를 몇 바이트 할당할까요? */
	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.StructureByteStride = /*m_iIndexStride*/0;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;

	_ushort*		pIndices = new _ushort[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

	_uint			iNumIndices = { 0 };

	for (size_t i = 0; i < m_iNumInstances; i++)
	{
		pIndices[iNumIndices++] = 0;
		pIndices[iNumIndices++] = 1;
		pIndices[iNumIndices++] = 2;

		pIndices[iNumIndices++] = 0;
		pIndices[iNumIndices++] = 2;
		pIndices[iNumIndices++] = 3;
	}

	ZeroMemory(&m_SubResourceDesc, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceDesc.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion


#pragma region INSTANCE_BUFFER
	m_iInstanceStride = sizeof(VTXINSTANCE);
	m_iNumIndexPerInstance = 6;

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼를 몇 바이트 할당할까요? */
	m_InstanceBufferDesc.ByteWidth = m_iInstanceStride * m_iNumInstances;
	m_InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_InstanceBufferDesc.StructureByteStride = m_iInstanceStride;
	m_InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_InstanceBufferDesc.MiscFlags = 0;

	m_pInstanceVertices = new VTXINSTANCE[m_iNumInstances];
	ZeroMemory(m_pInstanceVertices, m_iInstanceStride * m_iNumInstances);

	m_isLoop = ParticleDesc.isLoop;
	m_pSpeeds = new _float[m_iNumInstances];	

	m_vPivot = ParticleDesc.vPivot;	

	for (size_t i = 0; i < m_iNumInstances; i++)
	{
		m_pSpeeds[i] = m_pGameInstance->Compute_Random(ParticleDesc.vSpeed.x, ParticleDesc.vSpeed.y);
		
		_float		fSize = m_pGameInstance->Compute_Random(ParticleDesc.vSize.x, ParticleDesc.vSize.y);

		m_pInstanceVertices[i].vRight = _float4(fSize, 0.f, 0.f, 0.f);
		m_pInstanceVertices[i].vUp = _float4(0.f, fSize, 0.f, 0.f);
		m_pInstanceVertices[i].vLook = _float4(0.f, 0.f, fSize, 0.f);
		m_pInstanceVertices[i].vTranslation = 
			_float4(
				m_pGameInstance->Compute_Random(ParticleDesc.vCenter.x - ParticleDesc.vRange.x * 0.5f, ParticleDesc.vCenter.x + ParticleDesc.vRange.x * 0.5f),
				m_pGameInstance->Compute_Random(ParticleDesc.vCenter.y - ParticleDesc.vRange.y * 0.5f, ParticleDesc.vCenter.y + ParticleDesc.vRange.y * 0.5f),
				m_pGameInstance->Compute_Random(ParticleDesc.vCenter.z - ParticleDesc.vRange.z * 0.5f, ParticleDesc.vCenter.z + ParticleDesc.vRange.z * 0.5f), 
				1.f);

		m_pInstanceVertices[i].vLifeTime = _float2(
			m_pGameInstance->Compute_Random(ParticleDesc.vLifeTime.x, ParticleDesc.vLifeTime.y),
			0.f
		);
	}

	ZeroMemory(&m_InstanceInitialDesc, sizeof m_InstanceInitialDesc);
	m_InstanceInitialDesc.pSysMem = m_pInstanceVertices;	

#pragma endregion


	return S_OK;
}

HRESULT CVIBuffer_Particle_Rect::Initialize(void * pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

CVIBuffer_Particle_Rect * CVIBuffer_Particle_Rect::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const PARTICLE_DESC& ParticleDesc)
{
	CVIBuffer_Particle_Rect*	pInstance = new CVIBuffer_Particle_Rect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(ParticleDesc)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Particle_Rect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CVIBuffer_Particle_Rect::Clone(void * pArg)
{
	CVIBuffer_Particle_Rect*	pInstance = new CVIBuffer_Particle_Rect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Particle_Rect");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CVIBuffer_Particle_Rect::Free()
{
	__super::Free();

}

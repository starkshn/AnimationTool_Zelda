#include "..\Public\VIBuffer_Particle_Instance.h"

CVIBuffer_Particle_Instance::CVIBuffer_Particle_Instance(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer { pDevice, pContext }
{

}

CVIBuffer_Particle_Instance::CVIBuffer_Particle_Instance(const CVIBuffer_Particle_Instance & Prototype)
	: CVIBuffer{ Prototype }
	, m_pVBInstance { Prototype.m_pVBInstance}
	, m_InstanceBufferDesc{ Prototype.m_InstanceBufferDesc }
	, m_InstanceInitialDesc{ Prototype.m_InstanceInitialDesc }
	, m_iNumInstances{ Prototype.m_iNumInstances }
	, m_iNumIndexPerInstance{ Prototype.m_iNumIndexPerInstance }
	, m_iInstanceStride{ Prototype.m_iInstanceStride }
	, m_pInstanceVertices { Prototype.m_pInstanceVertices }
	, m_pSpeeds { Prototype.m_pSpeeds }
	, m_isLoop { Prototype.m_isLoop }
	, m_vPivot { Prototype.m_vPivot }
{
	Safe_AddRef(m_pVBInstance);
}

HRESULT CVIBuffer_Particle_Instance::Initialize_Prototype()
{


	return S_OK;
}

HRESULT CVIBuffer_Particle_Instance::Initialize(void * pArg)
{
	return m_pDevice->CreateBuffer(&m_InstanceBufferDesc, &m_InstanceInitialDesc, &m_pVBInstance);	
}

HRESULT CVIBuffer_Particle_Instance::Render()
{
	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstances, 0, 0, 0);

	return S_OK;
}

HRESULT CVIBuffer_Particle_Instance::Bind_BufferDesc()
{
	ID3D11Buffer*			pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance, 
	};

	_uint					iVertexStrides[] = {
		m_iVertexStride,
		m_iInstanceStride
	};

	_uint					iOffsets[] = {
		0,
		0
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	return S_OK;
}

void CVIBuffer_Particle_Instance::Drop(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXINSTANCE*		pVertices = static_cast<VTXINSTANCE*>(SubResource.pData);

	for (size_t i = 0; i < m_iNumInstances; i++)
	{
		pVertices[i].vTranslation.y -= m_pSpeeds[i] * fTimeDelta;

		pVertices[i].vLifeTime.y += fTimeDelta;

		if (pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
		{
			if (true == m_isLoop)
			{
				pVertices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
				pVertices[i].vLifeTime.y = 0.f;
			}
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Particle_Instance::Spread(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXINSTANCE*		pVertices = static_cast<VTXINSTANCE*>(SubResource.pData);

	for (size_t i = 0; i < m_iNumInstances; i++)
	{
		_vector		vDir = XMLoadFloat4(&pVertices[i].vTranslation) - XMVectorSetW(XMLoadFloat3(&m_vPivot), 1.f);

		XMStoreFloat4(&pVertices[i].vTranslation, XMLoadFloat4(&pVertices[i].vTranslation) + XMVector3Normalize(vDir) * m_pSpeeds[i] * fTimeDelta);

		pVertices[i].vLifeTime.y += fTimeDelta;

		if (pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
		{
			if (true == m_isLoop)
			{
				pVertices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
				pVertices[i].vLifeTime.y = 0.f;
			}
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}



void CVIBuffer_Particle_Instance::Free()
{
	__super::Free();

	if (false == m_isCloned)
	{
		
		Safe_Delete_Array(m_pSpeeds);
		Safe_Delete_Array(m_pInstanceVertices);
	}

	Safe_Release(m_pVBInstance);
}

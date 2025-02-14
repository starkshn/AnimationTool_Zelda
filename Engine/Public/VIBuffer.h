#pragma once

#include "Component.h"

/* 여러 메시(사각형, 지형, 큐브 ) 을 그려내기위한 컴포넌트의 부모 클래스 */
/* 메시를 구성하기위해서는 정점, 인덱스가필요하고 저장한다.*/

BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer(const CVIBuffer& Prototype);
	virtual ~CVIBuffer() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render();

public:
	virtual HRESULT Bind_BufferDesc();

protected:
	ID3D11Buffer*						m_pVB = { nullptr };	
	ID3D11Buffer*						m_pIB = { nullptr };

protected:
	D3D11_BUFFER_DESC					m_BufferDesc;
	D3D11_SUBRESOURCE_DATA				m_SubResourceDesc;

protected:
	_uint								m_iVertexStride = {};
	_uint								m_iNumVertices = {};
	_uint								m_iNumVertexBuffers = {};
		
	_uint								m_iIndexStride = {};
	_uint								m_iNumIndices = {};
	DXGI_FORMAT							m_eIndexFormat = {};
	D3D11_PRIMITIVE_TOPOLOGY			m_ePrimitiveTopology = {};

	_float3*						m_pVertexPositions = { nullptr };

protected:
	HRESULT Create_Buffer(ID3D11Buffer** ppOut);

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END
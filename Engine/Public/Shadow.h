#pragma once

#include "Base.h"

/* 그림자 깊이 비교용 카메라를 위한 뷰, 투영행렬을 보관한다. */

BEGIN(Engine)

class CShadow final : public CBase
{
public:
	enum D3DTRANSFORMSTATE { D3DTS_VIEW, D3DTS_PROJ, D3DTS_END };

private:
	CShadow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShadow() = default;

public:
	const _float4x4* Get_Shadow_Transform(D3DTRANSFORMSTATE eState) {
		return &m_TransformMatrices[eState];
	}

public:
	void SetUp_TransformMatrix(D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix) {
		XMStoreFloat4x4(&m_TransformMatrices[eState], TransformMatrix);
	}

public:
	HRESULT Initialize();
	void Update();

private:	
	_float4x4							m_TransformMatrices[D3DTS_END];
	_uint								m_iWidth{}, m_iHeight{};
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };		

public:
	static CShadow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END
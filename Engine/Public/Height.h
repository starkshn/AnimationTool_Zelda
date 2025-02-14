#pragma once

#include "Base.h"

/* 높이 계산용 카메라를 위한 뷰, 투영행렬을 보관한다. */

BEGIN(Engine)

class CHeight final : public CBase
{
public:
	enum D3DTRANSFORMSTATE { D3DTS_VIEW, D3DTS_PROJ, D3DTS_END };

private:
	CHeight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CHeight() = default;

public:
	const _float4x4* Get_Height_Transform(D3DTRANSFORMSTATE eState) {
		return &m_TransformMatrices[eState];
	}

public:
	void SetUp_TransformMatrix(D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix) {
		XMStoreFloat4x4(&m_TransformMatrices[eState], TransformMatrix);
	}

public:
	HRESULT Initialize();
	void Update();
	_float Compute_Height(_fvector vWorldPos);

private:	
	_float4x4							m_TransformMatrices[D3DTS_END];

	_uint								m_iWidth{}, m_iHeight{};
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };
	ID3D11Texture2D*					m_pTexture2D = { nullptr };
	class CGameInstance*				m_pGameInstance = { nullptr };

	_float*								m_pHeights = { nullptr };

public:
	static CHeight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END
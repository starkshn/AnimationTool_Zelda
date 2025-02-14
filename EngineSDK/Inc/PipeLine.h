#pragma once

#include "Base.h"

/* 메인으로 사용되는 원근투영을 위한 뷰, 투영행렬을 보관한다. */
/* 고정기능렌더링파이플아ㅣㄴ을 지원하지 않는다.(장치에 행렬을 보관한다 사라졌다.) */
/* 카메라가 뷰, 투영을 관리하긴하지만 결과물을 항상 파이프라인에 저장해줄거다. */
/* 파이프라인을 통해서 쉽고 빠르게 뷰, ㅋ투영을 얻어올 수 있도록 하겠다. */
/* Update함수를 통해서 뷰, 투영행렬의 역행려을 매프레임마다 한번씩 구해놓는다. */

BEGIN(Engine)

class CPipeLine final : public CBase
{
public:
	enum D3DTRANSFORMSTATE { D3DTS_VIEW, D3DTS_PROJ, D3DTS_END };
private:
	CPipeLine();
	virtual ~CPipeLine() = default;

public:
	void Set_Transform(D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix) {
		XMStoreFloat4x4(&m_TransformMatrices[eState], TransformMatrix);
	}

	_matrix Get_TransformMatrix(D3DTRANSFORMSTATE eState) {
		return XMLoadFloat4x4(&m_TransformMatrices[eState]);
	}

	_float4x4 Get_TransformFloat4x4(D3DTRANSFORMSTATE eState) {
		return m_TransformMatrices[eState];
	}

	_float4x4 Get_TransformFloat4x4_Inverse(D3DTRANSFORMSTATE eState) {
		return m_TransformInverseMatrices[eState];
	}

	_matrix Get_TransformMatrix_Inverse(D3DTRANSFORMSTATE eState) {
		return XMLoadFloat4x4(&m_TransformInverseMatrices[eState]);
	}

	const _float4* Get_CamPosition() const {
		return &m_vCamPosition;
	}

	string ConvertToString(wstring strName) {
		string ConvertName(strName.begin(), strName.end());
		return ConvertName;
	}

	wstring ConvertToWString(string strName) {
		wstring ConvertName(strName.begin(), strName.end());
		return ConvertName;
	}

public:
	HRESULT Initialize();
	void Update();

private:
	_float4x4				m_TransformMatrices[D3DTS_END];
	_float4x4				m_TransformInverseMatrices[D3DTS_END];

	_float4					m_vCamPosition = {};

public:
	static CPipeLine* Create();
	virtual void Free() override;
};

END
#pragma once

#include "Base.h"

BEGIN(Engine)

class CFrustum final : public CBase
{
private:
	CFrustum();
	virtual ~CFrustum() = default;

public:
	HRESULT Initialize();
	void Update();	/* 매 프레임 ㅎ호출하여 공통영역인 월드까지 변환 */

	void Transform_ToLocalSpace(_fmatrix WorldMatrix); /* 특정 오브젝트의 로컬로 변환하기위해 필요에 따라 호출되는 함수. */

	_bool isIn_InWorldSpace(_fvector vWorldPos, _float fRange = 0.f);
	_bool isIn_InLocalSpace(_fvector vLocalPos, _float fRange = 0.f);

private:
	class CGameInstance*				m_pGameInstance = { nullptr };

	_float3								m_vPoints[8] = {};
	_float3								m_vPoints_InWorld[8] = {};

	_float4								m_vPlanes_InWorld[6] = {};
	_float4								m_vPlanes_InLocal[6] = {};

private:
	void Make_Planes(const _float3* pPoints, _Out_ _float4* pPlanes);

public:
	static CFrustum* Create();
	virtual void Free() override;
};

END
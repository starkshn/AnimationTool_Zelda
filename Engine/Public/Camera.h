#pragma once

#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CCamera abstract : public CGameObject
{
public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		_float3			vEye = {};
		_float3			vAt = {};

		_float			fFovy = { 0.f };
		_float			fAspect = { 0.f };
		_float			fNear = { 0.f };
		_float			fFar = { 0.f };		
	}CAMERA_DESC;

protected:
	CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera(const CCamera& Prototype);
	virtual ~CCamera() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	/* 마늗ㄹ어낸 뷰, 투영행를은 파이플아니에 저장ㅇㅇ으 ㄹ 하겠디 .*/
	/* 뷰 투영ㅇ행렬을 만들엉ㄴ내기위한 데이터들을 보관해서 각행렬을 만들는 기능은 수행을해주자. */
	/* 뷰 -> Transform대체하과 있다. */
	/* 투영 -> x */

	_float			m_fFovy = { 0.f };
	_float			m_fAspect = { 0.f };
	_float			m_fNear = { 0.f };
	_float			m_fFar = { 0.f };

protected:
	void Compute_PipeLineMatrices();

	
public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END
#pragma once

#include "Client_Defines.h"
#include "Camera.h"

BEGIN(Client)

class CAnimTool_Camera final : public CCamera
{

public:
	typedef struct : public CCamera::CAMERA_DESC
	{
		_float			fMouseSensor = {};
	} CAMERA_FREE_DESC;

private:
	CAnimTool_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimTool_Camera(const CAnimTool_Camera& Prototype);
	virtual ~CAnimTool_Camera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float			m_fMouseSensor = {};
	_bool			m_bBlockCamMove = 1;


public:
	static CAnimTool_Camera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END
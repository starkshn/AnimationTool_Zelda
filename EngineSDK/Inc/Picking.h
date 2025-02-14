#pragma once

#include "Base.h"

BEGIN(Engine)

class CPicking final : public CBase
{
private:
	CPicking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPicking() = default;

public:
	HRESULT Initialize(HWND hWnd);
	void Update();

public:
	_bool isPicked(_Inout_ _float4* pWorldPos);

private:
	HWND								m_hWnd = {};
	_uint								m_iWidth{}, m_iHeight{};
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };
	ID3D11Texture2D*					m_pTexture2D = { nullptr };
	class CGameInstance*				m_pGameInstance = { nullptr };

public:
	static CPicking* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd);
	virtual void Free() override;
};

END
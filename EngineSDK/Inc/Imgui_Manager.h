#pragma once
#include "Base.h"
#include "Engine_Defines.h"

BEGIN(Engine)

class CImgui_Manager : public CBase
{
private:
	CImgui_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CImgui_Manager() = default;

public:
	HRESULT		Initialize(HWND _hWnd);

public:
	void Update();
	void Render();

private:
	ID3D11Device*				m_pDevice = {nullptr};
	ID3D11DeviceContext*		m_pContext = {nullptr};

public:
	static CImgui_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND _hWnd);
	virtual void Free() override;
};

END
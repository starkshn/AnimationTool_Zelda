#pragma once

#include "Client_Defines.h"
#include "Base.h"

BEGIN(Engine)
class CGameInstance;
END

BEGIN(Client)

class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

private:
	CGameInstance*				m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };


#ifdef _DEBUG
private:
	_float						m_fTimeAcc = {};
	_tchar						m_szFPS[MAX_PATH] = {};
	_uint						m_iNumDraw = {};

#endif

private:	
	HRESULT SetUp_StartLevel(LEVELID eLevelID);
	HRESULT Ready_Gara();
	HRESULT Ready_Prototype_Component_For_Static();

public:
	static CMainApp* Create();
	virtual void Free() override;
};

END
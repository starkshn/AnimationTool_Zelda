#pragma once

#include "Client_Defines.h"
#include "Level.h"

/* 로딩화면을 구성하기위한 객체(로딩화면의 배경, 로딩 바, 로딩 폰트)를 만들어낸다. */
/* 다음 레벨에 필요한 자원을 로드하는 역활을 하는 객체를 생성해준다.  */

BEGIN(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(LEVELID eNextLevelID);
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	LEVELID			m_eNextLevelID = { LEVEL_END };
	class CLoader*	m_pLoader = { nullptr };

public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVELID eNextLevelID);
	virtual void Free() override;
};

END
#pragma once
#include "Base.h"

BEGIN(Engine)

class CBasic_Tool final : public CBase 
{
private:
	CBasic_Tool();
	virtual ~CBasic_Tool() = default;

public:
	HRESULT Initialize();
	void	Update(_float fTimeDelta);
	void	Select_GameObject(class CGameObject* pObj);
	void	Reset_Select_GameObject() { m_pSelectedObj = nullptr; }

private:
	void	Hierarchy();
	void	Inspector();
	void	Project();

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	class CGameObject*		m_pSelectedObj = { nullptr };

public:
	static CBasic_Tool* Create();
	virtual void Free();
};

END

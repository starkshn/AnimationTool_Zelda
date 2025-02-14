#pragma once

#include "Base.h"

/* 객체들을 레이어 별로 묶어서 다수 보관한다. */

BEGIN(Engine)

class CLayer final : public CBase
{
private:
	CLayer();
	virtual ~CLayer() = default;

public:
	//class CComponent* Get_Component(const _wstring& strComponentTag, _uint iObjectIndex = 0);
	//class CComponent* Get_Component(_uint iPartObjectIndex, const _wstring& strPartComponentTag, _uint iObjectIndex = 0);

public:
	HRESULT Add_GameObject(class CGameObject* pGameObject, const _wstring& strObjTag = L"GameObject");
	void Enable_Update();
	void Priority_Update(_float fTimeDelta);
	void Update(_float fTimeDelta);
	void Late_Update(_float fTimeDelta);
	void Disable_Update();


	//게임아이디로 레이어에서 오브젝트 탐색
	class CGameObject* Find_Object_By_ID(_uint iID);
	class CGameObject* Find_Object_By_Name(const _wstring& strName);
	void	ReName(const _wstring& strName);


	_bool Remove_Object(const _wstring& strObjTag);

#ifdef _DEBUG
	void	Hierarchy();
#endif 


private:
	//list<class CGameObject*>			m_GameObjects;
	map<const _wstring, class CGameObject*> m_GameObjects;

public:
	static CLayer* Create();
	virtual void Free() override;
};

END
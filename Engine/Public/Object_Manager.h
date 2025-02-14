#pragma once

/* 레이어 : 내가 나누고 싶은 기준으로 나눠서 보관한 그룹. */
/* 레이어들을 레벨별로 보관한다. */
#include "Base.h"

BEGIN(Engine)

class CObject_Manager final : public CBase
{
private:
	CObject_Manager();
	virtual ~CObject_Manager() = default;

public:
	//class CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponentTag, _uint iObjectIndex = 0);
	//class CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, _uint iPartObjectIndex, const _wstring& strPartComponentTag, _uint iObjectIndex = 0);


public:
	HRESULT Initialize(_uint iNumLevels);
	HRESULT Add_GameObject_ToLayer(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, 
		_uint iLevelIndex, const _wstring& strLayerTag, void* pArg = nullptr, const _wstring& strObjTag = L"GameObject");
	void Enable_Update();
	void Priority_Update(_float fTimeDelta);
	void Update(_float fTimeDelta);
	void Late_Update(_float fTimeDelta);
	void Clear(_uint iLevelIndex);
	void Disable_Update();

public:
	//게임아이디로 레이어에서 오브젝트 탐색
	class CGameObject* Find_Object_By_ID(_uint iLevelIndex, const _wstring& strLayerTag, _uint iID);
	class CGameObject* Find_Object_By_Name(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strName);

	HRESULT Remove_Object(_uint iCurLevelIdx, const _wstring& strObjTag);


#ifdef _DEBUG
public:
	void	Hierarchy();
#endif // _DEBUG

private:
	_uint										m_iNumLevels = { 0 };
	map<const _wstring, class CLayer*>*			m_pLayers = { nullptr };
	typedef map<const _wstring, class CLayer*>	LAYERS;

	class CGameInstance*						m_pGameInstance = { nullptr };

private:
	class CLayer* Find_Layer(_uint iLevelIndex, const _wstring& strLayerTag);



public:
	static CObject_Manager* Create(_uint iNumLevels);
	virtual void Free() override;
};

END
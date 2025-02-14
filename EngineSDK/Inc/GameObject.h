#pragma once

#include "Transform.h"

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct : public CTransform::TRANSFORM_DESC
	{
		const _float4x4* pParentMatrix = {};
		
	}GAMEOBJECT_DESC;
protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& Prototype);
	virtual ~CGameObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_Height() { return S_OK; }
	virtual HRESULT Render_Shadow() { return S_OK; }

	virtual void	Enable() {};
	virtual void	Disalbe() {};

	void	Set_Active(_bool isActive) { m_isActive = isActive; }
	_bool	Get_Active() { return m_isActive; }
	const _wstring& Get_Name() { return m_strName; }
	void	ReName(const _wstring& strName) { m_strName = strName; }

	void	Add_Child(class CGameObject* pChild);
#ifdef _DEBUG
public:
	virtual void Hierarchy();
	virtual void Insepector();
#endif

public: 
	class CComponent*			Find_Component(const _wstring& strComponentTag);
	//트랜스폼 컴포넌트는 자주 사용해서 Get함수 만들었음
	class CTransform*			Get_Transform() { return m_pTransformCom; }

	//이건 오브젝트가 활성 비활성화 됬을 때 Enable/Disable부르기위한 판단을 위해 존재(건들필요 없음)
	_bool						Check_PrevState(ABLE_STATE eAbleState);

	_bool						Check_ID(_uint iID) { 
		_bool isResult{}; (m_iGameID == iID) ? isResult = true : isResult = false;
		return isResult;
	}


	class CGameObject* Find_Child(_uint iChildIndex);
	class CGameObject* Find_Child(const _wstring& strName);
	class CGameObject* Find_Child(class CGameObject* pChild);

	_int Get_ChildSize() { return m_vecChildren.size(); }

	class CGameObject* Get_Parent() { return m_pParent; } // NY
	
protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };
	class CTransform*			m_pTransformCom = { nullptr };

protected:
	map<const _wstring, class CComponent*>		m_Components;

public:
	vector<CGameObject*>						m_vecChildren;

protected:
	HRESULT Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);

protected:
	_bool						m_isActive = { true };

	_int						m_iGameID = {};

	_wstring					m_strName = L"";
	const _float4x4*			m_pParentMatrix = {};
	_float4x4					m_WorldMatrix = {};
	class CGameObject*			m_pParent = { nullptr };

private:
	_bool						m_isPrevActive = { false };
	static _int					g_iNextGameID;

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;

};

END
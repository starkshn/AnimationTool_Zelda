#include "..\Public\Layer.h"
#include "GameObject.h"

CLayer::CLayer()
{
}

//CComponent * CLayer::Get_Component(const _wstring & strComponentTag, _uint iObjectIndex)
//{
//	if (iObjectIndex >= m_GameObjects.size())
//		return nullptr;
//
//	auto	iter = m_GameObjects.begin();
//
//	for (size_t i = 0; i < iObjectIndex; i++)
//		++iter;
//
//	return (*iter)->Find_Component(strComponentTag);	
//}

//CComponent * CLayer::Get_Component(_uint iPartObjectIndex, const _wstring & strPartComponentTag, _uint iObjectIndex)
//{
//	if (iObjectIndex >= m_GameObjects.size())
//		return nullptr;
//
//	auto	iter = m_GameObjects.begin();
//
//	for (size_t i = 0; i < iObjectIndex; i++)
//		++iter;
//
//	return static_cast<CContainerObj*>(*iter)->Find_Component(iPartObjectIndex, strPartComponentTag);
//}

HRESULT CLayer::Add_GameObject(CGameObject * pGameObject, const _wstring& strObjTag)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	auto& iter = m_GameObjects.find(strObjTag);

	if (iter != m_GameObjects.end())
	{
		if (strObjTag == L"GameObject")
		{
			_uint iNumbering = { 1 };
			_wstring strTag = L"";
			do
			{
				strTag = strObjTag + L"(" + to_wstring(iNumbering++) + L")";
				iter = m_GameObjects.find(strTag);
			} while (iter != m_GameObjects.end());

			m_GameObjects.emplace(strTag, pGameObject);
			pGameObject->ReName(strTag);
		}
		else
		{
			wstring strMSG = strObjTag;

			strMSG = strMSG + L" : already Exists";
			MessageBox(NULL, strMSG.c_str(), L"System Message", MB_OK);

			return E_FAIL;
		}
	}
	else
	{
		m_GameObjects.emplace(strObjTag, pGameObject);
		pGameObject->ReName(strObjTag);
	}

	return S_OK;
}

void CLayer::Enable_Update()
{
	for (auto& pGameObject : m_GameObjects)
	{
		if(pGameObject.second->Check_PrevState(ENABLE))
			pGameObject.second->Enable();
	}
}

void CLayer::Priority_Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
		pGameObject.second->Priority_Update(fTimeDelta);
}

void CLayer::Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
		pGameObject.second->Update(fTimeDelta);
}

void CLayer::Late_Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
		pGameObject.second->Late_Update(fTimeDelta);
}

void CLayer::Disable_Update()
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (pGameObject.second->Check_PrevState(DISABLE))
			pGameObject.second->Disalbe();
	}
}

CGameObject* CLayer::Find_Object_By_ID(_uint iID)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (pGameObject.second->Check_ID(iID))
			return pGameObject.second;
	}

	return nullptr;
}

CGameObject* CLayer::Find_Object_By_Name(const _wstring& strName)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (pGameObject.second->Get_Name() == strName)
			return pGameObject.second;
	}

	return nullptr;
}



_bool CLayer::Remove_Object(const _wstring& strObjTag)
{
	auto iter = find_if(m_GameObjects.begin(), m_GameObjects.end(), [&](const std::pair<const _wstring, CGameObject*>& pair)
		{
			return pair.second->Get_Name() == strObjTag;
		}
	);
	
	// 찾았다면 
	if (iter != m_GameObjects.end())
	{
		Safe_Release((*iter).second);
		m_GameObjects.erase((*iter).first);
		return true;
	}
	return false;
}

void CLayer::Hierarchy()
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (pGameObject.second != nullptr)
			pGameObject.second->Hierarchy();
	}
}

CLayer * CLayer::Create()
{
	 return new CLayer();
}


void CLayer::Free()
{
	__super::Free();

	for (auto& pGameObject : m_GameObjects)
		Safe_Release(pGameObject.second);

	m_GameObjects.clear();
}

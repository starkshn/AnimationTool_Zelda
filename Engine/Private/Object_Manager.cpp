#include "..\Public\Object_Manager.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "Layer.h"

CObject_Manager::CObject_Manager()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

//CComponent * CObject_Manager::Get_Component(_uint iLevelIndex, const _wstring & strLayerTag, const _wstring & strComponentTag, _uint iObjectIndex)
//{
//	CLayer*		pLayer = Find_Layer(iLevelIndex, strLayerTag);
//	if (nullptr == pLayer)
//		return nullptr;
//
//	return pLayer->Get_Component(strComponentTag, iObjectIndex);
//}
//
//CComponent * CObject_Manager::Get_Component(_uint iLevelIndex, const _wstring & strLayerTag, _uint iPartObjectIndex, const _wstring & strPartComponentTag, _uint iObjectIndex)
//{
//	/* 컨테이너 오브젝트에 해당하는 객체가 포함되어있는 레이어를 먼저 찾는다. */
//	CLayer*		pLayer = Find_Layer(iLevelIndex, strLayerTag);
//	if (nullptr == pLayer)
//		return nullptr;
//
//	return pLayer->Get_Component(iPartObjectIndex, strPartComponentTag, iObjectIndex);	
//}

HRESULT CObject_Manager::Initialize(_uint iNumLevels)
{
	if (nullptr != m_pLayers)
		return E_FAIL;

	m_pLayers = new LAYERS[iNumLevels];

	m_iNumLevels = iNumLevels;

	return S_OK;
}

HRESULT CObject_Manager::Add_GameObject_ToLayer(_uint iPrototypeLevelIndex, const _wstring & strPrototypeTag, _uint iLevelIndex,
	const _wstring & strLayerTag, void * pArg, const _wstring& strObjTag)
{
	CGameObject*		pGameObject = dynamic_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_GAMEOBJ, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pGameObject)
		return E_FAIL;

	CLayer*			pLayer = Find_Layer(iLevelIndex, strLayerTag);

	if (nullptr == pLayer)
	{
		pLayer = CLayer::Create();

		pLayer->Add_GameObject(pGameObject, strObjTag);

		m_pLayers[iLevelIndex].emplace(strLayerTag, pLayer);
	}
	else
		pLayer->Add_GameObject(pGameObject, strObjTag);

	return S_OK;
}

void CObject_Manager::Enable_Update()
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Pair.second->Enable_Update();
	}
}

void CObject_Manager::Priority_Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])		
			Pair.second->Priority_Update(fTimeDelta);		
	}
}

void CObject_Manager::Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Pair.second->Update(fTimeDelta);
	}
}

void CObject_Manager::Late_Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Pair.second->Late_Update(fTimeDelta);
	}
}

void CObject_Manager::Clear(_uint iLevelIndex)
{
	if (iLevelIndex >= m_iNumLevels)
		return;

	for (auto& Pair : m_pLayers[iLevelIndex])
		Safe_Release(Pair.second);

	m_pLayers[iLevelIndex].clear();
}

void CObject_Manager::Disable_Update()
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Pair.second->Disable_Update();
	}
}

CGameObject* CObject_Manager::Find_Object_By_ID(_uint iLevelIndex, const _wstring& strLayerTag, _uint iID)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	auto	iter = m_pLayers[iLevelIndex].find(strLayerTag);
	if (iter == m_pLayers[iLevelIndex].end())
		return nullptr;

	return iter->second->Find_Object_By_ID(iID);
}

CGameObject* CObject_Manager::Find_Object_By_Name(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strName)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	auto	iter = m_pLayers[iLevelIndex].find(strLayerTag);
	if (iter == m_pLayers[iLevelIndex].end())
		return nullptr;

	return iter->second->Find_Object_By_Name(strName);
}

HRESULT CObject_Manager::Remove_Object(_uint iCurLevelIdx, const _wstring& strObjTag)
{
	for (auto Pair : m_pLayers[iCurLevelIdx])
	{
		if (Pair.second->Remove_Object(strObjTag))
			return S_OK;
	}

	return E_FAIL;
}

void CObject_Manager::Hierarchy()
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
		{
			if (ImGui::TreeNode(CONVERTTOSTRING(Pair.first).c_str()))
			{
				Pair.second->Hierarchy();

				ImGui::TreePop();
			}
		}
		
	}
}

CLayer * CObject_Manager::Find_Layer(_uint iLevelIndex, const _wstring & strLayerTag)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	auto	iter = m_pLayers[iLevelIndex].find(strLayerTag);
	if (iter == m_pLayers[iLevelIndex].end())		
		return nullptr;

	return iter->second;

}

CObject_Manager * CObject_Manager::Create(_uint iNumLevels)
{
	CObject_Manager*		pInstance = new CObject_Manager();

	if (FAILED(pInstance->Initialize(iNumLevels)))
	{
		MSG_BOX("Failed to Created : CObject_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObject_Manager::Free()
{
	__super::Free();

	for (_uint i = 0; i < m_iNumLevels; ++i)
	{
		for (auto& Pair : m_pLayers[i])
			Safe_Release(Pair.second);

		m_pLayers[i].clear();
	}

	Safe_Delete_Array(m_pLayers);

	Safe_Release(m_pGameInstance);
}


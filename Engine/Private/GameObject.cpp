#include "..\Public\GameObject.h"
#include "GameInstance.h"
#include "Transform.h"

_int	CGameObject::g_iNextGameID = -1;

CGameObject::CGameObject(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance() }
	, m_iGameID{ g_iNextGameID++ }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CGameObject::CGameObject(const CGameObject & Prototype) 
	: m_pDevice{ Prototype.m_pDevice }
	, m_pContext{ Prototype.m_pContext }
	, m_pGameInstance{ Prototype.m_pGameInstance }
	, m_iGameID{ g_iNextGameID++ }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

/* 게임내에서 직접 사용되기 위한 사본 객체가 생성되낟. */
HRESULT CGameObject::Initialize(void * pArg)
{
	if (nullptr != pArg)
	{
		GAMEOBJECT_DESC*		pDesc = static_cast<GAMEOBJECT_DESC*>(pArg);
		m_pParentMatrix = pDesc->pParentMatrix;
	}	

	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CGameObject::Priority_Update(_float fTimeDelta)
{
	for (CGameObject* pChild : m_vecChildren)
		if (pChild != nullptr && pChild->m_isActive)
			pChild->Priority_Update(fTimeDelta);

}

void CGameObject::Update(_float fTimeDelta)
{
	if (m_pParent != nullptr)
		XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));
	else
		XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix());

	for (CGameObject* pChild : m_vecChildren)
		if (pChild != nullptr && pChild->m_isActive)
			pChild->Update(fTimeDelta);
}

void CGameObject::Late_Update(_float fTimeDelta)
{
	for (CGameObject* pChild : m_vecChildren)
		if (pChild != nullptr && pChild->m_isActive)
			pChild->Late_Update(fTimeDelta);
}

HRESULT CGameObject::Render()
{
	return S_OK;
}

void CGameObject::Add_Child(CGameObject* pChild)
{
	if (!wcscmp(pChild->Get_Name().c_str(), L""))
	{
		wstring strName;
		strName = TEXT("Child_") + to_wstring(m_vecChildren.size());
		pChild->ReName(strName);
		
	}
	pChild->m_pParent = this;
	m_vecChildren.push_back(pChild);
}

#ifdef _DEBUG


void CGameObject::Hierarchy()
{
	//string strName(m_strName.begin(), m_strName.end());
	
	if (ImGui::TreeNode(CONVERTTOSTRING(m_strName).c_str()))
	{
		ImGui::SameLine();
		if (ImGui::Button("Active Inspector"))
		{
			m_pGameInstance->Select_GameObject(this);
		}
		_uint iCount{};
		for (CGameObject* pChild : m_vecChildren)
			if (pChild != nullptr)
				pChild->Hierarchy();
				
		ImGui::TreePop();
	}
}

void CGameObject::Insepector()
{
	_float3 vFloat{};
	string strName(m_strName.begin(), m_strName.end());
	ImGui::Text(strName.c_str());

	ImGui::InputFloat3("Test", &vFloat.x);
}
#endif 



CComponent * CGameObject::Find_Component(const _wstring & strComponentTag)
{
	auto	iter = m_Components.find(strComponentTag);

	if(iter == m_Components.end())
		return nullptr;

	return iter->second;
}

_bool CGameObject::Check_PrevState(ABLE_STATE eAbleState)
{
	switch (eAbleState)
	{
	case Engine::ENABLE:
		if (m_isActive)
		{
			for (CGameObject* pChild : m_vecChildren)
				if (pChild != nullptr && pChild->m_isActive)
				{
					if (pChild->Check_PrevState(ENABLE))
					{
						pChild->Enable();
					}
				}

			if (m_isActive != m_isPrevActive)
			{
				m_isPrevActive = m_isActive;
				return true;
			}
			else 
				return false;
		}
		break;
	case Engine::DISABLE:
		if (!m_isActive)
		{
			for (CGameObject* pChild : m_vecChildren)
				if (pChild != nullptr && pChild->m_isActive)
				{
					if (pChild->Check_PrevState(DISABLE))
					{
						pChild->Disalbe();
					}
				}
			if (m_isActive != m_isPrevActive)
			{
				m_isPrevActive = m_isActive;
				return true;
			}
			else
				return false;
		}
		break;
	}

	return false;
}

CGameObject* CGameObject::Find_Child(_uint iChildIndex)
{
	if (iChildIndex >= m_vecChildren.size())
		return nullptr;

	return m_vecChildren[iChildIndex];
}

CGameObject* CGameObject::Find_Child(const _wstring& strName)
{
	for (_uint i = 0; i < m_vecChildren.size(); ++i)
		if (m_vecChildren[i]->Get_Name() == strName)
			return m_vecChildren[i];

	return nullptr;
}

CGameObject* CGameObject::Find_Child(CGameObject* pChild)
{
	for (_uint i = 0; i < m_vecChildren.size(); ++i)
		if (m_vecChildren[i] == pChild)
			return m_vecChildren[i];

	return nullptr;
}

HRESULT CGameObject::Add_Component(_uint iPrototypeLevelIndex, const _wstring & strPrototypeTag, const _wstring & strComponentTag, CComponent** ppOut, void * pArg)
{
	if (nullptr != Find_Component(strComponentTag))
		return E_FAIL;

	CComponent*		pComponent = static_cast<CComponent*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::PROTO_COMPONENT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pComponent)
		return E_FAIL;

	pComponent->Set_Owner(this);

	m_Components.emplace(strComponentTag, pComponent);

	*ppOut = pComponent;

	Safe_AddRef(pComponent);

	return S_OK;
}

void CGameObject::Free()
{
	__super::Free();

	for (auto& Pair : m_Components)
		Safe_Release(Pair.second);
	m_Components.clear();

	for(CGameObject* pChild : m_vecChildren)
		Safe_Release(pChild);

	m_vecChildren.clear();

	Safe_Release(m_pTransformCom);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}

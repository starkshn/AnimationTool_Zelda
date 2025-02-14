#include "Basic_Tool.h"
#include "GameInstance.h"
#include "Layer.h"
#include "GameObject.h"

CBasic_Tool::CBasic_Tool()
	:m_pGameInstance{CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CBasic_Tool::Initialize()
{
    return S_OK;
}

void CBasic_Tool::Update(_float fTimeDelta)
{
	// Hierarchy();
	// Inspector();
	// Project();
}

void CBasic_Tool::Select_GameObject(CGameObject* pObj)
{
	m_pSelectedObj = pObj;
}

void CBasic_Tool::Hierarchy()
{
	ImGui::Begin("Hierarchy");

	m_pGameInstance->Object_Hierarchy();

	ImGui::End();
}

void CBasic_Tool::Inspector()
{
	ImGui::Begin("Inspector");

	if (m_pSelectedObj != nullptr)
	{
		m_pSelectedObj->Insepector();
	}

	ImGui::End();
}

void CBasic_Tool::Project()
{
	ImGui::Begin("Project");


	ImGui::End();
}

CBasic_Tool* CBasic_Tool::Create()
{
	CBasic_Tool* pInstance = new CBasic_Tool();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CBasic_Tool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBasic_Tool::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
}

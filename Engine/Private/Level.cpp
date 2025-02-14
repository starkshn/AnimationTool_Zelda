#include "..\Public\Level.h"

#include "GameInstance.h"

CLevel::CLevel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}

HRESULT CLevel::Initialize()
{
	return S_OK;
}

void CLevel::Update(_float fTimeDelta)
{


}

HRESULT CLevel::Render()
{
#ifdef _DEBUG

#endif 
	return S_OK;
}

void CLevel::Free()
{
#ifdef _DEBUG
	m_pGameInstance->Reset_Select_GameObject();

#endif

	__super::Free();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}

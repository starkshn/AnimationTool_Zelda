#include "..\Public\Shadow.h"



/* 16384 */

CShadow::CShadow(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }	
{

	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CShadow::Initialize()
{	
	
	return S_OK;
}

void CShadow::Update()
{
	

}

CShadow * CShadow::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CShadow*		pInstance = new CShadow(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CShadow");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CShadow::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}

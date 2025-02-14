#include "..\Public\UIObject.h"

CUIObject::CUIObject(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject { pDevice, pContext }
{
}

CUIObject::CUIObject(const CUIObject & Prototype)
	: CGameObject{ Prototype }
{
}

HRESULT CUIObject::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CUIObject::Initialize(void * pArg)
{
	if (nullptr == pArg)
		return E_FAIL;
	
	UIOBJECT_DESC*	pDesc = static_cast<UIOBJECT_DESC*>(pArg);

	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;
	m_fX = pDesc->fX;
	m_fY = pDesc->fY;	

	_uint		iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);


	/* 직교튀영을 위한 뷰ㅡ, 투영행르을 만들었다. */
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.f, 1.f));

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	/* 던져준 fX, fY,  fSizeX, fSizeY로 그려질 수 있도록 월드행렬의 상태를 제어해둔다. */
	m_pTransformCom->Scaling(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fX - ViewportDesc.Width * 0.5f, -m_fY + ViewportDesc.Height * 0.5f, 0.f, 1.f));


	return S_OK;
}



void CUIObject::Free()
{
	__super::Free();
}

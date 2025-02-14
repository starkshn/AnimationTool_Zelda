#include "..\Public\Frustum.h"

#include "GameInstance.h"

CFrustum::CFrustum()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);

}

HRESULT CFrustum::Initialize()
{
	/* 절두체를 구성하기위한 여덟개점을 만들자. */
	/* 투영스페이스 공간에 존재한다. */
	m_vPoints[0] = _float3(-1.f, 1.f, 0.f);
	m_vPoints[1] = _float3(1.f, 1.f, 0.f);
	m_vPoints[2] = _float3(1.f, -1.f, 0.f);
	m_vPoints[3] = _float3(-1.f, -1.f, 0.f);

	m_vPoints[4] = _float3(-1.f, 1.f, 1.f);
	m_vPoints[5] = _float3(1.f, 1.f, 1.f);
	m_vPoints[6] = _float3(1.f, -1.f, 1.f);
	m_vPoints[7] = _float3(-1.f, -1.f, 1.f);

	return S_OK;
}

void CFrustum::Update()
{
	/* 여덟개 점을 월드까지 변환시킨 후, 평면을 구성한다. */
	/* 공통의 영역의 최소 단계(월드) */

	_matrix		ViewMatrixInverse = m_pGameInstance->Get_TransformMatrix_Inverse(CPipeLine::D3DTS_VIEW);
	_matrix		ProjMatrixInverse = m_pGameInstance->Get_TransformMatrix_Inverse(CPipeLine::D3DTS_PROJ);
	
	for (size_t i = 0; i < 8; i++)
	{
		/* 뷰스페이스까지 이동. */
		XMStoreFloat3(&m_vPoints_InWorld[i], XMVector3TransformCoord(XMLoadFloat3(&m_vPoints[i]), ProjMatrixInverse));
		/* 월드스페이스까지 이동. */
		XMStoreFloat3(&m_vPoints_InWorld[i], XMVector3TransformCoord(XMLoadFloat3(&m_vPoints_InWorld[i]), ViewMatrixInverse));			
	}

	Make_Planes(m_vPoints_InWorld, m_vPlanes_InWorld);
}

void CFrustum::Transform_ToLocalSpace(_fmatrix WorldMatrix)
{
	_matrix WorldMatrixInverse = XMMatrixInverse(nullptr, WorldMatrix);

	_float3	vLocalPoints[8] = {};

	for (size_t i = 0; i < 8; i++)	
		XMStoreFloat3(&vLocalPoints[i], XMVector3TransformCoord(XMLoadFloat3(&m_vPoints_InWorld[i]), WorldMatrixInverse));

	Make_Planes(vLocalPoints, m_vPlanes_InLocal);
}

_bool CFrustum::isIn_InWorldSpace(_fvector vWorldPos, _float fRange)
{
	for (size_t i = 0; i < 6; i++)
	{
		/*	a b c d
			x y z 1	*/
		/* ax + by + cz + d = ? */
		if (fRange < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vPlanes_InWorld[i]), vWorldPos)))
			return false;
	}

	return true;
}

_bool CFrustum::isIn_InLocalSpace(_fvector vLocalPos, _float fRange)
{
	for (size_t i = 0; i < 6; i++)
	{	
		if (fRange < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vPlanes_InLocal[i]), vLocalPos)))
			return false;
	}

	return true;
}

void CFrustum::Make_Planes(const _float3 * pPoints, _float4 * pPlanes)
{
	/* +x */
	XMStoreFloat4(&pPlanes[0], XMPlaneFromPoints(
		XMLoadFloat3(&pPoints[1]),
		XMLoadFloat3(&pPoints[5]),
		XMLoadFloat3(&pPoints[6])));
	/* -x */
	XMStoreFloat4(&pPlanes[1], XMPlaneFromPoints(
		XMLoadFloat3(&pPoints[4]),
		XMLoadFloat3(&pPoints[0]),
		XMLoadFloat3(&pPoints[3])));

	/* +y */
	XMStoreFloat4(&pPlanes[2], XMPlaneFromPoints(
		XMLoadFloat3(&pPoints[4]),
		XMLoadFloat3(&pPoints[5]),
		XMLoadFloat3(&pPoints[1])));
	/* -y */
	XMStoreFloat4(&pPlanes[3], XMPlaneFromPoints(
		XMLoadFloat3(&pPoints[3]),
		XMLoadFloat3(&pPoints[2]),
		XMLoadFloat3(&pPoints[6])));

	/* +z */
	XMStoreFloat4(&pPlanes[4], XMPlaneFromPoints(
		XMLoadFloat3(&pPoints[5]),
		XMLoadFloat3(&pPoints[4]),
		XMLoadFloat3(&pPoints[7])));
	/* -z */
	XMStoreFloat4(&pPlanes[5], XMPlaneFromPoints(
		XMLoadFloat3(&pPoints[0]),
		XMLoadFloat3(&pPoints[1]),
		XMLoadFloat3(&pPoints[2])));
}

CFrustum * CFrustum::Create()
{
	CFrustum*		pInstance = new CFrustum();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CFrustum");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFrustum::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}

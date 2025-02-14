#include "..\Public\Bounding_AABB.h"

#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"

CBounding_AABB::CBounding_AABB(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CBounding { pDevice, pContext }
{
}

HRESULT CBounding_AABB::Initialize(const BOUND_DESC* pBoundDesc)
{
	const BOUND_AABB_DESC*		pDesc = static_cast<const BOUND_AABB_DESC*>(pBoundDesc);

	m_pOriginalBoundDesc = new BoundingBox(pDesc->vCenter, pDesc->vExtents);
	m_pBoundDesc = new BoundingBox(*m_pOriginalBoundDesc);

	return S_OK;
}

void CBounding_AABB::Update(_fmatrix WorldMatrix)
{
	_matrix			TransformMatrix = WorldMatrix;

	TransformMatrix.r[0] = XMVectorSet(1.f, 0.f, 0.f, 0.f) * XMVector3Length(TransformMatrix.r[0]);
	TransformMatrix.r[1] = XMVectorSet(0.f, 1.f, 0.f, 0.f) * XMVector3Length(TransformMatrix.r[1]);
	TransformMatrix.r[2] = XMVectorSet(0.f, 0.f, 1.f, 0.f) * XMVector3Length(TransformMatrix.r[2]);

	m_pOriginalBoundDesc->Transform(*m_pBoundDesc, TransformMatrix);
}

void CBounding_AABB::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
	DX::Draw(pBatch, *m_pBoundDesc, m_isColl == true ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));
}

_bool CBounding_AABB::Intersect(CCollider::COLLIDERTYPE eType, CBounding * pTargetBounding)
{
	m_isColl = false;

	switch (eType)
	{
	case CCollider::TYPE_AABB:
		// m_isColl = m_pBoundDesc->Intersects(*static_cast<CBounding_AABB*>(pTargetBounding)->Get_Desc());		
		m_isColl = Intersect_to_AABB(static_cast<CBounding_AABB*>(pTargetBounding));
		break;

	case CCollider::TYPE_OBB:
		m_isColl = m_pBoundDesc->Intersects(*static_cast<CBounding_OBB*>(pTargetBounding)->Get_Desc());
		break;

	case CCollider::TYPE_SPHERE:
		m_isColl = m_pBoundDesc->Intersects(*static_cast<CBounding_Sphere*>(pTargetBounding)->Get_Desc());
		break;
	}

	return m_isColl;	
}



_bool CBounding_AABB::Intersect_to_AABB(CBounding_AABB * pTarget)
{
	_float3			vSourMin, vSourMax;
	_float3			vDestMin, vDestMax;

	vSourMin = _float3(m_pBoundDesc->Center.x - m_pBoundDesc->Extents.x, 
		m_pBoundDesc->Center.y - m_pBoundDesc->Extents.y,
		m_pBoundDesc->Center.z - m_pBoundDesc->Extents.z);
	vSourMax = _float3(m_pBoundDesc->Center.x + m_pBoundDesc->Extents.x,
		m_pBoundDesc->Center.y + m_pBoundDesc->Extents.y,
		m_pBoundDesc->Center.z + m_pBoundDesc->Extents.z);

	vDestMin = _float3(pTarget->m_pBoundDesc->Center.x - pTarget->m_pBoundDesc->Extents.x,
		pTarget->m_pBoundDesc->Center.y - pTarget->m_pBoundDesc->Extents.y,
		pTarget->m_pBoundDesc->Center.z - pTarget->m_pBoundDesc->Extents.z);
	vDestMax = _float3(pTarget->m_pBoundDesc->Center.x + pTarget->m_pBoundDesc->Extents.x,
		pTarget->m_pBoundDesc->Center.y + pTarget->m_pBoundDesc->Extents.y,
		pTarget->m_pBoundDesc->Center.z + pTarget->m_pBoundDesc->Extents.z);

	/* 너비가 겹치지 않았냐? */
	if (min(vSourMax.x, vDestMax.x) < max(vSourMin.x, vDestMin.x))		
		return false;

	/* 높이가 겹치지 않았냐? */
	if (min(vSourMax.y, vDestMax.y) < max(vSourMin.y, vDestMin.y))
		return false;

	/* 깊이가 겹치지 않았냐? */
	if (min(vSourMax.z, vDestMax.z) < max(vSourMin.z, vDestMin.z))
		return false;

	return true;
}

CBounding_AABB * CBounding_AABB::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const BOUND_DESC * pBoundDesc)
{
	CBounding_AABB*	pInstance = new CBounding_AABB(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pBoundDesc)))
	{
		MSG_BOX("Failed to Created : CBounding_AABB");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBounding_AABB::Free()
{
	__super::Free();

	Safe_Delete(m_pOriginalBoundDesc);
	Safe_Delete(m_pBoundDesc);
}

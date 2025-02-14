#include "..\Public\QuadTree.h"
#include "GameInstance.h"

CQuadTree::CQuadTree()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CQuadTree::Initialize(_uint iLT, _uint iRT, _uint iRB, _uint iLB)
{
	m_iCorners[CORNER_LT] = iLT;
	m_iCorners[CORNER_RT] = iRT;
	m_iCorners[CORNER_RB] = iRB;
	m_iCorners[CORNER_LB] = iLB;

	if (1 == m_iCorners[CORNER_RT] - m_iCorners[CORNER_LT])
		return S_OK;

	m_iCenter = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RB]) >> 1;

	_uint		iLC, iTC, iRC, iBC;

	iLC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_LB]) >> 1;
	iTC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RT]) >> 1;
	iRC = (m_iCorners[CORNER_RT] + m_iCorners[CORNER_RB]) >> 1;
	iBC = (m_iCorners[CORNER_LB] + m_iCorners[CORNER_RB]) >> 1;

	m_pChildren[CORNER_LT] = CQuadTree::Create(m_iCorners[CORNER_LT], iTC, m_iCenter, iLC);
	m_pChildren[CORNER_RT] = CQuadTree::Create(iTC, m_iCorners[CORNER_RT], iRC, m_iCenter);
	m_pChildren[CORNER_RB] = CQuadTree::Create(m_iCenter, iRC, m_iCorners[CORNER_RB], iBC);
	m_pChildren[CORNER_LB] = CQuadTree::Create(iLC, m_iCenter, iBC, m_iCorners[CORNER_LB]);

	return S_OK;
}

void CQuadTree::Culling(const _float3* pVertexPositions, _uint* pIndices, _uint* pNumIndices, _fmatrix WorldMatrixInv)
{
	if (nullptr == m_pChildren[CORNER_LT] || 
		true == isDraw(pVertexPositions, WorldMatrixInv))
	{
		_uint		iIndices[] = {
			m_iCorners[CORNER_LT], 
			m_iCorners[CORNER_RT],
			m_iCorners[CORNER_RB],
			m_iCorners[CORNER_LB],
		};

		_bool		isDraw[4] = {true, true, true, true};

		for (size_t i = 0; i < 4; i++)
		{
			if(nullptr != m_pNeighbors[i])
				isDraw[i] = m_pNeighbors[i]->isDraw(pVertexPositions, WorldMatrixInv);
		}
		

		_bool		isIn[4] = {
			m_pGameInstance->isIn_Frustum_InLocalSpace(XMLoadFloat3(&pVertexPositions[iIndices[0]])),
			m_pGameInstance->isIn_Frustum_InLocalSpace(XMLoadFloat3(&pVertexPositions[iIndices[1]])),
			m_pGameInstance->isIn_Frustum_InLocalSpace(XMLoadFloat3(&pVertexPositions[iIndices[2]])),
			m_pGameInstance->isIn_Frustum_InLocalSpace(XMLoadFloat3(&pVertexPositions[iIndices[3]]))
		};

		if (true == isDraw[0] &&
			true == isDraw[1] &&
			true == isDraw[2] &&
			true == isDraw[3])
		{
			if (true == isIn[0] ||
				true == isIn[1] ||
				true == isIn[2])
			{
				pIndices[(*pNumIndices)++] = iIndices[0];
				pIndices[(*pNumIndices)++] = iIndices[1];
				pIndices[(*pNumIndices)++] = iIndices[2];
			}

			if (true == isIn[0] ||
				true == isIn[2] ||
				true == isIn[3])
			{
				pIndices[(*pNumIndices)++] = iIndices[0];
				pIndices[(*pNumIndices)++] = iIndices[2];
				pIndices[(*pNumIndices)++] = iIndices[3];
			}
			return;
		}

		_uint		iLC, iTC, iRC, iBC;

		iLC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_LB]) >> 1;
		iTC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RT]) >> 1;
		iRC = (m_iCorners[CORNER_RT] + m_iCorners[CORNER_RB]) >> 1;
		iBC = (m_iCorners[CORNER_LB] + m_iCorners[CORNER_RB]) >> 1;

		if (true == isIn[0] ||
			true == isIn[2] ||
			true == isIn[3])
		{
			if (false == isDraw[NEIGHBOR_LEFT])
			{
				pIndices[(*pNumIndices)++] = iIndices[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iLC;

				pIndices[(*pNumIndices)++] = iLC;
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iIndices[CORNER_LB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = iIndices[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iIndices[CORNER_LB];
			}

			if (false == isDraw[NEIGHBOR_BOTTOM])
			{
				pIndices[(*pNumIndices)++] = iIndices[CORNER_LB];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iBC;

				pIndices[(*pNumIndices)++] = iBC;
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iIndices[CORNER_RB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = iIndices[CORNER_LB];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iIndices[CORNER_RB];
			}
		}
		if (true == isIn[0] ||
			true == isIn[1] ||
			true == isIn[2])
		{
			if (false == isDraw[NEIGHBOR_TOP])
			{
				pIndices[(*pNumIndices)++] = iIndices[CORNER_LT];
				pIndices[(*pNumIndices)++] = iTC;
				pIndices[(*pNumIndices)++] = m_iCenter;

				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iTC;
				pIndices[(*pNumIndices)++] = iIndices[CORNER_RT];
			}
			else
			{
				pIndices[(*pNumIndices)++] = iIndices[CORNER_LT];
				pIndices[(*pNumIndices)++] = iIndices[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCenter;
			}

			if (false == isDraw[NEIGHBOR_RIGHT])
			{
				pIndices[(*pNumIndices)++] = iIndices[CORNER_RT];
				pIndices[(*pNumIndices)++] = iRC;
				pIndices[(*pNumIndices)++] = m_iCenter;

				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iRC;
				pIndices[(*pNumIndices)++] = iIndices[CORNER_RB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = iIndices[CORNER_RT];
				pIndices[(*pNumIndices)++] = iIndices[CORNER_RB];
				pIndices[(*pNumIndices)++] = m_iCenter;
			}
		}

		return;
	}

	
	_float		fRange = XMVectorGetX(XMVector3Length(XMLoadFloat3(&pVertexPositions[m_iCorners[CORNER_LT]]) - XMLoadFloat3(&pVertexPositions[m_iCenter])));

	if (true == m_pGameInstance->isIn_Frustum_InLocalSpace(XMLoadFloat3(&pVertexPositions[m_iCenter]), fRange))
	{
		for (size_t i = 0; i < CORNER_END; i++)
		{
			if (nullptr != m_pChildren[i])
				m_pChildren[i]->Culling(pVertexPositions, pIndices, pNumIndices, WorldMatrixInv);
		}	
	}	

}

void CQuadTree::Make_Neighbors()
{
	if (nullptr == m_pChildren[CORNER_LT]->m_pChildren[CORNER_LT])
		return;

	m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pChildren[CORNER_RT];
	m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pChildren[CORNER_LB];

	m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_LEFT] = m_pChildren[CORNER_LT];
	m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pChildren[CORNER_RB];

	m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_LEFT] = m_pChildren[CORNER_LB];
	m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_TOP] = m_pChildren[CORNER_RT];

	m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pChildren[CORNER_RB];
	m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_TOP] = m_pChildren[CORNER_LT];

	if (nullptr != m_pNeighbors[NEIGHBOR_RIGHT])
	{
		m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->m_pChildren[CORNER_LT];
		m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->m_pChildren[CORNER_LB];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_BOTTOM])
	{
		m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->m_pChildren[CORNER_LT];
		m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->m_pChildren[CORNER_RT];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_LEFT])
	{
		m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->m_pChildren[CORNER_RT];
		m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->m_pChildren[CORNER_RB];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_TOP])
	{
		m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->m_pChildren[CORNER_LB];
		m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->m_pChildren[CORNER_RB];
	}

	for (size_t i = 0; i < CORNER_END; i++)
	{
		if (nullptr != m_pChildren[i])
			m_pChildren[i]->Make_Neighbors();
	}
	
}

_bool CQuadTree::isDraw(const _float3 * pVertexPositions, _fmatrix WorldMatrixInv)
{
	_vector		vCamPosition = XMLoadFloat4(m_pGameInstance->Get_CamPosition());

	vCamPosition = XMVector3TransformCoord(vCamPosition, WorldMatrixInv);

	_float		fDistance = XMVectorGetX(XMVector3Length(vCamPosition - XMLoadFloat3(&pVertexPositions[m_iCenter])));

	_float		fWidth = m_iCorners[CORNER_RT] - m_iCorners[CORNER_LT];

	if (fDistance * 0.2f > fWidth)
		return true;

	return false;
}

CQuadTree * CQuadTree::Create(_uint iLT, _uint iRT, _uint iRB, _uint iLB)
{
	CQuadTree*	pInstance = new CQuadTree();

	if (FAILED(pInstance->Initialize(iLT, iRT, iRB, iLB)))
	{
		MSG_BOX("Failed to Created : CQuadTree");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CQuadTree::Free()
{
	__super::Free();

	for (auto& pChild : m_pChildren)
		Safe_Release(pChild);

	Safe_Release(m_pGameInstance);
}


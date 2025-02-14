#pragma once

#include "Base.h"

/* 가장 큰 지형의 정보르를 표현한 노드 */

BEGIN(Engine)

class CQuadTree final : public CBase
{
public:
	enum CORNER { CORNER_LT, CORNER_RT, CORNER_RB, CORNER_LB, CORNER_END };
	enum NEIGHBOR { NEIGHBOR_LEFT, NEIGHBOR_TOP, NEIGHBOR_RIGHT, NEIGHBOR_BOTTOM, NEIGHBOR_END };
private:
	CQuadTree();
	virtual ~CQuadTree() = default;

public:
	HRESULT Initialize(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	void Culling(const _float3* pVertexPositions, _uint* pIndices, _uint* pNumIndices, _fmatrix WorldMatrixInv);
	void Make_Neighbors();
private:
	_uint							m_iCorners[CORNER_END] = {};
	_uint							m_iCenter = {};
	CQuadTree*						m_pChildren[CORNER_END] = { nullptr };
	CQuadTree*						m_pNeighbors[NEIGHBOR_END] = { nullptr };
	class CGameInstance*			m_pGameInstance = { nullptr };

private:
	_bool isDraw(const _float3* pVertexPositions, _fmatrix WorldMatrixInv);

public:
	static CQuadTree* Create(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	virtual void Free() override;
};

END
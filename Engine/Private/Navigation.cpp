#include "..\Public\Navigation.h"
#include "Cell.h"

#include "Shader.h"
#include "GameInstance.h"

_float4x4 CNavigation::m_WorldMatrix = {};

CNavigation::CNavigation(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent { pDevice, pContext }	
{	

}

CNavigation::CNavigation(const CNavigation & Prototype)
	: CComponent{ Prototype }
	, m_Cells { Prototype.m_Cells }
#ifdef _DEBUG
	, m_pShader { Prototype.m_pShader}
#endif
{
#ifdef _DEBUG
	Safe_AddRef(m_pShader);
#endif
	for (auto& pCell : m_Cells)
		Safe_AddRef(pCell);
}

HRESULT CNavigation::Initialize_Prototype(const _tchar * pNavigationDataFile)
{
	_ulong			dwByte = {};
	HANDLE			hFile = CreateFile(pNavigationDataFile, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (0 == hFile)
		return E_FAIL;

	while (true)
	{
		_float3		vPoints[3] = {};

		ReadFile(hFile, vPoints, sizeof(_float3) * 3, &dwByte, nullptr);
		if (0 == dwByte)
			break;

		CCell*			pCell = CCell::Create(m_pDevice, m_pContext, vPoints, m_Cells.size());
		if (nullptr == pCell)
			return E_FAIL;

		m_Cells.push_back(pCell);
	}

	CloseHandle(hFile);

	if (FAILED(SetUp_Neighbors()))
		return E_FAIL;

#ifdef _DEBUG

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../../EngineSDK/hlsl/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

#endif

	return S_OK;
}

HRESULT CNavigation::Initialize(void * pArg)
{
	if (nullptr == pArg)
		return S_OK;

	NAVIGATION_DESC*		pDesc = static_cast<NAVIGATION_DESC*>(pArg);

	m_iCurrentIndex = pDesc->iStartIndex;

	return S_OK;
}

void CNavigation::Update(_fmatrix WorldMatrix)
{
	XMStoreFloat4x4(&m_WorldMatrix, WorldMatrix);
}

_vector CNavigation::SetUp_OnNavigation(_fvector vPosition)
{
	_vector			vLocalPos = XMVector3TransformCoord(vPosition, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	vLocalPos = XMVectorSetY(vPosition, Compute_Height(m_iCurrentIndex, vLocalPos));

	return XMVector3TransformCoord(vLocalPos, XMLoadFloat4x4(&m_WorldMatrix));
}

_bool CNavigation::isMove(_fvector vWorldPos)
{
	_vector			vLocalPos = XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	_int			iNeighborIndex = { -1 };	

	/* 원래 존재하고 있던 셀안에서만 움직인 것이다. */
	if (true == m_Cells[m_iCurrentIndex]->isIn(vLocalPos, iNeighborIndex))
		return true;	

	/* 원래 존재하고 있던 셀밖으로 움직인 것이다. */
	else
	{
		/* 나간 방향에 이웃이 없는 것이ㅏㄷ. */
		if(-1 == iNeighborIndex)
			return false;

		/* 나간 방향에 이웃이 있다. */
		else
		{
			/*if (-1 != iNeighborIndex)
			{
				m_iCurrentIndex = iNeighborIndex;
				return true;
			}*/
			while (true)
			{
				if (-1 == iNeighborIndex)
					return false;

				if(true == m_Cells[iNeighborIndex]->isIn(vLocalPos, iNeighborIndex))
				{
					m_iCurrentIndex = iNeighborIndex;
					return true;
				}
			}
			
			return true;
		}
	}
}

#ifdef _DEBUG

HRESULT CNavigation::Render()
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	
	m_pShader->Begin(0);

	for (auto& pCell : m_Cells)
	{
		if (nullptr != pCell)
			pCell->Render();
	}

	return S_OK;
}

#endif

HRESULT CNavigation::SetUp_Neighbors()
{
	for (auto& pSourCell : m_Cells)
	{
		for (auto& pDestCell : m_Cells)
		{
			if (pSourCell == pDestCell)
				continue;

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(CCell::POINT_A), pSourCell->Get_Point(CCell::POINT_B)))			
				pSourCell->Set_Neighbor(CCell::LINE_AB, pDestCell);

			else if (true == pDestCell->Compare_Points(pSourCell->Get_Point(CCell::POINT_B), pSourCell->Get_Point(CCell::POINT_C)))
				pSourCell->Set_Neighbor(CCell::LINE_BC, pDestCell);

			else if (true == pDestCell->Compare_Points(pSourCell->Get_Point(CCell::POINT_C), pSourCell->Get_Point(CCell::POINT_A)))
				pSourCell->Set_Neighbor(CCell::LINE_CA, pDestCell);			
		}
	}

	return S_OK;
}

_float CNavigation::Compute_Height(_int iCellIndex, _fvector vPosition)
{
	if (-1 == iCellIndex)
		return 0.f;

	return m_Cells[iCellIndex]->Compute_Height(vPosition);	
}

CNavigation * CNavigation::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _tchar * pNavigationDataFile)
{
	CNavigation*	pInstance = new CNavigation(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pNavigationDataFile)))
	{
		MSG_BOX("Failed to Created : CNavigation");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CComponent * CNavigation::Clone(void * pArg)
{
	CNavigation*	pInstance = new CNavigation(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNavigation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNavigation::Free()
{
	__super::Free();

#ifdef _DEBUG
	Safe_Release(m_pShader);
#endif

	for (auto& pCell : m_Cells)
		Safe_Release(pCell);

	m_Cells.clear();
}

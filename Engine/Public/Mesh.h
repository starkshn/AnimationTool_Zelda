#pragma once

#include "VIBuffer.h"
#include "Model.h"

BEGIN(Engine)

class CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);	
	virtual ~CMesh() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(MODEL_TYPE eModelType, class CModel* pModel, const aiMesh* pAIMesh, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg);

public:
	HRESULT Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, const vector<CBone*>& Bones);

public:
	const _char* Get_MeshName() { return m_szName; }
	_int Get_MeshMaterialIdx() { return m_iMaterialIndex; }
	_int Get_MeshNumBones() { return m_iNumBones; }
	// vector<_uint>& Get_BondIndices() { return m_BoneIndices; }
	// vector<_float4x4>& Get_BoneOffsetMatrices() { return m_BoneOffsetMatrices; }
	

private:
	_char						m_szName[MAX_PATH] = ""; 
	_uint						m_iMaterialIndex = {};
	_uint						m_iNumBones = {};

	/* 이 모델을 구성하는 전체 뼈의 몇번째 뼈들이 이 메시에 영향ㅇ르 주는거야 라는 정보를 저장. */
	vector<_uint>				m_BoneIndices;
	vector<_float4x4>			m_BoneOffsetMatrices;
	_float4x4					m_BoneMatrices[512];
	

public:
	HRESULT Ready_VertexBuffer_For_NonAnim(const aiMesh* pAIMesh, _fmatrix PreTransformMatrix);
	HRESULT Ready_VertexBuffer_For_Anim(const aiMesh* pAIMesh, class CModel* pModel);



public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL_TYPE eModelType, class CModel* pModel, const aiMesh* pAIMesh, _fmatrix PreTransformMatrix);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;


};

END
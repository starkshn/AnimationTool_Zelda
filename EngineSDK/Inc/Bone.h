#pragma once

#include "Base.h"
#include "Model.h"

/* assimp에서는 뼈의 정보를 세가지 타입으로 제공한다. */
/* aiNode, aiBone, aiNodeAnim */

BEGIN(Engine)

class CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	const _char* Get_Name() const {
		return m_szName;
	}

	_matrix Get_CombinedTransformationMatrix() const { 
		return XMLoadFloat4x4(&m_CombindTransformationMatrix);
	}

	const _float4x4* Get_CombinedTransformationFloat4x4() const {
		return &m_CombindTransformationMatrix;
	}

	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	}

public:
	HRESULT Initialize(const aiNode* pAINode, _int iParentBoneIndex);

	void Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix PreTransformMatrix, CModel* pModelCom);

	// =======================================================================================================================================================
	// NY

	void Set_Bone_CombinedTransformationMatrix(_fmatrix CombinedMatrix) // 250205
	{
		XMStoreFloat4x4(&m_CombindTransformationMatrix, CombinedMatrix);
	}
	
	_bool Is_RootBoneName(CModel* pModelCom); // 250122

	_float4x4* Get_TransformationMatrix_Ptr()
	{
		return &m_TransformationMatrix;
	}

	void Set_Bone_TransformMatrix(_fmatrix MatTransformLocal)
	{
		XMStoreFloat4x4(&m_TransformationMatrix, MatTransformLocal);
	}

	// IK
	_bool Is_UpdateExcludeBone(CModel* pModelCom); // 250224

	// 업데이트 한다, 애니매이션 업데이트에서 제외된 뼈의 TransformationMatrix를
	// - 모델에서 지정한 애니매이션 업데이트에서 제외된 뼈를 찾아 TransformationMatrix를 구성하게 하는 함수
	void Update_UpdateExcludeBone_TransformationMatrix(CModel* pModelCom); // 250207

	// 250208
	_int Get_Parent_BoneIdx()
	{
		return m_iParentBoneIndex;
	}

	// =======================================================================================================================================================



private:
	_char				m_szName[MAX_PATH] = {};

	/* 부모를 기준으로 표현한 나만의 상태 행렬.  */
	_float4x4			m_TransformationMatrix = {};

	/* m_TransformationMatrix * Parent`s m_CombindTransformationMatrix  */
	_float4x4			m_CombindTransformationMatrix = {};

	_int				m_iParentBoneIndex = { -1 };

public:
	static CBone* Create(const aiNode*	pAINode, _int iParentBoneIndex);
	virtual CBone* Clone();
	virtual void Free() override;
};

END
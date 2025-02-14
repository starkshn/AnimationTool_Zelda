#include "..\Public\Bone.h"
#include "GameObject.h"

CBone::CBone()
{

}

HRESULT CBone::Initialize(const aiNode * pAINode, _int iParentBoneIndex)
{
	strcpy_s(m_szName, pAINode->mName.data);

	memcpy(&m_TransformationMatrix, &pAINode->mTransformation, sizeof(_float4x4));

	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));
	XMStoreFloat4x4(&m_CombindTransformationMatrix, XMMatrixIdentity());

	m_iParentBoneIndex = iParentBoneIndex;

	return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix PreTransformMatrix, CModel* pModelCom)
{
	if (-1 == m_iParentBoneIndex)
	{
		XMStoreFloat4x4(&m_CombindTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * PreTransformMatrix);
		return;
	}

	if (pModelCom)
	{
		ANIM_PLAY_TYPE eAnimPlayType = pModelCom->Get_AnimPlayType();
		
		switch (eAnimPlayType)
		{
		case Engine::ANIM_PLAY_TYPE::ANIM_DEFAULT: // 기본 애니매이션 재생
		{
			XMStoreFloat4x4(&m_CombindTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombindTransformationMatrix));
		}
		break;
		case Engine::ANIM_PLAY_TYPE::ANIM_ROOT_MOTION: // 루트 모션(위치만 원점 고정, 회전은 유지)
		{
			if (Is_RootBoneName(pModelCom))
			{
				m_TransformationMatrix._41 = 0.f;
				m_TransformationMatrix._42 = 0.f;
				m_TransformationMatrix._43 = 0.f;
				// return;
			}

			//string strUpdateExBoneName = pModelCom->Get_UpdateExcludeBoneName();
			//_int iParentBoneIdx = pModelCom->Get_Parent_BoneIdx(strUpdateExBoneName);
			//string strBoneName = pModelCom->Get_BoneName(iParentBoneIdx);
			//if (!strcmp(strBoneName.c_str(), m_szName))
			//{
			//	XMStoreFloat4x4(&m_CombindTransformationMatrix, XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombindTransformationMatrix));
			//	return;
			//}

			// 250204
			// 업데이트에서 제외시킨 뼈의 이름이 None("", "none")이 아니라면 행렬을 Get_Update_Exclude_BoneMatrix로 구성한다.
			if (Is_UpdateExcludeBone(pModelCom))
			{
				Update_UpdateExcludeBone_TransformationMatrix(pModelCom); // 250207
			}

			XMStoreFloat4x4(&m_CombindTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombindTransformationMatrix));
		}
		break;
		case Engine::ANIM_PLAY_TYPE::ANIM_IDENTITY: // 위치 원점 고정, 회전 0 -> 항등
		{
			if (Is_RootBoneName(pModelCom))
			{
				m_TransformationMatrix._11 = 1.f;
				m_TransformationMatrix._12 = 0.f;
				m_TransformationMatrix._13 = 0.f;

				m_TransformationMatrix._21 = 0.f;
				m_TransformationMatrix._22 = 1.f;
				m_TransformationMatrix._23 = 0.f;

				m_TransformationMatrix._31 = 0.f;
				m_TransformationMatrix._32 = 0.f;
				m_TransformationMatrix._33 = 1.f;

				m_TransformationMatrix._41 = 0.f;
				m_TransformationMatrix._42 = 0.f;
				m_TransformationMatrix._43 = 0.f;
			}

			XMStoreFloat4x4(&m_CombindTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombindTransformationMatrix));
		}
		break;
		// 루트 뼈를 따라가는 경우
		case Engine::ANIM_PLAY_TYPE::ANIM_FOLLOW_ROOT_BONE:
		{
			if (Is_RootBoneName(pModelCom))
			{
				auto pContainerObj = pModelCom->Get_Owner()->Get_Parent();
				if (pContainerObj)
				{
					XMStoreFloat4x4(&m_CombindTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombindTransformationMatrix));
					pModelCom->Get_RootBoneCombinedTransfomationMatrix() = XMLoadFloat4x4(&m_CombindTransformationMatrix);

					m_TransformationMatrix._41 = 0.f;
					m_TransformationMatrix._42 = 0.f;
					m_TransformationMatrix._43 = 0.f;

					XMStoreFloat4x4(&m_CombindTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * PreTransformMatrix);
					return;
				}
			}
			
			XMStoreFloat4x4(&m_CombindTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombindTransformationMatrix));
		}
		break;
		default:
			break;
		}
	}
	else
	{
		XMStoreFloat4x4(&m_CombindTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombindTransformationMatrix));
	}
}

_bool CBone::Is_RootBoneName(CModel* pModelCom)
{
	if (!strcmp(m_szName, pModelCom->Get_RootMotionBoneName().c_str()))
		return true;
	return false;
}


_bool CBone::Is_UpdateExcludeBone(CModel* pModelCom)
{
	if (pModelCom->Get_UpdateExcludeBoneName() == "None" || pModelCom->Get_UpdateExcludeBoneName() == "none" || pModelCom->Get_UpdateExcludeBoneName() == "")
		return false;

	if (!strcmp(m_szName, pModelCom->Get_UpdateExcludeBoneName().c_str()))
		return true;

	return false;
}

void CBone::Update_UpdateExcludeBone_TransformationMatrix(CModel* pModelCom)
{
	// Model에서 애니매이션 업데이트 제외 시키기로한 행렬을 가지고 와서 현재 뼈의 로컬 행렬로 구성한다.
	// 모델에서 타겟을 향한 회전 행렬을 연산하여 저장한 것을 해당 뼈에서 들고와서 자기 자신의 로컬 행렬로 구성을 해준다.
	_float4x4 UpdateExcludeBoneTransformationMatrix = pModelCom->Get_UpdateExcludeBone_TransformationMatrix();
	XMStoreFloat4x4(&m_TransformationMatrix, XMLoadFloat4x4(&UpdateExcludeBoneTransformationMatrix));
}

CBone * CBone::Create(const aiNode * pAINode, _int iParentBoneIndex)
{
	CBone*	pInstance = new CBone();

	if (FAILED(pInstance->Initialize(pAINode, iParentBoneIndex)))
	{
		MSG_BOX("Failed to Created : CBone");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CBone * CBone::Clone()
{
	return new CBone(*this);	
}

void CBone::Free()
{
	__super::Free();


}

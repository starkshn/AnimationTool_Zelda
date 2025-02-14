#include "..\Public\Channel.h"
#include "Model.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const aiNodeAnim * pAIChannel, const CModel* pModel)
{
	strcpy_s(m_szName, pAIChannel->mNodeName.data);

	m_iBoneIndex = pModel->Get_BoneIndex(m_szName);

	m_iNumKeyFrames = max(pAIChannel->mNumScalingKeys, pAIChannel->mNumRotationKeys);

	m_iNumKeyFrames = max(m_iNumKeyFrames, pAIChannel->mNumPositionKeys);

	_float3			vScale, vPosition;
	_float4			vRotation;

	for (size_t i = 0; i < m_iNumKeyFrames; i++)
	{		
		KEYFRAME		KeyFrame = {};

		if (i < pAIChannel->mNumScalingKeys)
		{
			memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(_float3));
			KeyFrame.fKeyFramePosition = (float)pAIChannel->mScalingKeys[i].mTime;
		}
		if (i < pAIChannel->mNumRotationKeys)
		{
			vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;	
			KeyFrame.fKeyFramePosition = (float)pAIChannel->mRotationKeys[i].mTime;
		}
		if (i < pAIChannel->mNumPositionKeys)
		{
			memcpy(&vPosition, &pAIChannel->mPositionKeys[i].mValue, sizeof(_float3));
			KeyFrame.fKeyFramePosition = (float)pAIChannel->mPositionKeys[i].mTime;
		}

		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vPosition = vPosition;

		m_KeyFrames.push_back(KeyFrame);
	}

	return S_OK;
}

void CChannel::Update_TransformationMatrix(_float fCurrentTrackPosition, _uint* pKeyFrameIndex, const vector<class CBone*>& Bones)
{
	if (0.f == fCurrentTrackPosition)
		*pKeyFrameIndex = 0;

	if (Bones.size() <= m_iBoneIndex)
		return;

	static KEYFRAME		LastKeyFrame = m_KeyFrames.back();
	LastKeyFrame = m_KeyFrames.back();

	_float3			vScale;
	_float4			vRotation;
	_float3			vPosition;

	if (fCurrentTrackPosition >= LastKeyFrame.fKeyFramePosition)
	{
		vScale = LastKeyFrame.vScale;
		vRotation = LastKeyFrame.vRotation;
		vPosition = LastKeyFrame.vPosition;
	}
	else
	{
		while (fCurrentTrackPosition >= m_KeyFrames[*pKeyFrameIndex + 1].fKeyFramePosition)
			++*pKeyFrameIndex;

		_float			fRatio = (fCurrentTrackPosition - m_KeyFrames[*pKeyFrameIndex].fKeyFramePosition) / (m_KeyFrames[*pKeyFrameIndex + 1].fKeyFramePosition - m_KeyFrames[*pKeyFrameIndex].fKeyFramePosition);
		fRatio = min(max(0.f, fRatio), 1.f);


		_float3			vLeftScale, vRightScale;
		_float4			vLeftRotation, vRightRotation;
		_float3			vLeftPosition, vRightPosition;

		vLeftScale = m_KeyFrames[*pKeyFrameIndex].vScale;
		vLeftRotation = m_KeyFrames[*pKeyFrameIndex].vRotation;
		vLeftPosition = m_KeyFrames[*pKeyFrameIndex].vPosition;

		vRightScale = m_KeyFrames[*pKeyFrameIndex + 1].vScale;
		vRightRotation = m_KeyFrames[*pKeyFrameIndex + 1].vRotation;
		vRightPosition = m_KeyFrames[*pKeyFrameIndex + 1].vPosition;

		XMStoreFloat3(&vScale, XMVectorLerp(XMLoadFloat3(&vLeftScale), XMLoadFloat3(&vRightScale), fRatio));
		XMStoreFloat4(&vRotation, XMQuaternionSlerp(XMLoadFloat4(&vLeftRotation), XMLoadFloat4(&vRightRotation), fRatio));
		XMStoreFloat3(&vPosition, XMVectorLerp(XMLoadFloat3(&vLeftPosition), XMLoadFloat3(&vRightPosition), fRatio));

		m_vCurScale = vScale;
		m_vCurRotation = vRotation;
		m_vCurPosition = vPosition;
	}

	// 회전을 나타내는 RotationQuaternion 매개변수는 쿼터니언이며, 이를 기반으로 회전 행렬을 생성
	Bones[m_iBoneIndex]->Set_TransformationMatrix(XMMatrixAffineTransformation(XMLoadFloat3(&vScale), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&vRotation), XMVectorSetW(XMLoadFloat3(&vPosition), 1.f)));
}


// =====================================================================================================
// NY

void CChannel::Update_TransformationMatrix_Interpolation(_float fCurInterTrackPos, const _float fInterTime, _uint* pKeyFrameIndex, KEYFRAME InterDstKeyFrame, const vector<class CBone*>& Bones, _bool bChainInterpolation)
{
	_float3		vScale, vPosition;
	_float4		vRotation;

	_float fRatio = fCurInterTrackPos / fInterTime;
	fRatio = min(max(0.f, fRatio), 1.f);

	_float3	vS0, vS1, vP0, vP1;
	_float4	vR0, vR1;

	// 250130
	// 보간인 경우 현재 키프레임 scale, rot, pos를 시작점으로 지정한다.
	if (!bChainInterpolation)
	{
		vS0 = m_KeyFrames[*pKeyFrameIndex].vScale;
		vR0 = m_KeyFrames[*pKeyFrameIndex].vRotation;
		vP0 = m_KeyFrames[*pKeyFrameIndex].vPosition;
	}
	// 보보간일 때, 업데이트를 해왔던 보간중의 scale, rot, pos를 시작점으로 한다.
	else
	{
		vS0 = m_vCurScaleInter;
		vR0 = m_vCurRotationInter;
		vP0 = m_vCurPositionInter;
	}

	vS1 = InterDstKeyFrame.vScale;
	vR1 = InterDstKeyFrame.vRotation;
	vP1 = InterDstKeyFrame.vPosition;

	XMStoreFloat3(&vScale, XMVectorLerp(XMLoadFloat3(&vS0), XMLoadFloat3(&vS1), fRatio));
	XMStoreFloat4(&vRotation, XMQuaternionSlerp(XMLoadFloat4(&vR0), XMLoadFloat4(&vR1), fRatio));
	XMStoreFloat3(&vPosition, XMVectorLerp(XMLoadFloat3(&vP0), XMLoadFloat3(&vP1), fRatio));

	// 보보간 중이 아닐 때, 보간중인 scale, rotaion, position을 업데이트 한다.
	if (!bChainInterpolation)
	{
		m_vCurScaleInter = vScale;
		m_vCurRotationInter = vRotation;
		m_vCurPositionInter = vPosition;
	}

	// 보간이든 보보간이든 언제든지 scale, rot, pos를 추적
	m_vCurScaleTemp = vScale;
	m_vCurRotationTemp = vRotation;
	m_vCurPositionTemp = vPosition;


	Bones[m_iBoneIndex]->Set_TransformationMatrix(XMMatrixAffineTransformation(XMLoadFloat3(&vScale), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&vRotation), XMVectorSetW(XMLoadFloat3(&vPosition), 1.f)));
}

void CChannel::Update_TransformationMatrix_Interpolation(_float fCurInterTrackPos, const _float fInterTime, _uint* pKeyFrameIndex, KEYFRAME InterSrcKeyFrame, KEYFRAME InterDstKeyFrame, const vector<class CBone*>& Bones, _bool bChainInterpolation)
{
	_float3 vScale, vPosition;
	_float4 vRotation;

	// 보간 비율 계산 (Ease-in-Out 적용 가능)
	_float fRatio = fCurInterTrackPos / fInterTime;
	fRatio = min(max(0.f, fRatio), 1.f);

	_float3	vS0, vS1, vP0, vP1;
	_float4	vR0, vR1;
	
	// 250130
	// 보보간이 아닐 때
	if (!bChainInterpolation)
	{
		vS0 = m_KeyFrames[*pKeyFrameIndex].vScale;
		vR0 = m_KeyFrames[*pKeyFrameIndex].vRotation;
		vP0 = m_KeyFrames[*pKeyFrameIndex].vPosition;
	}
	// 보보간 일 때
	else
	{
		vS0 = m_vCurScaleInter;
		vR0 = m_vCurRotationInter;
		vP0 = m_vCurPositionInter;
	}

	// 대상 애니메이션 키프레임 정보
	vS1 = InterDstKeyFrame.vScale;
	vR1 = InterDstKeyFrame.vRotation;
	vP1 = InterDstKeyFrame.vPosition;

	/*vS1 = m_vInterpolationScale;
	vR1 = m_vInterpolationRotation;
	vP1 = m_vInterpolationPosition;*/


	// 쿼터니언 정규화 (필요시)
	XMStoreFloat4(&vR0, XMQuaternionNormalize(XMLoadFloat4(&vR0)));
	XMStoreFloat4(&vR1, XMQuaternionNormalize(XMLoadFloat4(&vR1)));

	// 스케일, 회전, 위치 보간
	XMStoreFloat3(&vScale, XMVectorLerp(XMLoadFloat3(&vS0), XMLoadFloat3(&vS1), fRatio));
	XMStoreFloat4(&vRotation, XMQuaternionSlerp(XMLoadFloat4(&vR0), XMLoadFloat4(&vR1), fRatio));
	XMStoreFloat3(&vPosition, XMVectorLerp(XMLoadFloat3(&vP0), XMLoadFloat3(&vP1), fRatio));

	if (!bChainInterpolation)
	{
		m_vCurScaleInter = vScale;
		m_vCurRotationInter = vRotation;
		m_vCurPositionInter = vPosition;
	}


	// 보간이든 보보간이든 언제든지 scale, rot, pos를 추적
	m_vCurScaleTemp = vScale;
	m_vCurRotationTemp = vRotation;
	m_vCurPositionTemp = vPosition;

	// 최종 변환 행렬 계산 및 설정
	Bones[m_iBoneIndex]->Set_TransformationMatrix(XMMatrixAffineTransformation(XMLoadFloat3(&vScale), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&vRotation), XMVectorSetW(XMLoadFloat3(&vPosition), 1.f)));
}

void CChannel::Cal_DstKeyFrame(_float fAnimDurtaion, _float fDstTrackPos)
{
	_float fDstTrackPosition = fDstTrackPos;
	if (fDstTrackPos > fAnimDurtaion)
	{
		fDstTrackPosition = fDstTrackPos - fAnimDurtaion;
	}

	_int iKeyFrameSize = m_KeyFrames.size();
	_int iKeyFrameIdx = 0;

	// |-----|-----|
	// 0-----1-----2
	KEYFRAME SrcKeyFrame{};
	KEYFRAME DstKeyFrame{};

	if (iKeyFrameSize == 2)
	{
		// |-----|
		// 0-----1
		SrcKeyFrame = m_KeyFrames[0];		// 0에 해당되는 키 프레임
		DstKeyFrame = m_KeyFrames[1];		// 1에 해당되는 키 프레임
	}
	else if (iKeyFrameSize == 1)
	{
		SrcKeyFrame = m_KeyFrames[0];		// 0에 해당되는 키 프레임
		DstKeyFrame = m_KeyFrames[0];		// 1에 해당되는 키 프레임
	}
	else if (iKeyFrameSize == 0)
		return;
	else
	{
		// 0번 키프레임 부터 fDstTrackPosition에 맞는 키프레임을 찾는다.
		while (1)
		{
			if (iKeyFrameSize == iKeyFrameIdx + 1)
				break;

			if (fDstTrackPosition >= m_KeyFrames[iKeyFrameIdx].fKeyFramePosition)
				++iKeyFrameIdx;
			else
				break;
		}

		// |-----|-----|-----|
		// 0-----1-----2-----3
		SrcKeyFrame = m_KeyFrames[iKeyFrameIdx];						// 0에 해당되는 키 프레임
		DstKeyFrame = m_KeyFrames[(iKeyFrameIdx + 1) % iKeyFrameSize];	// 1에 해당되는 키 프레임
	}

	_float3 vSrcScale = SrcKeyFrame.vScale;
	_float4 vSrcRot = SrcKeyFrame.vRotation;
	_float3 vSrcPos = SrcKeyFrame.vPosition;
	_float fSrcKeyFramePos = SrcKeyFrame.fKeyFramePosition;

	_float3 vDstScale = DstKeyFrame.vScale;
	_float4 vDstRot = DstKeyFrame.vRotation;
	_float3 vDstPos = DstKeyFrame.vPosition;
	_float fDstKeyFramePos = DstKeyFrame.fKeyFramePosition;

	// |-----|-----|
	// 0-----1-----2
	// |--v--|-----|
	// v위치에 해당하는 비율을 구한다.
	_float fRatio = (fDstTrackPosition - fSrcKeyFramePos) / (fDstKeyFramePos - fSrcKeyFramePos);
	fRatio = min(max(0.f, fRatio), 1.f);

	// 구한 비율로 보간한다.
	XMStoreFloat3(&m_vInterpolationScale, XMVectorLerp(XMLoadFloat3(&vSrcScale), XMLoadFloat3(&vDstScale), fRatio));
	XMStoreFloat4(&m_vInterpolationRotation, XMQuaternionSlerp(XMLoadFloat4(&vSrcRot), XMLoadFloat4(&vDstRot), fRatio));
	XMStoreFloat3(&m_vInterpolationPosition, XMVectorLerp(XMLoadFloat3(&vSrcPos), XMLoadFloat3(&vDstPos), fRatio));
}

//void CChannel::Update_TransformationMatrix(_float fCurrentTrackPosition, _uint* pKeyFrameIndex, const vector<class CBone*>& Bones, _int iMinBoneIdx, _int iMaxBoneIdx)
//{
//	if (0.f == fCurrentTrackPosition)
//		*pKeyFrameIndex = 0;
//
//	if (Bones.size() <= m_iBoneIndex)
//		return;
//
//	static KEYFRAME		LastKeyFrame = m_KeyFrames.back();
//	LastKeyFrame = m_KeyFrames.back();
//
//	_float3			vScale;
//	_float4			vRotation;
//	_float3			vPosition;
//
//	if (fCurrentTrackPosition >= LastKeyFrame.fKeyFramePosition)
//	{
//		vScale = LastKeyFrame.vScale;
//		vRotation = LastKeyFrame.vRotation;
//		vPosition = LastKeyFrame.vPosition;
//	}
//	else
//	{
//		while (fCurrentTrackPosition >= m_KeyFrames[*pKeyFrameIndex + 1].fKeyFramePosition)
//			++*pKeyFrameIndex;
//
//		_float			fRatio = (fCurrentTrackPosition - m_KeyFrames[*pKeyFrameIndex].fKeyFramePosition) / (m_KeyFrames[*pKeyFrameIndex + 1].fKeyFramePosition - m_KeyFrames[*pKeyFrameIndex].fKeyFramePosition);
//
//
//		_float3			vLeftScale, vRightScale;
//		_float4			vLeftRotation, vRightRotation;
//		_float3			vLeftPosition, vRightPosition;
//
//		vLeftScale = m_KeyFrames[*pKeyFrameIndex].vScale;
//		vLeftRotation = m_KeyFrames[*pKeyFrameIndex].vRotation;
//		vLeftPosition = m_KeyFrames[*pKeyFrameIndex].vPosition;
//
//		vRightScale = m_KeyFrames[*pKeyFrameIndex + 1].vScale;
//		vRightRotation = m_KeyFrames[*pKeyFrameIndex + 1].vRotation;
//		vRightPosition = m_KeyFrames[*pKeyFrameIndex + 1].vPosition;
//
//		XMStoreFloat3(&vScale, XMVectorLerp(XMLoadFloat3(&vLeftScale), XMLoadFloat3(&vRightScale), fRatio));
//		XMStoreFloat4(&vRotation, XMQuaternionSlerp(XMLoadFloat4(&vLeftRotation), XMLoadFloat4(&vRightRotation), fRatio));
//		XMStoreFloat3(&vPosition, XMVectorLerp(XMLoadFloat3(&vLeftPosition), XMLoadFloat3(&vRightPosition), fRatio));
//
//		m_vCurScale = vScale;
//		m_vCurRotation = vRotation;
//		m_vCurPosition = vPosition;
//	}
//
//	// 회전을 나타내는 RotationQuaternion 매개변수는 쿼터니언이며, 이를 기반으로 회전 행렬을 생성
//	Bones[m_iBoneIndex]->Set_TransformationMatrix(XMMatrixAffineTransformation(XMLoadFloat3(&vScale), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&vRotation), XMVectorSetW(XMLoadFloat3(&vPosition), 1.f)));
//}


void CChannel::Find_Suitable_KeyFrameIndices(_float fCurrentTrackPosition, _uint* pKeyFrameIndex, const vector<class CBone*>& Bones)
{
	if (0.f == fCurrentTrackPosition)
		*pKeyFrameIndex = 0;

	if (Bones.size() <= m_iBoneIndex)
		return;

	static KEYFRAME		LastKeyFrame = m_KeyFrames.back();
	LastKeyFrame = m_KeyFrames.back();

	_float3			vScale;
	_float4			vRotation;
	_float3			vPosition;

	if (fCurrentTrackPosition >= LastKeyFrame.fKeyFramePosition)
	{
		vScale = LastKeyFrame.vScale;
		vRotation = LastKeyFrame.vRotation;
		vPosition = LastKeyFrame.vPosition;
	}
	else
	{
		while (fCurrentTrackPosition >= m_KeyFrames[*pKeyFrameIndex + 1].fKeyFramePosition)
			++*pKeyFrameIndex;

		_float			fRatio = (fCurrentTrackPosition - m_KeyFrames[*pKeyFrameIndex].fKeyFramePosition) / (m_KeyFrames[*pKeyFrameIndex + 1].fKeyFramePosition - m_KeyFrames[*pKeyFrameIndex].fKeyFramePosition);
		fRatio = min(max(0.f, fRatio), 1.f);


		_float3			vLeftScale, vRightScale;
		_float4			vLeftRotation, vRightRotation;
		_float3			vLeftPosition, vRightPosition;

		vLeftScale = m_KeyFrames[*pKeyFrameIndex].vScale;
		vLeftRotation = m_KeyFrames[*pKeyFrameIndex].vRotation;
		vLeftPosition = m_KeyFrames[*pKeyFrameIndex].vPosition;

		vRightScale = m_KeyFrames[*pKeyFrameIndex + 1].vScale;
		vRightRotation = m_KeyFrames[*pKeyFrameIndex + 1].vRotation;
		vRightPosition = m_KeyFrames[*pKeyFrameIndex + 1].vPosition;

		XMStoreFloat3(&vScale, XMVectorLerp(XMLoadFloat3(&vLeftScale), XMLoadFloat3(&vRightScale), fRatio));
		XMStoreFloat4(&vRotation, XMQuaternionSlerp(XMLoadFloat4(&vLeftRotation), XMLoadFloat4(&vRightRotation), fRatio));
		XMStoreFloat3(&vPosition, XMVectorLerp(XMLoadFloat3(&vLeftPosition), XMLoadFloat3(&vRightPosition), fRatio));

	}

	// 회전을 나타내는 RotationQuaternion 매개변수는 쿼터니언이며, 이를 기반으로 회전 행렬을 생성
	Bones[m_iBoneIndex]->Set_TransformationMatrix(XMMatrixAffineTransformation(XMLoadFloat3(&vScale), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&vRotation), XMVectorSetW(XMLoadFloat3(&vPosition), 1.f)));
}

CChannel * CChannel::Create(const aiNodeAnim * pAIChannel, const CModel* pModel)
{
	CChannel*	pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(pAIChannel, pModel)))
	{
		MSG_BOX("Failed to Created : CChannel");
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CChannel::Free()
{
	__super::Free();
}

#pragma once

#include "Base.h"


/* 0. 이 뼈가 소속된 애니메이션을 표현하기위해서 뼈는 어떤 상태(KeyFrame)(행렬)들을 가지는가?! 에 대한 정보를 가진다. */
/* 1. 현재 재생시간에 맞는 뼈의 상태행렬을 만들어낸다.  */
/* 2. 같은 이름을 가진 채널이 다수 존재할 수 있다?! */

BEGIN(Engine)

class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const aiNodeAnim* pAIChannel, const class CModel* pModel);
	void Update_TransformationMatrix(_float fCurrentTrackPosition, _uint* pKeyFrameIndex, const vector<class CBone*>& Bones);

	// ======================================================================================================================================
	// NY

	// 아무데서나 -> 처음을 보간하는 함수
	void Update_TransformationMatrix_Interpolation(_float fCurInterTrackPos, const _float fInterTime, _uint* pKeyFrameIndex, KEYFRAME InterDstKeyFrame, const vector<class CBone*>& Bones, _bool bChainInterpolation = false);

	// 끝과 처음을 보간하는 함수
	void Update_TransformationMatrix_Interpolation(_float fCurInterTrackPos, const _float fInterTime, _uint* pKeyFrameIndex, KEYFRAME InterSrcKeyFrame, KEYFRAME InterDstKeyFrame, const vector<class CBone*>& Bones, _bool bChainInterpolation = false);

	// 블렌딩 그룹이 호출하는 함수 250125
	// void Update_TransformationMatrix(_float fCurrentTrackPosition, _uint* pKeyFrameIndex, const vector<class CBone*>& Bones, _int iMinBoneIdx, _int iMaxBoneIdx);

	_uint Get_BoneIdx() { return m_iBoneIndex; }
	KEYFRAME Get_FirstKeyFrame() 
	{
		if (!m_KeyFrames.empty())
			return m_KeyFrames.front();
		return KEYFRAME{};
	}

	KEYFRAME Get_LastKeyFrame()
	{
		if (!m_KeyFrames.empty())
			return m_KeyFrames.back();
		return KEYFRAME{};
	}


	void Cal_DstKeyFrame(_float fAnimDurtaion, _float fDstTrackPos); // 250210


	_uint Get_NumKeyFrames() { return m_iNumKeyFrames; } // 250130
	vector<KEYFRAME>& Get_KeyFrame() { return m_KeyFrames; } // 250130
	const _char* Get_ChannelName() { return m_szName; } // 250130

	void Reset_ChainInterpolation_SRT() // 250203
	{
		m_vCurScaleInter = m_vCurScaleTemp;
		m_vCurRotationInter = m_vCurRotationTemp;
		m_vCurPositionInter = m_vCurPositionTemp;
	}

	void Setup_ChainInterpolation_BoneSRT() // 250212
	{
		m_vCurScaleInter = m_vCurScaleTemp;
		m_vCurRotationInter = m_vCurRotationTemp;
		m_vCurPositionInter = m_vCurPositionTemp;
	}

	KEYFRAME Get_InterpolationKeyFrame() 
	{
		return KEYFRAME{ m_vInterpolationScale, m_vInterpolationRotation, m_vInterpolationPosition , 0.f };
	}


private:
	_float3			m_vCurScale;
	_float4			m_vCurRotation;
	_float3			m_vCurPosition;

	_float3			m_vCurScaleInter; // 250130
	_float4			m_vCurRotationInter;
	_float3			m_vCurPositionInter;

	_float3			m_vCurScaleTemp; // 250203
	_float4			m_vCurRotationTemp;
	_float3			m_vCurPositionTemp;

	// 250212
	_float3			m_vInterpolationScale;
	_float4			m_vInterpolationRotation;
	_float3			m_vInterpolationPosition;


	// --------------------------- Anim Tool ---------------------------

	void Find_Suitable_KeyFrameIndices(_float fCurrentTrackPosition, _uint* pKeyFrameIndex, const vector<class CBone*>& Bones);

	// ---------------------------------------------------------------------------------


	// ======================================================================================================================================

private:
	_char						m_szName[MAX_PATH] = {};
	_uint						m_iNumKeyFrames = {};
	vector<KEYFRAME>			m_KeyFrames;

public:
	_uint						m_iBoneIndex = {};

public:
	static CChannel* Create(const aiNodeAnim* pAIChannel, const class CModel* pModel);
	virtual void Free() override;
};

END
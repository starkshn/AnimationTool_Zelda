#pragma once

#include "Base.h"
#include "Channel.h"

/* 1. 이 애니메이션(대기, 공격 etc)이 컨트롤해야하는 뼈(Channel)의 정보를 가진다.  */
/* 2. */

BEGIN(Engine)


class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& Prototype);
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation, const class CModel* pModel);
	_bool Update_TransformationMatrices(const vector<class CBone*>& Bones, _bool isLoop, _float fTimeDelta);

private:
	_char					m_szName[MAX_PATH] = {};
	_uint					m_iNumChannels = {};
	vector<class CChannel*> m_Channels;

	_float					m_fDuration = {};
	_float					m_fTickPerSecond = {};
	_float					m_fCurrentTrackPosition = {};

	// CAnimation을 깊은 복사 할 것이기 때문에 애니매이션이 가지고 있는 Channel들을 각각 다른 Animation객체에서 관리를 위해 Channel별 KeyFrameIndex들을 애니매이션이 관리해준다.
	// CModel은 여러개의 애니매이션을 가지고 있고 각 애니매이션은 여러개의 CChannel을 가지고 있다. 각 CChannel은 여러 키프레임을 가지고 있다.
	// - A애니매이션이 재생될 때 A가 가지고 있는 채널들의 행렬 업데이트를 한다.(보간하면서) 이때 0~i번까지 각 채널에 대해 Update_TransformMatrix를 하는데 i번 채널의 x번 째 y번째 키프레임을 보간해야할때 이 x, y의 기준을 잡아주는게 m_CurrentKeyFrameIndices 의 역할이다.
	vector<_uint>			m_CurrentKeyFrameIndices;



	// ====================================================================================================================================================================
	// NY

public:
	// 일반 애니매이션 보간 함수
	// 보간중 매틱 호출되는 함수, 보간이 끝나면 true를 반화한다.
	_bool Update_Inter_TransformationMatrices(CAnimation* pInterDstAnim, const _float fInterDis, _float fInterSP, const vector<class CBone*>& Bones, _float fTimeDelta, _float& fInterTrackPos, _bool bChainInterpolation, _bool bEndToFirstInterpolation = false);

	// 블렌딩 그룹이 호출하는 함수
	_bool Update_TransformationMatrices(const vector<class CBone*>& Bones, _bool isLoop, _int iMinBoneIdx, _int iMaxBoneIdx, _float fTimeDelta, _int iAddtionalBoneIdx = -1); // 250125

	// 블렌딩 그룹이 호출하는 함수
	_bool Update_Inter_TransformationMatrices(CAnimation* pInterDstAnim, const _float fInterDis, _float fInterSP, const vector<class CBone*>& Bones, _float fTimeDelta, _bool bEndToFirstInterpolation, _int iSrcMinBoneIdx, _int iSrcMaxBoneIdx, _int iDstMinBoneIdx, _int iDstMaxBoneIdx, _bool bChainInterpolation, _float& fInterTrackPos, _int iAdditionalBoneIdx = -1); // 250127


	const _char* Get_AnimName() { return m_szName; }
	_uint Get_NumChannels() { return m_iNumChannels; }
	vector<class CChannel*>& Get_Channels() { return m_Channels; } // 현재 애니매이션이 영향을 주는 뼈들
	_float Get_AnimDuration() { return m_fDuration; }
	_float Get_TickPerSecond() { return m_fTickPerSecond; }
	void Set_TickPerSecond(_float fTickPerSeoncd) { m_fTickPerSecond = fTickPerSeoncd; }
	_float Get_CurTrackPos() { return m_fCurrentTrackPosition; }
	vector<_uint>& Get_CurKeyFrameIndices() { return m_CurrentKeyFrameIndices; } // 현재 애니매이션의 키 프레임 인덱스
	_bool Get_IsPlayingAnimEnd() { return m_bIsPlayingAnimEnd; }
	void Set_IsPlayingAnimEnd(_bool bIsPlayingAnimEnd) { m_bIsPlayingAnimEnd = bIsPlayingAnimEnd; }
	_bool Get_IsInterpolationing() { return m_bIsInterpoationing; }
	void Set_IsInterpolationing(_bool bInterpolationing) { m_bIsInterpoationing = bInterpolationing; }
	_float Get_CurInterTrackPos() { return m_fCurInterTrackPos; }

	// 보간중이던 여러 정보를 리셋 해준다.
	void Reset_Inter_Anim();
	void Reset_TrackPos() { m_fCurrentTrackPosition = 0.f; }
	void Reset_CurKeyFrameIndices()
	{
		fill(m_CurrentKeyFrameIndices.begin(), m_CurrentKeyFrameIndices.end(), 0);
	}

private: // events 250202
	void Add_Event(string strLayerName, string strEventName, _float fTrackPos)
	{
		m_EventLayer[strLayerName].push_back(make_pair(strEventName, fTrackPos));
	}
	friend class CModel;

	void Check_Events(_float fCurTrackPos, _float fTimeDelta); // 250202

public:
	HRESULT Bind_Event(string strLayerName, string strEventName, function<void()> CallBack); // 250202
	

public: // 250203
	void Reset_ChainInterpolation_Bones_SRT() 
	{
		for (auto pCh : m_Channels)
			pCh->Reset_ChainInterpolation_SRT();
	}

	// fDstTrackPos에 해당하는 키프레임을 계산한다.
	void Cal_DstKeyFrame(_float fAnimDuration, _float fDstTrackPos); // 250210

	void Setup_ChainInterpolation_BoneSRT()
	{
		for (auto pCh : m_Channels)
			pCh->Setup_ChainInterpolation_BoneSRT();
	}

	void Set_CurTrackPos(_float fCurTrackPos); // 250130

	// ---------------------- AnimTool에서만 ----------------------

public:
	_float* Get_TrackPos_Ptr() { return &m_fCurrentTrackPosition; }

	vector<class CBone*>*	m_vecBones = nullptr;
	// ------------------------------------------------------------------

	

private:
	_bool					m_bIsPlayingAnimEnd = false; // 보간중이 아닌 애니매이션 재생이 끝난 경우 true
	_bool					m_bIsInterpoationing = false;	// 현재 애니매이션이 보간 중인지의 여부
	_bool					m_bInterAnimEnd = false;	// 보간중인 애니매이션 보간이 끝난 경우 true
	_float					m_fCurInterTrackPos = 0.f;  // 보간중인 현재 거리(시간)

	// events 250202
private:
	// map<레이어 이름, vector<pair<이벤트 이름, 트랙 포지션>>>
	map<string, vector<pair<string, _float>>> m_EventLayer;
	
	// map<레이어 이름, unordered_map<이벤트 이름, pair<TrackPos, 콜백함수>>>>
	map<string, unordered_map<string, pair<_float, function<void()>>>> m_EventCallBacks;
	

	// ====================================================================================================================================================================



public:
	static CAnimation* Create(const aiAnimation* pAIAnimation, const class CModel* pModel);
	virtual CAnimation* Clone();
	virtual void Free() override;
};

END
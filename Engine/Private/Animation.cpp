#include "..\Public\Animation.h"
#include "Channel.h"
#include "Model.h"
#include "Bone.h"

#include <iostream>
using namespace std;

CAnimation::CAnimation()
{
}

CAnimation::CAnimation(const CAnimation & Prototype)
	: m_iNumChannels { Prototype.m_iNumChannels }
	, m_Channels { Prototype.m_Channels }
	, m_fDuration{ Prototype.m_fDuration }
	, m_fTickPerSecond{ Prototype.m_fTickPerSecond }
	, m_fCurrentTrackPosition{ Prototype.m_fCurrentTrackPosition }
	, m_CurrentKeyFrameIndices { Prototype.m_CurrentKeyFrameIndices} // 얕은 복사
{
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);

	strcpy_s(m_szName, Prototype.m_szName);	

	// 250130
	// ----------------------------------------------------------------------
	// AnimTool에서만 사용
	m_vecBones = Prototype.m_vecBones;
	// ----------------------------------------------------------------------
}

HRESULT CAnimation::Initialize(const aiAnimation * pAIAnimation, const CModel* pModel)
{
	strcpy_s(m_szName, pAIAnimation->mName.data);

	m_fDuration = (float)pAIAnimation->mDuration;
	m_fTickPerSecond = (float)pAIAnimation->mTicksPerSecond;

	/* 이 애니메이션(대기, 공격 etc)이 컨트롤해야하는 뼈(Channel)들의 정보 */
	m_iNumChannels = pAIAnimation->mNumChannels;

	m_CurrentKeyFrameIndices.resize(m_iNumChannels);

	for (size_t i = 0; i < m_iNumChannels; i++)
	{
		CChannel*		pChannel = CChannel::Create(pAIAnimation->mChannels[i], pModel);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);		
	}

	// 250130
	// ----------------------------------------------------------------------
	// AnimTool에서만 사용
	if (!m_vecBones)
		m_vecBones = const_cast<CModel*>(pModel)->Get_Bones_Ptr();
	// ----------------------------------------------------------------------

	return S_OK;
}

// ======================================================================================================
// NY


// m_bIsPlayingAnimEnd : 단일 재생 애니매이션이든 루프 애니매이션이든 끝났다는 것을 알려준다. 
// 애니매이션 재생이 완벽하게 한번이 끝나면 true를 반환한다.
_bool CAnimation::Update_TransformationMatrices(const vector<class CBone*>& Bones, _bool isLoop, _float fTimeDelta)
{
	m_bIsPlayingAnimEnd = false;	// 초기화를 해준다.
	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;
	Check_Events(m_fCurrentTrackPosition, fTimeDelta); // 250202

	// 루프 애니매이션인 경우 m_fCurrentTrackPosition를 0으로 만들고 행렬을 구성한다.
	if (true == isLoop && m_fCurrentTrackPosition >= m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		m_bIsPlayingAnimEnd = true;
		for (size_t i = 0; i < m_iNumChannels; i++)
			m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, &m_CurrentKeyFrameIndices[i], Bones);
		return true;
	}
	// 루프 애니매이션이 아닌 경우 행렬을 구성하지 않는다.
	else if (m_fCurrentTrackPosition >= m_fDuration)
	{
		m_fCurrentTrackPosition = m_fDuration;
		m_bIsPlayingAnimEnd = true;
		return true;
	}

	for (size_t i = 0; i < m_iNumChannels; i++)
	{
		m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, &m_CurrentKeyFrameIndices[i], Bones);
	}

	return false;
}

_bool CAnimation::Update_Inter_TransformationMatrices(CAnimation* pInterDstAnim, const _float fInterDis, _float fInterSP, const vector<class CBone*>& Bones, _float fTimeDelta, _float& fInterTrackPos, _bool bChainInterpolation, _bool bEndToFirstInterpolation)
{
	fInterTrackPos += fTimeDelta * fInterSP;

	// 보간 시간을 초과한 경우 보간이 끝났다.
	// - 애니매이션 재생이 완벽하게 끝나고 다음 애니매이션으로 보간하는 경우 : m_fCurrentTrackPosition는 0.f일 것이기 때문애 0.f로 초기화를 안해도 된다.
	// - 애니매이션 재생중에 다음 애니매이션으로 보간하는 경우 : m_fCurrentTrackPosition는 0.f가 아닐 것이기 때문에 0.f로 초기화 해준다.
	// - 귀찮으니 그냥 m_fCurrentTrackPosition를 0.f로 초기화 해준다.
	if (fInterDis <= fInterTrackPos)
	{
		m_fCurrentTrackPosition = 0.f;
		fInterTrackPos = 0.f; // 보간 현재 거리(시간) 초기화
		return true;
	}

	auto& DstAnimChannels = pInterDstAnim->Get_Channels();
	for (_int i = 0; i < m_Channels.size(); ++i)
	{
		_uint iSrcBoneIdx = m_Channels[i]->Get_BoneIdx();
		for (auto pDstAnimCH : DstAnimChannels)
		{
			// Src BoneIdx와 DstBoneIdx가 같은경우
			if (pDstAnimCH->Get_BoneIdx() == iSrcBoneIdx)
			{
				// 끝과 처음을 보간하는 경우가 아닌 경우
				if (!bEndToFirstInterpolation)
				{
					KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_FirstKeyFrame();
					m_Channels[i]->Update_TransformationMatrix_Interpolation(fInterTrackPos, fInterDis, &m_CurrentKeyFrameIndices[i], InterDstKeyFrame, Bones, bChainInterpolation);
				}
				// 끝과 처음을 보간하는 경우
				else
				{
					KEYFRAME InterSrcKeyFrame = m_Channels[i]->Get_LastKeyFrame();
					KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_FirstKeyFrame();
					m_Channels[i]->Update_TransformationMatrix_Interpolation(fInterTrackPos, fInterDis, &m_CurrentKeyFrameIndices[i], InterSrcKeyFrame, InterDstKeyFrame, Bones, bChainInterpolation);
				}
			}
		}
	}
	
	return false;
}

_bool CAnimation::Update_Inter_TransformationMatrices(CAnimation* pInterDstAnim, const _float fInterDis, _float fInterSP, const vector<class CBone*>& Bones, _float fTimeDelta, _bool bEndToFirstInterpolation, _int iSrcMinBoneIdx, _int iSrcMaxBoneIdx, _int iDstMinBoneIdx, _int iDstMaxBoneIdx, _bool bChainInterpolation, _float& fInterTrackPos, _int iAdditionalBoneIdx)
{
	fInterTrackPos += fTimeDelta * fInterSP;

	if (fInterDis <= fInterTrackPos)
	{
		// m_fCurrentTrackPosition = 0.f;
		fInterTrackPos = 0.f; // 보간 현재 거리(시간) 초기화

#pragma region "보간이 끝남, 정보 출력"

#ifdef _DEBUG

		string strDstAnimName = pInterDstAnim->Get_AnimName();

		cout << "-----------------------------------------" << endl;
		cout << "보간이 끝남" << endl;
		cout << strDstAnimName << " 으로의 보간이 끝남" << endl;
		cout << "-----------------------------------------" << endl;

#endif

#pragma endregion

		return true;
	}

	for (_int i = 0; i < m_Channels.size(); ++i)
	{
		_uint iSrcBoneIdx = m_Channels[i]->Get_BoneIdx(); // 현재 애니매이션이 가지고 있는 실제 본 인덱스

		if (iAdditionalBoneIdx != -1)
		{
			if (iSrcBoneIdx == iAdditionalBoneIdx)
			{
				auto& DstAnimChannels = pInterDstAnim->Get_Channels(); // 보간 목적지 채널들 가져온다.
				for (auto pDstAnimCH : DstAnimChannels)
				{
					// iAdditionalBoneIdx DstBoneIdx가 다르다면 건너 뛴다.
					if (pDstAnimCH->Get_BoneIdx() != iSrcBoneIdx)
						continue;

					// 끝과 처음을 보간하는 경우가 아닌 경우
					if (!bEndToFirstInterpolation)
					{
						// KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_FirstKeyFrame();
						KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_InterpolationKeyFrame(); // 250210
						m_Channels[i]->Update_TransformationMatrix_Interpolation(fInterTrackPos, fInterDis, &m_CurrentKeyFrameIndices[i], InterDstKeyFrame, Bones, bChainInterpolation);
					}
					// 끝과 처음을 보간하는 경우
					else
					{
						KEYFRAME InterSrcKeyFrame = m_Channels[i]->Get_LastKeyFrame();
						KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_FirstKeyFrame();
						m_Channels[i]->Update_TransformationMatrix_Interpolation(fInterTrackPos, fInterDis, &m_CurrentKeyFrameIndices[i], InterSrcKeyFrame, InterDstKeyFrame, Bones, bChainInterpolation);
					}
				}
			}
			else
			{
				if ((iSrcBoneIdx < iDstMinBoneIdx) || (iSrcBoneIdx > iDstMaxBoneIdx))
					continue;

				auto& DstAnimChannels = pInterDstAnim->Get_Channels(); // 보간 목적지 채널들 가져온다.
				for (auto pDstAnimCH : DstAnimChannels)
				{
					if ((pDstAnimCH->Get_BoneIdx() < iSrcMinBoneIdx) || (pDstAnimCH->Get_BoneIdx() > iSrcMaxBoneIdx)) // LowerBody(151~194), Total(3~196)에서 151~194만 보간하기 위한 작업
						continue;

					// Src BoneIdx와 DstBoneIdx가 다르다면 건너 뛴다.
					if (pDstAnimCH->Get_BoneIdx() != iSrcBoneIdx)
						continue;

					// 끝과 처음을 보간하는 경우가 아닌 경우
					if (!bEndToFirstInterpolation)
					{
						// KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_FirstKeyFrame();
						KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_InterpolationKeyFrame(); // 250210
						m_Channels[i]->Update_TransformationMatrix_Interpolation(fInterTrackPos, fInterDis, &m_CurrentKeyFrameIndices[i], InterDstKeyFrame, Bones, bChainInterpolation);
					}
					// 끝과 처음을 보간하는 경우
					else
					{
						KEYFRAME InterSrcKeyFrame = m_Channels[i]->Get_LastKeyFrame();
						KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_FirstKeyFrame();
						m_Channels[i]->Update_TransformationMatrix_Interpolation(fInterTrackPos, fInterDis, &m_CurrentKeyFrameIndices[i], InterSrcKeyFrame, InterDstKeyFrame, Bones, bChainInterpolation);
					}
				}
			}
		}
		else
		{
			// 현재 애니매이션(A) 보간 해야하는 애니매이션(B), A의 채널이 가르키는 뼈 인덱스와 B의 채널이 가르키는 뼈 인덱스가 일치하는 경우만 업데이트를 진행한다.
			// 현재 애니매이션의 채널의 본 인덱스(A)가 목적지 애니매이션 채널의 본 인덱스(B) MinB보다 작거나 MaxB보다 크면 건너 뛴다.
			// 왜냐하면 LowerBody에서 Total로 가는 경우를 생각해보자 LowerBody(151~194), Total(3~196)이다. 이중 3~196에서 151~194만 보간해야한다.
			if (iSrcBoneIdx < iDstMinBoneIdx || iSrcBoneIdx > iDstMaxBoneIdx)
				continue;

			auto& DstAnimChannels = pInterDstAnim->Get_Channels(); // 보간 목적지 채널들 가져온다.

			for (auto pDstAnimCH : DstAnimChannels)
			{
				if (pDstAnimCH->Get_BoneIdx() < iSrcMinBoneIdx || pDstAnimCH->Get_BoneIdx() > iSrcMaxBoneIdx) // LowerBody(151~194), Total(3~196)에서 151~194만 보간하기 위한 작업
					continue;

				// Src BoneIdx와 DstBoneIdx가 다르다면 건너 뛴다.
				if (pDstAnimCH->Get_BoneIdx() != iSrcBoneIdx)
					continue;

				// 끝과 처음을 보간하는 경우가 아닌 경우
				if (!bEndToFirstInterpolation)
				{
					// KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_FirstKeyFrame();
					KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_InterpolationKeyFrame(); // 250210
					m_Channels[i]->Update_TransformationMatrix_Interpolation(fInterTrackPos, fInterDis, &m_CurrentKeyFrameIndices[i], InterDstKeyFrame, Bones, bChainInterpolation);
				}
				// 끝과 처음을 보간하는 경우
				else
				{
					KEYFRAME InterSrcKeyFrame = m_Channels[i]->Get_LastKeyFrame();
					KEYFRAME InterDstKeyFrame = pDstAnimCH->Get_FirstKeyFrame();
					m_Channels[i]->Update_TransformationMatrix_Interpolation(fInterTrackPos, fInterDis, &m_CurrentKeyFrameIndices[i], InterSrcKeyFrame, InterDstKeyFrame, Bones, bChainInterpolation);
				}
			}
		}
	}

	return false;
}

_bool CAnimation::Update_TransformationMatrices(const vector<class CBone*>& Bones, _bool isLoop, _int iMinBoneIdx, _int iMaxBoneIdx, _float fTimeDelta, _int iAddtionalBoneIdx)
{
	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;
	Check_Events(m_fCurrentTrackPosition, fTimeDelta); // 250202

	_bool bEnd = false;

#ifdef _DEBUG

	static _float fAccAnimEnd = 0.f;
	fAccAnimEnd += fTimeDelta;

#endif

	if (true == isLoop && m_fCurrentTrackPosition > m_fDuration) // 루프 애니매이션인 경우
	{
		m_fCurrentTrackPosition = 0.f;
		bEnd = true;

#pragma region "애니매이션 재생이 끝남"

#ifdef _DEBUG

		{
			

			if (bEnd)
			{
				if (fAccAnimEnd > 2.f)
				{
					fAccAnimEnd = 0.f;
					
					cout << endl;
					cout << this->Get_AnimName() << " | 애니매이션 재생이 끝남" << endl;
					cout << endl;
				}
			}
		}


#endif


#pragma endregion

	}
	else if (m_fCurrentTrackPosition >= m_fDuration) // 루프 애니매이션이 아닌 경우 행렬 구성 X
	{
		m_fCurrentTrackPosition = m_fDuration;
		bEnd = true;

#pragma region "loop가 false(애니매이션을 멈춘다)이고 m_fDuration보다 큰 경우"

#ifdef _DEBUG

		static _float fAcc = 0.f;
		fAcc += fTimeDelta;
		if (fAcc >= 1.5f)
		{
			fAcc = 0.f;

			cout << "*************************************************************************" << endl;
			cout << endl;

			cout << this->Get_AnimName() << endl;
			cout << "루프가 꺼져있기 때문에(반복 재생을 한다는 옵션을 껏기 때문애) 애니매이션이 멈춰있습니다." << endl;
			cout << "(하체에 대해 루프를 false로 설정해놓은 경우 skl_root 뼈가 포함되어 있어서 애니매이션 이동, 회전등이 상쇄 됩니다. 굳어 있는 것 처럼 보임)" << endl;

			cout << endl;
			cout << "*************************************************************************" << endl;

		}

#endif
		

#pragma endregion

		return bEnd;
	}
	
	for (_int i = 0; i < m_iNumChannels; i++)
	{
		// 추가 본 인덱스는 바로 업데이트 해준다 : 루트본
		if (m_Channels[i]->Get_BoneIdx() == iAddtionalBoneIdx)
		{
			m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, &m_CurrentKeyFrameIndices[i], Bones);
			return bEnd;
		}

		// 채널이 가르키는 BoneIdx와 뭔가 다르다면 continue? 등의 작업
		// m_Channels[i]->Get_BoneIdx() < iMin || m_Channels[i]->Get_BoneIdx() > iMax
		// 위 의 경우에 행렬을 구성하지 않는다 continue
		if (m_Channels[i]->Get_BoneIdx() < iMinBoneIdx || m_Channels[i]->Get_BoneIdx() > iMaxBoneIdx)
			continue;

		m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, &m_CurrentKeyFrameIndices[i], Bones);
	}

	return bEnd;
}

void CAnimation::Check_Events(_float fCurTrackPos, _float fTimeDelta)
{
	for (auto iter = m_EventCallBacks.begin(); iter != m_EventCallBacks.end(); ++iter)
	{
		auto Events = iter->second;
		for (auto iterse = Events.begin(); iterse != Events.end(); ++iterse)
		{
			string strEventName = iterse->first;

			auto Pair = iterse->second;
			_float fEventTrackPos = Pair.first;
			function<void()> CallBack = Pair.second;

			_float fPrevTrackPos = fCurTrackPos - m_fTickPerSecond * fTimeDelta;
			if (fPrevTrackPos < fEventTrackPos && fCurTrackPos >= fEventTrackPos)
				CallBack();
		}
	}
}

HRESULT CAnimation::Bind_Event(string strLayerName, string strEventName, function<void()> CallBack)
{
	auto iter = m_EventLayer.find(strLayerName);
	if (iter == m_EventLayer.end())
		return E_FAIL;

	vector<pair<string, _float>> vecEvents = iter->second;
	auto iterFind = find_if(vecEvents.begin(), vecEvents.end(), [&strEventName](const auto& Pair) 
		{
			if (Pair.first == strEventName)
				return true;
			return false;
		}
	);

	if (iterFind == vecEvents.end())
		return E_FAIL;

	string strFindLayerName = iter->first;
	string strFindEventName = iterFind->first;
	_float fFindTrackPos = iterFind->second;

	m_EventCallBacks[strFindLayerName].emplace(strFindEventName, make_pair(fFindTrackPos, CallBack));

	return S_OK;
}


void CAnimation::Cal_DstKeyFrame(_float fAnimDuration, _float fDstTrackPos)
{
	for (_int i = 0; i < m_Channels.size(); ++i)
		m_Channels[i]->Cal_DstKeyFrame(fAnimDuration, fDstTrackPos);
}

// 250130
void CAnimation::Set_CurTrackPos(_float fCurTrackPos)
{
	m_fCurrentTrackPosition = fCurTrackPos;

	// 본들의 키프레임 인덱스들을 초기화 시켜준다.
	Reset_CurKeyFrameIndices();

	// 적절한 키프레임 인덱스를 강제 설정한다.
	// for (_int i = 0; i < m_iNumChannels; i++)
	// 	m_Channels[i]->Find_Suitable_KeyFrameIndices(m_fCurrentTrackPosition, &m_CurrentKeyFrameIndices[i], *m_vecBones);
}

void CAnimation::Reset_Inter_Anim()
{
	m_fCurInterTrackPos = 0.f;
}

// ======================================================================================================

CAnimation * CAnimation::Create(const aiAnimation * pAIAnimation, const CModel* pModel)
{
	CAnimation*	pInstance = new CAnimation();
		
	if (FAILED(pInstance->Initialize(pAIAnimation, pModel)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CAnimation * CAnimation::Clone()
{
	return new CAnimation(*this);
}

void CAnimation::Free()
{
	__super::Free();

	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);

	m_Channels.clear();
}


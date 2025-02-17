#include "..\Public\Model.h"
#include "GameInstance.h"
#include "Mesh.h"
#include "Shader.h"
#include "Bone.h" // 250208

// 250214
#include <iostream>
using namespace std;

CModel::CModel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent { pDevice, pContext }
{
}

CModel::CModel(const CModel & Prototype)
	: CComponent{ Prototype }
	, m_eModelType { Prototype.m_eModelType }
	, m_pAIScene { Prototype.m_pAIScene }	
	, m_iNumMeshes{ Prototype.m_iNumMeshes }
	, m_Meshes{ Prototype.m_Meshes }
	, m_iNumMaterials { Prototype.m_iNumMaterials }
	, m_Materials { Prototype.m_Materials }
	, m_PreTransformMatrix { Prototype.m_PreTransformMatrix }
	// , m_Bones { Prototype.m_Bones }
	, m_iNumAnimations { Prototype.m_iNumAnimations },
	// , m_Animations { Prototype.m_Animations }
	m_AnimationNames(Prototype.m_AnimationNames),
	m_InterInfos(Prototype.m_InterInfos), // 복사,
	m_fDefaultInterDis(Prototype.m_fDefaultInterDis), // copy
	m_fDefaultInterSP(Prototype.m_fDefaultInterSP) // copy
{
	for (auto& pPrototypeAnimation : Prototype.m_Animations)
		m_Animations.push_back(pPrototypeAnimation->Clone());

	for (auto& pPrototypeBone : Prototype.m_Bones)
		m_Bones.push_back(pPrototypeBone->Clone());

	for (auto& Textures : m_Materials)
		for (auto& SRVs : Textures)
			for (auto& pSRV : SRVs)
				Safe_AddRef(pSRV);
	
	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);

	for (_int i = _int(Abgt::LOWER_BODY); i <= _int(Abgt::TOTAL); ++i)
	{
		m_BlendGroups[i].DefaultInfo = {};
		m_BlendGroups[i].AnimInfo = {};

		memset(m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices, -1, sizeof m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices);
	}
}

_uint CModel::Get_BoneIndex(const _char * pBoneName) const
{
	_uint	iBoneIndex = { 0 };

	auto	iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool 
	{
		if (false == strcmp(pBone->Get_Name(), pBoneName))
			return true;

		++iBoneIndex;

		return false;
	});

	// if (iter == m_Bones.end())
	// 	MSG_BOX("없어");


	return iBoneIndex;
}

const _float4x4 * CModel::Get_BoneMatrix(const _char * pBoneName) const
{
	auto	iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool
	{
		if (false == strcmp(pBone->Get_Name(), pBoneName))
			return true;

		return false;
	});

	if (iter == m_Bones.end())
		return nullptr;

	return (*iter)->Get_CombinedTransformationFloat4x4();
}

HRESULT CModel::Initialize_Prototype(MODEL_TYPE eModelType, const _char * pModelFilePath, _fmatrix PreTransformMatrix)
{
	_uint		iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };

	if (eModelType == TYPE_NONANIM)
		iFlag |= aiProcess_PreTransformVertices;

	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (0 == m_pAIScene)
		return E_FAIL;

	m_eModelType = eModelType;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	if (FAILED(Ready_Bones(m_pAIScene->mRootNode, -1)))
		return E_FAIL;

	if (FAILED(Ready_Meshes()))
		return E_FAIL;

	if (FAILED(Ready_Materials(pModelFilePath)))
		return E_FAIL;

	if (FAILED(Ready_Animations()))
		return E_FAIL;

	return S_OK;
}

HRESULT CModel::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CModel::Render(_uint iMeshIndex)
{	
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	m_Meshes[iMeshIndex]->Bind_BufferDesc();
	m_Meshes[iMeshIndex]->Render();	

	return S_OK;
}

// 플레이어 전용 
void CModel::Play_Animation(_float fTimeDelta, _bool bPlayer)
{
	if (!m_bPlayAnim)
		return;

	Update_Interpolation_Animation(fTimeDelta, bPlayer);
	Update_Animation(fTimeDelta, bPlayer);

	Update_CheckAnim_End(bPlayer);
	Update_Bone_CombinedTransformationMatrix(bPlayer);
}

// 일반의 경우
_bool CModel::Play_Animation(_float fTimeDelta)
{
	// AnimTool에서 막은 경우 끝나지 않았고 행렬 업데이트를 하지 않도록 막는다. (AnimTool 에서 Stop || Play CurAnim을 호출하지 않는 이상 막힐 경우는 없습니다.)
	if (!m_bPlayAnim)
		return false;

	// A -> B보간하는 경우 A 애니매이션에서 보간을 수행한다. 보간중일때 까지는 Get_CurAnim 호출시 A애니매이션 포인터가 반환된다.
	if (m_bInterpolationing)
	{
		if (Update_Interpolation_Animation(fTimeDelta)) // 보간이 끝나면 true를 반환
		{
			Finialize_Interpolation(); // 보간이 끝났을 때의 작업들
			Update_Bone_CombinedTransformationMatrix(); // 실제 뼈들의 최종행렬 연산
			return false;
		}
	}
	else
		m_isFinished = Update_Animation(fTimeDelta);

	Update_CheckAnim_End(); // 매틱 애니매이션 재생이 끝났는지 확인
	Update_Bone_CombinedTransformationMatrix(); // 실제 뼈들의 최종행렬 연산
	return m_isFinished;
}

CAnimation* CModel::Get_CurAnim()
{
	if (0 <= m_iCurrentAnimIndex && m_iCurrentAnimIndex < m_iNumAnimations)
		if (m_Animations[m_iCurrentAnimIndex])
			return m_Animations[m_iCurrentAnimIndex];
	return nullptr;
}


HRESULT CModel::Load_Anim_InterXML(const WCHAR* strXMLFilePath)
{
	if (!m_InterInfos.empty())
		m_InterInfos.clear();

	// Read XML 파일 해준다.
	m_InterInfos.resize(m_iNumAnimations); // 애니매이션 개수 만큼 보간 정보 벡터를 resize해준다.
	
	// 루트 요소인 <AnimTransitions> 추가
	xml::XMLDocument doc;

	if (doc.LoadFile(CONVERTTOSTRING(strXMLFilePath).c_str()) != xml::XML_SUCCESS)
	{
#ifdef _DEBUG
		MSG_BOX("CModel::Load_Anim_InterXML Load Fail XML");
#endif
		return E_FAIL;
	}

	_int iAnimCnt = 0;
	_float fDefaultInterDis, fDefaultInterSP;

	xml::XMLElement* pReadAnimaInterpolationDataNode = doc.FirstChildElement("AnimationInterpolationData"); // 루트 노드
	if (!pReadAnimaInterpolationDataNode)
	{
		return E_FAIL;
	}

	pReadAnimaInterpolationDataNode->FindAttribute("AnimationCount")->QueryIntValue(&iAnimCnt);

	if (!iAnimCnt)
	{
#ifdef _DEBUG
		MSG_BOX("CModel::Load_Anim_InterXML Load Fail XML : iAnimCnt == 0");
#endif	
		return E_FAIL;
	}

	pReadAnimaInterpolationDataNode->FindAttribute("DefaultInterPolationDis")->QueryFloatValue(&fDefaultInterDis);
	pReadAnimaInterpolationDataNode->FindAttribute("DefaultInterpolationSP")->QueryFloatValue(&fDefaultInterSP);

	// 기본 거리, 속도 저장
	m_fDefaultInterDis = fDefaultInterDis;
	m_fDefaultInterSP = fDefaultInterSP;

	if (fDefaultInterDis <= 0.f || fDefaultInterSP <= 0.f)
	{
#ifdef _DEBUG
		MSG_BOX("CModel::Load_Anim_InterXML Load Fail XML fDefaultInterDis fDefaultInterSP Error Load Fail");
#endif
		return E_FAIL;
	}

	// 애니메이션 노드 읽기
	xml::XMLElement* pAnimNode = pReadAnimaInterpolationDataNode->FirstChildElement("Animation");
	if (!pAnimNode)
		return E_FAIL;

	for (_int i = 0; i < iAnimCnt; ++i)
	{
		if (!pAnimNode)
		{
#ifdef _DEBUG
			MSG_BOX("CModel::Load_Anim_InterXML Load XML Fail");
#endif
			return E_FAIL;
		}

		const _char* pAnimName = pAnimNode->Attribute("Name");
		
		// InterpolationInfo 노드 읽기
		xml::XMLElement* pInterpolationInfoNode = pAnimNode->FirstChildElement("InterpolationInfo");
		if (!pInterpolationInfoNode)
			return E_FAIL;

		while (pInterpolationInfoNode)
		{
			const _char* srcAnim = pInterpolationInfoNode->Attribute("SrcAnim");
			const _char* dstAnim = pInterpolationInfoNode->Attribute("DstAnim");

			_int iSrcIdx, iDstIdx;
			pInterpolationInfoNode->QueryIntAttribute("SrcAnimIdx", &iSrcIdx);
			pInterpolationInfoNode->QueryIntAttribute("DstAnimIdx", &iDstIdx);

			_float interDis = 0.f, interSP = 0.f;

			pInterpolationInfoNode->QueryFloatAttribute("InterpolationDistance", &interDis);
			pInterpolationInfoNode->QueryFloatAttribute("InterpolationSpeed", &interSP);

			// 값이 다른 노드가 있다면
			if ((fabs(interDis - fDefaultInterDis) > 0.01f) || (fabs(interSP - fDefaultInterSP) > 0.01f))
				m_InterInfos[i].insert({ iDstIdx, {interDis, interSP} });

			pInterpolationInfoNode = pInterpolationInfoNode->NextSiblingElement("InterpolationInfo");
		}

		// 다음 애니메이션으로 이동
		pAnimNode = pAnimNode->NextSiblingElement("Animation");
	}

	m_bLoadedInterInfo = true;
	return S_OK;
}

HRESULT CModel::Load_Anim_RootMotionBoneXML(const WCHAR* strXMLFilePath)
{
	xml::XMLDocument doc;
	if (doc.LoadFile(CONVERTTOSTRING(strXMLFilePath).c_str()) != xml::XML_SUCCESS)
		return E_FAIL;

	xml::XMLElement* pRootEle = doc.FirstChildElement("RootMotionBoneName");
	if (!pRootEle)
		return E_FAIL;

	const _char* pstrName = nullptr;
	if (pRootEle->QueryAttribute("Name", &pstrName) != xml::XML_SUCCESS)
		return E_FAIL;

	m_strRootMotionBoneName = pstrName; // 속성 값을 std::string으로 저장
	return S_OK;
}

HRESULT CModel::Load_BlendBones(const WCHAR* strXMLFilePath)
{
	xml::XMLDocument Doc;

	string stRetFilePath = CONVERTTOSTRING(strXMLFilePath);

	if (Doc.LoadFile(stRetFilePath.c_str()) != xml::XML_SUCCESS)
		return E_FAIL;

	xml::XMLElement* pStartNode = Doc.FirstChildElement("BlendBones"); // 루트 노드
	if (!pStartNode)
		return E_FAIL;

	_int iNumBlendBone = 0;
	pStartNode->QueryIntAttribute("Num", &iNumBlendBone);

	if (!iNumBlendBone)
		return E_FAIL;

	vector<string> vecBlendBoneNames; vecBlendBoneNames.resize(iNumBlendBone);
	vector<_int> vecBlendBoneindces; vecBlendBoneindces.resize(iNumBlendBone);
	vector<_int> vecBlendBoneTypes; vecBlendBoneTypes.resize(iNumBlendBone);

	xml::XMLElement* pBlendBoneNode = pStartNode->FirstChildElement("BlendBone");
	if (!pBlendBoneNode)
		return E_FAIL;

	for (_int i = 0; i < iNumBlendBone; ++i)
	{
		vecBlendBoneNames[i] = pBlendBoneNode->FindAttribute("BlendBoneName")->Value();
		vecBlendBoneindces[i] = pBlendBoneNode->FindAttribute("BlendBoneIndex")->IntValue();
		vecBlendBoneTypes[i] = pBlendBoneNode->FindAttribute("BlendBoneType")->IntValue();
		pBlendBoneNode = pBlendBoneNode->NextSiblingElement();
	}

	vector<BONE_XML> vecBlendBoneGroup;
	xml::XMLElement* pNextNode = pStartNode->NextSiblingElement();
	if (!pNextNode)
		return E_FAIL;

	for (_int i = 0; i < iNumBlendBone; ++i)
	{

		string strBlendBoneParentName = pNextNode->FindAttribute("Name")->Value();
		string strBlendBoneType = pNextNode->FindAttribute("Type")->Value();
		_int iBlendBoneMinIdx = pNextNode->FindAttribute("MinBoneIdx")->IntValue();
		_int iBlendBoneMaxIdx = pNextNode->FindAttribute("MaxBoneIdx")->IntValue();

		Abgt eBlendGroupType = Abgt::BLEND_END;
		if (strBlendBoneType == "LowerBody")
			eBlendGroupType = Abgt::LOWER_BODY;
		else if (strBlendBoneType == "UpperBody")
			eBlendGroupType = Abgt::UPPER_BODY;
		else if (strBlendBoneType == "ArmLeft")
			eBlendGroupType = Abgt::ARM_LEFT;
		else if (strBlendBoneType == "ArmRight")
			eBlendGroupType = Abgt::ARM_RIGHT;
		else if (strBlendBoneType == "Head")
			eBlendGroupType = Abgt::HEAD;
		else if (strBlendBoneType == "Total")
			eBlendGroupType = Abgt::TOTAL;
		else if (eBlendGroupType == Abgt::BLEND_END)
			return E_FAIL;

		// Blend Bone의 기본 초기값 설정해준다.
		Init_BlendGroup_DefaultInfo(eBlendGroupType, CModel::DEFAULT_INFO{ strBlendBoneParentName, eBlendGroupType, iBlendBoneMinIdx, iBlendBoneMaxIdx });

		pNextNode = pNextNode->NextSiblingElement();
	}

	return S_OK;
}

HRESULT CModel::Load_AnimEvents(const WCHAR* strXMLFilePath)
{
	xml::XMLDocument Doc;
	string stRetFilePath = CONVERTTOSTRING(strXMLFilePath);
	if (Doc.LoadFile(stRetFilePath.c_str()) != xml::XML_SUCCESS)
		return E_FAIL;

	xml::XMLElement* pRoot = Doc.FirstChildElement("AnimationEvent"); // 루트 노드
	if (!pRoot)
		return E_FAIL;

	_int iNumAnim = 0;
	pRoot->QueryIntAttribute("NumAnim", &iNumAnim);
	if (!iNumAnim) // 0개면 실패
		return E_FAIL;

	xml::XMLElement* pAnimEle = pRoot->FirstChildElement("Animation");
	for (_int i = 0; i < iNumAnim; ++i)
	{
		if (!pAnimEle)
			return E_FAIL;

		string strAnimName{};
		_int iNumLayer{};
		const _char* pstrAnimName = nullptr;

		pAnimEle->QueryAttribute("Name", &pstrAnimName);
		strAnimName = pstrAnimName;
		pAnimEle->QueryIntAttribute("NumLayer", &iNumLayer);

		xml::XMLElement* pLayerEle = pAnimEle->FirstChildElement("Layer");
		for (_int j = 0; j < iNumLayer; ++j)
		{
			const _char* pstrLayerName = nullptr;
			pLayerEle->QueryAttribute("Name", &pstrLayerName);
			
			string strLayerName{};
			_int iNumEvent{};

			strLayerName = pstrLayerName;
			pLayerEle->QueryIntAttribute("NumEvent", &iNumEvent);


			xml::XMLElement* pEventEle = pLayerEle->FirstChildElement("Event"); // Event라는 이름의 노드 찾는다.
			for (_int k = 0; k < iNumEvent; ++k)
			{
				const _char* pstrEventName = nullptr;
				pEventEle->QueryAttribute("Name", &pstrEventName);

				string strEventName = pstrEventName;
				_float fTrackPos{};
				
				pEventEle->QueryFloatAttribute("TrackPos", &fTrackPos);

				m_Events[strAnimName][strLayerName].emplace_back(make_pair(strEventName, fTrackPos));

				pEventEle = pEventEle->NextSiblingElement();
			}

			pLayerEle = pLayerEle->NextSiblingElement();
		}

		pAnimEle = pAnimEle->NextSiblingElement();
	}

	if (Bind_AnimEvents() != S_OK)
		return E_FAIL;

	return S_OK;
}

HRESULT CModel::Bind_AnimEvents()
{
	for (auto iter = m_Events.begin(); iter != m_Events.end(); ++iter)
	{
		string strAnimName = iter->first;
		map<string, vector<pair<string, _float>>> LayerEvents = iter->second;
		
		_int iAnimIdx = Get_AnimIdx(strAnimName);
		if (iAnimIdx == -1)
			return E_FAIL;

		CAnimation* pAnim = Get_Anim(iAnimIdx);
		if (!pAnim)
			return E_FAIL;

		for (auto iterse = LayerEvents.begin(); iterse != LayerEvents.end(); ++iterse)
		{
			string strLayerName = iterse->first;
			auto vecEvents = iterse->second;
			for (_int i = 0; i < vecEvents.size(); ++i)
				pAnim->Add_Event(strLayerName, vecEvents[i].first, vecEvents[i].second);
		}
	}

	return S_OK;
}


void CModel::Init_BlendGroup_DefaultInfo(Abgt eBlendGroupType, DEFAULT_INFO DefaultInfo)
{
	m_BlendGroups[_int(eBlendGroupType)].DefaultInfo = DefaultInfo;
}

void CModel::Init_BlendGroup_AnimInfo(Abgt eBlendGroupType, ANIM_INFO AnimInfo)
{
	if (m_BlendGroups[_int(eBlendGroupType)].AnimInfo.strAnimName == AnimInfo.strAnimName)
		return;
	if (m_BlendGroups[_int(eBlendGroupType)].AnimInfo.pAnim)
	{
		m_BlendGroups[_int(eBlendGroupType)].AnimInfo.pAnim->Reset_CurKeyFrameIndices();
		m_BlendGroups[_int(eBlendGroupType)].AnimInfo.pAnim->Reset_Inter_Anim();
		m_BlendGroups[_int(eBlendGroupType)].AnimInfo.pAnim->Reset_TrackPos();
	}
	if (AnimInfo.pAnim)
	{
		AnimInfo.pAnim->Reset_CurKeyFrameIndices();
		AnimInfo.pAnim->Reset_Inter_Anim();
		AnimInfo.pAnim->Reset_TrackPos();
	}
	m_BlendGroups[_int(eBlendGroupType)].AnimInfo = AnimInfo;
}

void CModel::Init_BlendGroup_InterpolationInfo(Abgt eBlendGroupType, INTERPOLATION_INFO InterpolationInfo)
{
	m_BlendGroups[_int(eBlendGroupType)].InterpolationInfo = InterpolationInfo;
 }

void CModel::SetUp_Animation(string strAnimName, _bool isLoop)
{
	for (_int i = 0; i < m_iNumAnimations; ++i)
		if (!strcmp(m_Animations[i]->Get_AnimName(), strAnimName.c_str()))
			m_iCurrentAnimIndex = i;
	m_isLoop = isLoop;
}

_bool CModel::Update_Interpolation_Animation(_float fTimeDelta)
{
	_bool bEnd = m_Animations[m_iCurrentAnimIndex]->Update_Inter_TransformationMatrices(m_Animations[m_iInterDstAnimIdx], m_fInterDis, m_fInterSP, m_Bones, fTimeDelta, m_fInterTrackPos, m_bChainInterpolation, m_bEndToFirst_Interpolation);
	return bEnd;
}

void CModel::Finialize_Interpolation()
{
	Set_Interpolationing(false);				// 보간중이 아니라고 설정한다.
	Reset_Anim(m_iInterDstAnimIdx);
	m_iCurrentAnimIndex = m_iInterDstAnimIdx;	// 보간이 끝났으니 현재 재생 애니매이션을 변경한다.
	Reset_Anim(m_iCurrentAnimIndex);
	m_iInterDstAnimIdx = -1;

	if (m_bChainInterpolation)
		m_bChainInterpolation = false;
}

_bool CModel::Update_Animation(_float fTimeDelta)
{
	/* 표현하고자 하는 애니메이션에 따른, 뼈의 상태(TransformationMatrix)를 갱신해준다.  */
	_bool bRet = m_Animations[m_iCurrentAnimIndex]->Update_TransformationMatrices(m_Bones, m_isLoop, fTimeDelta);
	return bRet;
}

void CModel::Update_CheckAnim_End()
{
	// 루프 애니매이션이든 단일 재생 애니매이션이든 현재 재생중인 애니매이션이 끝났는지 확인한다.
	m_bCurPlayAnimEnd = m_Animations[m_iCurrentAnimIndex]->Get_IsPlayingAnimEnd();
}

void CModel::Update_Bone_CombinedTransformationMatrix()
{
	/* 뼈의 바뀐 TransformationMatrix를 활용하여 렌덜이하기위한 최종 뼈의 CombinedTransformationMatrix를 만들어낸다. */
	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix), this);
}

_bool CModel::Update_Interpolation_Animation(_float fTimeDelta, _bool bPlayer)
{
	for (_int iSrc = iLower; iSrc <= iTotal; ++iSrc)
	{
		// i번째 그룹 애니매이션이 보간을 해야한다면
		if (!m_BlendGroups[iSrc].AnimInfo.bUse && m_BlendGroups[iSrc].AnimInfo.bInterpolationing)
		{
			for (_int iDst = (_int)Abgt::LOWER_BODY; iDst < (_int)Abgt::BLEND_END; ++iDst)
			{
				_int iDstAnimIdx = m_BlendGroups[iSrc].InterpolationInfo.InterpolationDstGroupIdices[iDst];
				if (iDstAnimIdx != -1)
				{
					_int iDstMinBoneIdx = m_BlendGroups[iDst].DefaultInfo.iMinBoneIdx;
					_int iDstMaxBoneIdx = m_BlendGroups[iDst].DefaultInfo.iMaxBoneIdx;

					_float fInterSP = m_BlendGroups[iSrc].InterpolationInfo.fInterpolationSP;
					_float fInterDis = m_BlendGroups[iSrc].InterpolationInfo.fInterpolationDis;

					// 하체에서 어디론가 갈 때
					if (m_BlendGroups[iSrc].DefaultInfo.eBlendGroupType == Abgt::LOWER_BODY)
					{
						_bool bInterpolationEachEnd = m_BlendGroups[iSrc].AnimInfo.pAnim->Update_Inter_TransformationMatrices(m_Animations[iDstAnimIdx], fInterDis, fInterSP, m_Bones, fTimeDelta, m_BlendGroups[iSrc].AnimInfo.bEnd, m_BlendGroups[iSrc].DefaultInfo.iMinBoneIdx, m_BlendGroups[iSrc].DefaultInfo.iMaxBoneIdx, iDstMinBoneIdx, iDstMaxBoneIdx, m_BlendGroups[iSrc].InterpolationInfo.bChainInterpolation, m_BlendGroups[iSrc].InterpolationInfo.fInterpolationCurTrackPos, 3);

						if (bInterpolationEachEnd)
							Finalize_Interpolation(m_BlendGroups[iSrc].DefaultInfo.eBlendGroupType, m_BlendGroups[iSrc].AnimInfo.iAnimIdx, Abgt(iDst), iDstAnimIdx);
					}
					// 어딘가에서 하체로 때
					else if (Abgt(iDst) == Abgt::LOWER_BODY)
					{
						_bool bInterpolationEachEnd = m_BlendGroups[iSrc].AnimInfo.pAnim->Update_Inter_TransformationMatrices(m_Animations[iDstAnimIdx], fInterDis, fInterSP, m_Bones, fTimeDelta, m_BlendGroups[iSrc].AnimInfo.bEnd, m_BlendGroups[iSrc].DefaultInfo.iMinBoneIdx, m_BlendGroups[iSrc].DefaultInfo.iMaxBoneIdx, iDstMinBoneIdx, iDstMaxBoneIdx, m_BlendGroups[iSrc].InterpolationInfo.bChainInterpolation, m_BlendGroups[iSrc].InterpolationInfo.fInterpolationCurTrackPos, 3);

						if (bInterpolationEachEnd)
							Finalize_Interpolation(m_BlendGroups[iSrc].DefaultInfo.eBlendGroupType, m_BlendGroups[iSrc].AnimInfo.iAnimIdx, Abgt(iDst), iDstAnimIdx);
					}
					else
					{
						_bool bInterpolationEachEnd = m_BlendGroups[iSrc].AnimInfo.pAnim->Update_Inter_TransformationMatrices(m_Animations[iDstAnimIdx], fInterDis, fInterSP, m_Bones, fTimeDelta, m_BlendGroups[iSrc].AnimInfo.bEnd, m_BlendGroups[iSrc].DefaultInfo.iMinBoneIdx, m_BlendGroups[iSrc].DefaultInfo.iMaxBoneIdx, iDstMinBoneIdx, iDstMaxBoneIdx, m_BlendGroups[iSrc].InterpolationInfo.bChainInterpolation, m_BlendGroups[iSrc].InterpolationInfo.fInterpolationCurTrackPos);

						if (bInterpolationEachEnd)
							Finalize_Interpolation(m_BlendGroups[iSrc].DefaultInfo.eBlendGroupType, m_BlendGroups[iSrc].AnimInfo.iAnimIdx, Abgt(iDst), iDstAnimIdx);
					}
				}
			}
		}
	}

	Finalize_Interpolation_Exception();

	return false;
}

void CModel::Finialize_Interpolation(_bool bPlayer)
{

}

void CModel::Update_Animation(_float fTimeDelta, _bool bPlayer)
{
	for (_int i = (_int)Abgt::LOWER_BODY; i < (_int)Abgt::BLEND_END; ++i)
	{
		if (!m_BlendGroups[i].AnimInfo.bUpdate) // 250214
			continue;

		if (m_BlendGroups[i].AnimInfo.bUse && !m_BlendGroups[i].AnimInfo.bInterpolationing && m_BlendGroups[i].AnimInfo.pAnim && m_BlendGroups[i].AnimInfo.iAnimIdx != -1) // 사용중인 BlendGroup 애니매이션 업데이트 갈긴다.
		{
			_int iMinBoneIdx = m_BlendGroups[i].DefaultInfo.iMinBoneIdx;
			_int iMaxBoneIdx = m_BlendGroups[i].DefaultInfo.iMaxBoneIdx;

			if (m_BlendGroups[i].DefaultInfo.eBlendGroupType == Abgt::LOWER_BODY)
				m_BlendGroups[i].AnimInfo.bEnd = m_BlendGroups[i].AnimInfo.pAnim->Update_TransformationMatrices(m_Bones, m_BlendGroups[i].AnimInfo.bIsLoop, iMinBoneIdx, iMaxBoneIdx, fTimeDelta, 3);
			else
				m_BlendGroups[i].AnimInfo.bEnd = m_BlendGroups[i].AnimInfo.pAnim->Update_TransformationMatrices(m_Bones, m_BlendGroups[i].AnimInfo.bIsLoop, iMinBoneIdx, iMaxBoneIdx, fTimeDelta);
		}
	}
}

void CModel::Update_CheckAnim_End(_bool bPlayer)
{
}

void CModel::Update_Bone_CombinedTransformationMatrix(_bool bPlayer)
{
	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix), this);
}

void CModel::Reset_BlendGroupBone(Abgt eGroupType)
{
	m_BlendGroups[(Abgti)eGroupType].AnimInfo.strAnimName = "";
	m_BlendGroups[(Abgti)eGroupType].AnimInfo.iAnimIdx = -1;
	m_BlendGroups[(Abgti)eGroupType].AnimInfo.bUse = false;
	m_BlendGroups[(Abgti)eGroupType].AnimInfo.bEnd = false;

	if (m_BlendGroups[(Abgti)eGroupType].AnimInfo.pAnim)
	{
		m_BlendGroups[(Abgti)eGroupType].AnimInfo.pAnim->Reset_CurKeyFrameIndices();
		m_BlendGroups[(Abgti)eGroupType].AnimInfo.pAnim->Reset_TrackPos();
		m_BlendGroups[(Abgti)eGroupType].AnimInfo.pAnim = nullptr;
	}
}

vector<pair<Abgt, _int>> CModel::Get_AnimEndGroup()
{
	vector<pair<Abgt, _int>> vecEndGroup;

	for (_int i = (_int)Abgt::LOWER_BODY; i <= (_int)Abgt::TOTAL; ++i)
	{
		if (m_BlendGroups[i].AnimInfo.bUse && m_BlendGroups[i].AnimInfo.bEnd)
			vecEndGroup.push_back(make_pair( m_BlendGroups[i].DefaultInfo.eBlendGroupType, m_BlendGroups[i].AnimInfo.iAnimIdx) );
	}
	
	return vecEndGroup;
}


// eSrcInterpolationGroup 그룹이 애니매이션 보간이 끝났을 때 호출하는 함수이다.
// A에서 보간을 시작해서 보간이 끝난 경우 eSrcInterpolationGroup는 A애니매이션이 속한 그룹 iSrcInterpolationAnimIdx는 A애니매이션 인덱스이다.
// Lower, Upper -> Total, Total -> Lower, Upper 등의 경우가 있다.
void CModel::Finalize_Interpolation(Abgt eSrcInterpolationGroup, _int iSrcInterpolationAnimIdx, Abgt eDstStartAnimGroup, _int iDstAnimIdx)
{
	// 현재 보간 시작 그룹이 TOTAL인 경우
	if (eSrcInterpolationGroup == Abgt::TOTAL)
	{
		// TOTAL -> TOTAL의 보간이였던 경우
		if (eDstStartAnimGroup == Abgt::TOTAL)
		{
			Finalize_Interpolation_When_TotalToTotal(eSrcInterpolationGroup, eDstStartAnimGroup);
		}
		// TOTAL -> TOTAL 보간이 아니였던 경우 TOTAL -> LOWER & UPPER 이런 경우
		else
		{
			Finalize_Interpolation_When_TotalToNotTotal(eSrcInterpolationGroup, eDstStartAnimGroup);
		}
	}
	// 현재 보간 시작 그룹이 TOTAL이 아닌 경우
	else
	{
		// 보간 시작 그룹이 TOTAL이 아닌 경우 현재 시작 그룹 애니매이션 정보, 애니매이션 보간 정보 초기화,
		// 보간 끝 그룹 애니매이션 초기화, 애니매이션 보간 정보 초기화를 하고 그래도 목적지 그룹에 토스한다.
		Finalize_Interpolation_When_NotTotalTo(eSrcInterpolationGroup, eDstStartAnimGroup);
	}
}

// TOTAL에서 시작한 경우가 아닌 경우
void CModel::Finalize_Interpolation_When_NotTotalTo(Abgt eSrcInterpolationGroup, Abgt eDstStartAnimGroup)
{
	// 보간의 끝인 목적지 그룹 인덱스
	_int iDstEndInterpolationGroupIdx = (_int)eDstStartAnimGroup;
	_int iDstAnimIdx = m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.InterpolationDstGroupIdices[iDstEndInterpolationGroupIdx];

	CAnimation* pCurAnim = m_Animations[iDstAnimIdx];
	CAnimation* pSrcAnim = nullptr;

	if (!pCurAnim)
	{
		assert(false, "pCurAnim이 nullptr이다");
	}

	// 현재 그룹 Src(보간 시작점 애니매이션)의 애니매이션 정보 및 보간 데이터를 초기화 해준다.
	// 아래 경우에서 Src처리를 해준다.
	// [A][X][X][X][X] : Src
	// [X][X][X][X][B] : Dst
	{
		if ((_int)eDstStartAnimGroup != iTotal)
		{
			m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim->Reset_CurKeyFrameIndices();
			m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim->Reset_Inter_Anim();
			m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim->Reset_TrackPos();
		}

		if (m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.bEnd)
			m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.bEnd = false;

		if (m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.bUse)
			m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.bUse = false;

		if (m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.iAnimIdx != -1)
			m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.iAnimIdx = -1;

		m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.strAnimName = "";

		// 현재 그룹의 보간 거리를 초기화 해준다.
		m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
		m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.bInterpolationing = false;

		// 현재 그룹이 보보간 중이였다면 초기화 해준다
		if (m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.bChainInterpolation)
		{
			m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.bChainInterpolation = false;
			m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim->Reset_ChainInterpolation_Bones_SRT();
		}

		pSrcAnim = m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim;
		m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim = nullptr;

		Reset_InterpolationDstIndices(eSrcInterpolationGroup);
	}

	// 어떤 그룹(상체 or 하체 or 팔 or 머리)에서 Total로의 보간이 끝난 경우
	// [A][X][X][X][X] : Src
	// [X][X][X][X][B] : Dst
	if (eDstStartAnimGroup == Abgt::TOTAL)
	{
		// 사용중인 그룹이 있다면 전부다 false로 만든다.
		for (_int i = iLower; i <= iTotal; ++i)
		{
			if (i == _int(eSrcInterpolationGroup)) 
				continue;

			if (m_BlendGroups[i].AnimInfo.bUse)
			{
				m_BlendGroups[i].AnimInfo.bUse = false;

				if (m_BlendGroups[i].AnimInfo.bEnd)
					m_BlendGroups[i].AnimInfo.bEnd = false;

				if (m_BlendGroups[i].AnimInfo.bInterpolationing)
					m_BlendGroups[i].AnimInfo.bInterpolationing = false;

				if (m_BlendGroups[i].AnimInfo.iAnimIdx != -1)
					m_BlendGroups[i].AnimInfo.iAnimIdx = -1;
				
				if (m_BlendGroups[i].AnimInfo.pAnim)
					m_BlendGroups[i].AnimInfo.pAnim = nullptr;

				if (m_BlendGroups[i].AnimInfo.strAnimName != "")
					m_BlendGroups[i].AnimInfo.strAnimName = "";

				if (m_BlendGroups[i].InterpolationInfo.bChainInterpolation)
					m_BlendGroups[i].InterpolationInfo.bChainInterpolation = false;

				m_BlendGroups[i].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
				
				Reset_InterpolationDstIndices(Abgt(i));
			}
		}

		m_BlendGroups[iTotal].AnimInfo.bUse = true;
		m_BlendGroups[iTotal].AnimInfo.iAnimIdx = iDstAnimIdx;
		m_BlendGroups[iTotal].AnimInfo.pAnim = pCurAnim;
		m_BlendGroups[iTotal].AnimInfo.strAnimName = pCurAnim->Get_AnimName();

		m_BlendGroups[iTotal].AnimInfo.bInterpolationing = false;
		m_BlendGroups[iTotal].InterpolationInfo.bChainInterpolation = false;
		
		m_BlendGroups[iTotal].InterpolationInfo.fInterpolationCurTrackPos = 0.f; // 250215
		Reset_InterpolationDstIndices(Abgt::TOTAL);

		// 보간이 끝나고 새롭게 재생할 애니매이션 TrackPos(시작 위치)를 맞춘다.
		_float fDstAnimDurtaion = m_BlendGroups[iTotal].AnimInfo.pAnim->Get_AnimDuration();
		_float fSrcInterpolationDis = m_BlendGroups[(_int)eSrcInterpolationGroup].InterpolationInfo.fInterpolationDis;
		Adjust_TrackPos_When_EndInterpolation(m_BlendGroups[iTotal].AnimInfo.pAnim, fDstAnimDurtaion, fSrcInterpolationDis);
	}
	// [A][X][X][X][X] : Src
	// [B][X][X][X][X] : Dst
	else
	{
		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.pAnim = m_Animations[iDstAnimIdx];
		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.iAnimIdx = iDstAnimIdx;
		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.strAnimName = m_Animations[iDstAnimIdx]->Get_AnimName();

		// 보간이 끝나고 이제 시작할 그룹의 pAnim에 현재 보간 시작점의 애니매이션을 넘겨준다.
		if (!m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bUse)
			m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bUse = true; // 사용한다고 설정

		if (m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bEnd)
			m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bEnd = false;

		if (m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bInterpolationing)
			m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bInterpolationing = false; // 보간 중이 아니라고 설정

		// iDstEndInterpolationGroupIdx 그룹 보간 정보 초기화 해준다.
		if (m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].InterpolationInfo.bChainInterpolation)
			m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].InterpolationInfo.bChainInterpolation = false;

		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

		// 보간 목적지 싹다 -1로 채워준다.
		Reset_InterpolationDstIndices(eDstStartAnimGroup);


		// 보간이 끝나고 새롭게 재생할 애니매이션 TrackPos(시작 위치)를 맞춘다.
		_float fDstAnimDurtaion = m_BlendGroups[iDstEndInterpolationGroupIdx].AnimInfo.pAnim->Get_AnimDuration();
		_float fSrcInterpolationDis = m_BlendGroups[(_int)eSrcInterpolationGroup].InterpolationInfo.fInterpolationDis;
		Adjust_TrackPos_When_EndInterpolation(m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.pAnim, fDstAnimDurtaion, fSrcInterpolationDis);
	}
}

// Total -> Total의 보간의 끝인 경우 호출되는 함수
void CModel::Finalize_Interpolation_When_TotalToTotal(Abgt eSrcInterpolationGroup, Abgt eDstStartAnimGroup)
{
#pragma region "Finalize_Interpolation_When_TotalToTotal"

#ifdef _DEBUG

	cout << AnimTypeStr[_int(eSrcInterpolationGroup)] << " 에서 " << AnimTypeStr[_int(eDstStartAnimGroup)] << " 으로의 보간이 끝났습니다." << endl;

#endif

#pragma endregion





	if (eSrcInterpolationGroup != Abgt::TOTAL && eDstStartAnimGroup != Abgt::TOTAL)
	{
		assert(false, "Total to Total의 보간이 끝이 아닙니다");
	}
	
	// 보간의 끝인 목적지 그룹 인덱스
	_int iDstEndInterpolationGroupIdx = (_int)eDstStartAnimGroup;

	// 현재 그룹 TOTAL의 애니매이션 정보 및 보간 데이터를 초기화 해준다.
	{
		m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim->Reset_CurKeyFrameIndices();
		m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim->Reset_Inter_Anim();
		m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim->Reset_TrackPos();

		if (m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.bEnd)
			m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.bEnd = false;

		if (m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.bInterpolationing)
			m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.bInterpolationing = false;
	}


	// 보간이 끝나고 이제 시작할 그룹의 pAnim에 현재 보간 시작점의 애니매이션을 넘겨준다.
	// iNowAnimIdx : 보간이 끝나고 난 후 다음 애니매이션 인덱스이다.
	_int iNowAnimIdx = m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.InterpolationDstGroupIdices[iDstEndInterpolationGroupIdx];

	// 목적지 그룹(TOTAL) 초기화 작업
	{
		// 애니매이션 포인터를 얻어온다.
		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.pAnim = m_Animations[iNowAnimIdx];

		// TOTAL -> TOTAL이라 하더라도 보간을 진행하였기 때문에 bUse가 false였을 것이기 때문에 다시 true로 설정해준다.
		if (!m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bUse)
			m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bUse = true; // 사용한다고 설정
		else
		{
			assert(false, "Finalize_Interpolation_When_TotalToTotal Error");
		}

		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bInterpolationing = false;								// 보간 중이 아니라고 설정
		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.iAnimIdx = iNowAnimIdx;									// 애니매이션 인덱스 설정
		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.strAnimName = m_Animations[iNowAnimIdx]->Get_AnimName(); // 애니매이션 이름 설정
		if (m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bEnd)												// 끝나있던 애니매이션이였던 경우 다시 false로 해준다.
			m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].AnimInfo.bEnd = false;

		// 목적지 TOTAL 그룹 보간 정보 초기화 해준다.
		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].InterpolationInfo.bChainInterpolation = false;
		m_BlendGroups[(_int)iDstEndInterpolationGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

		// iDstEndInterpolationGroupIdx 그룹의 보간 목적지 싹다 -1로 채워준다.
		Reset_InterpolationDstIndices(Abgt(iDstEndInterpolationGroupIdx));


		// 보간이 끝나고 새롭게 재생할 애니매이션 TrackPos(시작 위치)를 맞춘다.
		_float fDstAnimDurtaion = m_BlendGroups[(_int)eDstStartAnimGroup].AnimInfo.pAnim->Get_AnimDuration();
		_float fSrcInterpolationDis = m_BlendGroups[(_int)eSrcInterpolationGroup].InterpolationInfo.fInterpolationDis;
		Adjust_TrackPos_When_EndInterpolation(m_BlendGroups[iDstEndInterpolationGroupIdx].AnimInfo.pAnim, fDstAnimDurtaion, fSrcInterpolationDis);
	}

	// 현재 그룹의 보간 목적지 그룹을 -1로 초기화 해준다.
	// TOTAL이 아닌 경우 호출되는 함수라 fill로 전부 -1로 채우든 특정 목적지 그룹애니매이션 인덱스 하나만을 -1로 채우든 결과는 같을 것이라 생각이 된다
	Reset_InterpolationDstIndices(eSrcInterpolationGroup);

	// 현재 그룹의 보간 거리를 초기화 해준다.
	m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

	// 현재 그룹이 보보간 중이였다면 초기화 해준다
	if (m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.bChainInterpolation)
	{
		m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.bChainInterpolation = false;
		m_BlendGroups[_int(eSrcInterpolationGroup)].AnimInfo.pAnim->Reset_ChainInterpolation_Bones_SRT();
	}
}

// TOTAL에서 TOTAL이 아닌 경우로 보간이 끝난 경우,
// TOTAL -> LOWER & UPPER
// 위 경우 iTotalInterpolationRequestCnt는 무조건 2개 이상이다.
void CModel::Finalize_Interpolation_When_TotalToNotTotal(Abgt eSrcInterpolationGroup, Abgt eDstStartAnimGroup)
{
	const _int iEndValue = -((tot - low) + 1);

	// 현재 그룹이 모든 보간이 끝났는지 확인한다
	_int iEndInterpolationSumPre = 0;
	for (_int iDstBlendGroup = low; iDstBlendGroup <= tot; ++iDstBlendGroup)
		iEndInterpolationSumPre += m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.InterpolationDstGroupIdices[iDstBlendGroup];

	if (iEndValue == iEndInterpolationSumPre)
		return;

	for (_int iGroupIdx = low; iGroupIdx <= tot; ++iGroupIdx)
	{
		_int iDstInterpolationAnimIdx = m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.InterpolationDstGroupIdices[iGroupIdx];
		if (iDstInterpolationAnimIdx != -1)
		{
			// eSrcInterpolationGroup그룹이 보간이 끝났을 때 호출하는 함수이니 eSrcInterpolationGroup그룹의 애니매이션 보간 초기화 작업을 해준다.
			m_BlendGroups[iGroupIdx].AnimInfo.pAnim = m_Animations[iDstInterpolationAnimIdx];

			if (m_BlendGroups[iGroupIdx].AnimInfo.bEnd)
				m_BlendGroups[iGroupIdx].AnimInfo.bEnd = false;

			m_BlendGroups[iGroupIdx].AnimInfo.bUse = true;
			m_BlendGroups[iGroupIdx].AnimInfo.bInterpolationing = false;
			m_BlendGroups[iGroupIdx].AnimInfo.iAnimIdx = iDstInterpolationAnimIdx;
			m_BlendGroups[iGroupIdx].AnimInfo.strAnimName = Get_AnimName(iDstInterpolationAnimIdx);
			Reset_Anim(m_BlendGroups[iGroupIdx].AnimInfo.iAnimIdx);

			if (m_BlendGroups[iGroupIdx].InterpolationInfo.bChainInterpolation)
			{
				m_BlendGroups[iGroupIdx].InterpolationInfo.bChainInterpolation = false;
				m_BlendGroups[iGroupIdx].AnimInfo.pAnim->Reset_ChainInterpolation_Bones_SRT();
			}

			m_BlendGroups[iGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

			// 현재 그룹 보간 목적지 인덱스들을 -1로 만들어 버린다.
			Reset_InterpolationDstIndices(Abgt(iGroupIdx));

			m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.InterpolationDstGroupIdices[iGroupIdx] = -1;

			// 보간이 끝나고 새롭게 재생할 애니매이션 TrackPos(시작 위치)를 맞춘다.
			_float fDstAnimDurtaion = m_BlendGroups[iGroupIdx].AnimInfo.pAnim->Get_AnimDuration();
			_float fSrcInterpolationDis = m_BlendGroups[(_int)eSrcInterpolationGroup].InterpolationInfo.fInterpolationDis;
			Adjust_TrackPos_When_EndInterpolation(m_BlendGroups[iGroupIdx].AnimInfo.pAnim, fDstAnimDurtaion, fSrcInterpolationDis);
		}
	}

	// 보간의 목적지의 애니매이션 인덱스 합이 -6이라면 모든 보간의 finalize가 끝났다는 것을 의미한다. -> 현재 그룹의 애니매이션 정보, 보간 정보를 초기화 해준다.
	// 현재 그룹이 모든 보간이 끝났는지 확인한다
	_int iEndInterpolationSum = 0;
	for (_int iDstBlendGroup = low; iDstBlendGroup <= tot; ++iDstBlendGroup)
		iEndInterpolationSum += m_BlendGroups[_int(eSrcInterpolationGroup)].InterpolationInfo.InterpolationDstGroupIdices[iDstBlendGroup];

	// 현재 그룹의 모든 보간이 끝났다. -> 초기화를 해준다.
	if (iEndInterpolationSum == iEndValue)
	{
		_int iCurGroupIdx = _int(eSrcInterpolationGroup);
		Reset_Anim(m_BlendGroups[iCurGroupIdx].AnimInfo.iAnimIdx); // 모든 보간이 끝났기 때문에 현재 애니매이션을 초기화 해준다.

		// 현재 보간하던 그룹과 목적지 보간 그룹이 다른 경우 현재 애니매이션 재생 그룹을 사용하지 않는다고 설정하고 초기화 해준다.
		if (eSrcInterpolationGroup != eDstStartAnimGroup)
		{
			if (m_BlendGroups[iCurGroupIdx].AnimInfo.bEnd)
				m_BlendGroups[iCurGroupIdx].AnimInfo.bEnd = false;

			m_BlendGroups[iCurGroupIdx].AnimInfo.bUse = false;
			m_BlendGroups[iCurGroupIdx].AnimInfo.bInterpolationing = false;
			m_BlendGroups[iCurGroupIdx].AnimInfo.iAnimIdx = -1;
			m_BlendGroups[iCurGroupIdx].AnimInfo.pAnim = nullptr;
			m_BlendGroups[iCurGroupIdx].AnimInfo.strAnimName = "";

			// iCurGroupIdx 그룹 보간 초기화
			m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
			m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation = false;

		}
		// 현재 보간하던 그룹과 목적지 보간 그룹이 같은 경우
		else
		{
			m_BlendGroups[iCurGroupIdx].AnimInfo.bUse = true;									// 현재 그룹을 사용한다고 설정
			m_BlendGroups[iCurGroupIdx].AnimInfo.bInterpolationing = false;						// 현재 그룹 보간 여부를 false로 설정
			m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
		}

		// 모든 보간이 끝났기 때문에 보간 목적지 인덱스들을 전부 -1로 설정한다.
		Reset_InterpolationDstIndices(eSrcInterpolationGroup);

		// 현재 그룹 보간 거리 0 으로 초기화
		m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

		// 보보간 중이였다면 250130
		if (m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation)
		{
			m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation = false;
			m_BlendGroups[iCurGroupIdx].AnimInfo.pAnim->Reset_ChainInterpolation_Bones_SRT();
		}

		// 다른 그룹에서 보간이 끝났는지 아닌지 확인해준다.
		if (eSrcInterpolationGroup == Abgt::TOTAL)
		{
			for (_int i = low; i <= tot; ++i)
			{
				if (!m_BlendGroups[i].AnimInfo.bUse && m_BlendGroups[i].AnimInfo.bInterpolationing)
					memset(m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices, -1, sizeof m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices);
			}
		}
	}


	// 상하체가 같은지 확인한다.
	// 보간이 끝났는데 상하체의 애니매이션이 같다면 토탈로 재생시킨다.
	if (iEndInterpolationSum == iEndValue)
	{
		_bool bFindSameAnim = false;
		_int iSameAnimIdx = -1;
		for (_int i = iLower; i <= iLower + 1; ++i)
		{
			if (m_BlendGroups[i].AnimInfo.iAnimIdx != -1 && (i == iLower + 1) && iSameAnimIdx == m_BlendGroups[i].AnimInfo.iAnimIdx)
			{
				bFindSameAnim = true;
				break;
			}

			iSameAnimIdx = m_BlendGroups[i].AnimInfo.iAnimIdx;
		}

		if (bFindSameAnim)
		{
			for (_int i = iLower; i < iTotal; ++i)
			{
				m_BlendGroups[i].AnimInfo.bUse = false;
				m_BlendGroups[i].AnimInfo.bEnd = false;
				m_BlendGroups[i].AnimInfo.bInterpolationing = false;
				m_BlendGroups[i].AnimInfo.iAnimIdx = -1;
				m_BlendGroups[i].AnimInfo.strAnimName = "";

				m_BlendGroups[i].InterpolationInfo.bChainInterpolation = false;
				m_BlendGroups[i].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
				Reset_InterpolationDstIndices(Abgt(i));
			}

			m_BlendGroups[iTotal].AnimInfo.bUse = true;
			m_BlendGroups[iTotal].AnimInfo.bEnd = false;
			m_BlendGroups[iTotal].AnimInfo.iAnimIdx = iSameAnimIdx;
			m_BlendGroups[iTotal].AnimInfo.pAnim = m_Animations[iSameAnimIdx];
			m_BlendGroups[iTotal].AnimInfo.strAnimName = m_BlendGroups[iTotal].AnimInfo.pAnim->Get_AnimName();
			
			m_BlendGroups[iTotal].InterpolationInfo.bChainInterpolation = false;
			m_BlendGroups[iTotal].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

			// 보간이 끝나고 새롭게 재생할 애니매이션 TrackPos(시작 위치)를 맞춘다.
			_float fDstAnimDurtaion = m_BlendGroups[iTotal].AnimInfo.pAnim->Get_AnimDuration();
			_float fSrcInterpolationDis = m_BlendGroups[(_int)eSrcInterpolationGroup].InterpolationInfo.fInterpolationDis;
			Adjust_TrackPos_When_EndInterpolation(m_BlendGroups[iTotal].AnimInfo.pAnim, fDstAnimDurtaion, fSrcInterpolationDis);
		}
	}
}

void CModel::Reset_InterpolationDstIndices(Abgt eGroup)
{
	for (_int i = iLower; i <= iTotal; ++i)
		m_BlendGroups[_int(eGroup)].InterpolationInfo.InterpolationDstGroupIdices[i] = -1;
}

void CModel::Finalize_Interpolation_Exception()
{
	_int iUseCnt = 0;
	// 모든 보간이 끝났는데 사용중인 애니매이션이 하나도 없는 경우
	for (_int i = low; i <= tot; ++i)
	{
		if (!m_BlendGroups[i].AnimInfo.bUse && !m_BlendGroups[i].AnimInfo.bInterpolationing)
			++iUseCnt;
	}

	// 사용중인 그룹이 하나도 없는 경우
	if (!iUseCnt)
	{

#ifdef _DEBUG

		cout << endl;
		cout << "iUseCnt : " << iUseCnt << " 보간이 끝났는데 사용중인 그룹이 하나도 없습니다. 캐쉬된 애니매이션 인덱스를 사용합니다." << endl;
		cout << endl;

		Use_Cache_AnimIdx();

#endif

	}
}

void CModel::Use_Cache_AnimIdx()
{
	for (_int i = 0; i < m_Cache_AnimIdx[tot].size(); ++i)
	{
		Ready_ToPlay_Anim(Abgt::TOTAL, m_Cache_AnimIdx[tot].front());
		m_Cache_AnimIdx[tot].clear();
		break;
	}
}

void CModel::Ready_Interpolation(Abgt eSrcInterpolationGroup, Abgt eDstInterpolationGroup, _int iInterpolationDstAnimIdx)
{
	const _int iSrcGroupIdx = _int(eSrcInterpolationGroup);
	const _int iDstGroupIdx = _int(eDstInterpolationGroup);

	// 보간을 시작하기 전, 현재 그룹 애니매이션 정보 설정
	{
		// 사용하지 않는다고 설정, 보간시작이라 설정
		m_BlendGroups[iSrcGroupIdx].AnimInfo.bUse = false;
		m_BlendGroups[iSrcGroupIdx].AnimInfo.bInterpolationing = true;
	}

	// 보간을 시작하기 전, 현재 그룹 보간 정보 설정
	{
		// 보간 목적지 애니매이션 인덱스 설정
		m_BlendGroups[iSrcGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iDstGroupIdx] = iInterpolationDstAnimIdx;

		// 보간 TrackPos초기화
		m_BlendGroups[iSrcGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
		
		// 혹시나 보보간 이상의 설정이 true라면 false로 만든다.
		if (m_BlendGroups[iSrcGroupIdx].InterpolationInfo.bChainInterpolation)
			m_BlendGroups[iSrcGroupIdx].InterpolationInfo.bChainInterpolation = false;

		Set_InterpolationData(m_BlendGroups[iSrcGroupIdx].AnimInfo.iAnimIdx, iInterpolationDstAnimIdx, m_BlendGroups[iSrcGroupIdx].InterpolationInfo);
		Cal_Interpolation_KeyFrame(iInterpolationDstAnimIdx, m_BlendGroups[iSrcGroupIdx].InterpolationInfo);
	}
}

void CModel::Ready_Interpolation_TotalToTotal(_int iInterDstAnimIdx)
{
	const _int iTotal = _int(Abgt::TOTAL);
	// 보간을 시작하기 전, 현재 그룹 애니매이션 정보 설정
	{
		// 사용하지 않는다고 설정, 보간시작이라 설정
		m_BlendGroups[iTotal].AnimInfo.bUse = false;
		m_BlendGroups[iTotal].AnimInfo.bInterpolationing = true;
	}

	// 보간을 시작하기 전, 현재 그룹 보간 정보 설정
	{
		// 보간 목적지 애니매이션 인덱스 설정
		m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[iTotal] = iInterDstAnimIdx;

		// 보간 TrackPos초기화
		m_BlendGroups[iTotal].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

		// 혹시나 보보간 이상의 설정이 true라면 false로 만든다.
		if (m_BlendGroups[iTotal].InterpolationInfo.bChainInterpolation)
			m_BlendGroups[iTotal].InterpolationInfo.bChainInterpolation = false;

		Set_InterpolationData(m_BlendGroups[iTotal].AnimInfo.iAnimIdx, iInterDstAnimIdx, m_BlendGroups[iTotal].InterpolationInfo);
	}
}

void CModel::Ready_ToPlay_Anim(Abgt eGroup, const ANIM_INFO& SrcAnimInfo)
{
	const _int iGroupIdx = (_int)eGroup;

	m_BlendGroups[iGroupIdx].AnimInfo.bUse = true;
	m_BlendGroups[iGroupIdx].AnimInfo.bInterpolationing = false;
	if (m_BlendGroups[iGroupIdx].AnimInfo.bEnd)
		m_BlendGroups[iGroupIdx].AnimInfo.bEnd = false;

	m_BlendGroups[iGroupIdx].AnimInfo.pAnim = SrcAnimInfo.pAnim;
	m_BlendGroups[iGroupIdx].AnimInfo.iAnimIdx = SrcAnimInfo.iAnimIdx;
	m_BlendGroups[iGroupIdx].AnimInfo.strAnimName = SrcAnimInfo.strAnimName;

	if (m_BlendGroups[iGroupIdx].InterpolationInfo.bChainInterpolation)
		m_BlendGroups[iGroupIdx].InterpolationInfo.bChainInterpolation = false;

	m_BlendGroups[iGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

	Reset_InterpolationDstIndices(Abgt(iGroupIdx));
}

void CModel::Ready_ToPlay_Anim(Abgt eGroup, const _int iAnimIdx)
{
	const _int iGroupIdx = (_int)eGroup;

	m_BlendGroups[iGroupIdx].AnimInfo.bUse = true;
	m_BlendGroups[iGroupIdx].AnimInfo.bInterpolationing = false;
	if (m_BlendGroups[iGroupIdx].AnimInfo.bEnd)
		m_BlendGroups[iGroupIdx].AnimInfo.bEnd = false;


	m_BlendGroups[iGroupIdx].AnimInfo.pAnim = m_Animations[iAnimIdx];
	m_BlendGroups[iGroupIdx].AnimInfo.iAnimIdx = iAnimIdx;
	m_BlendGroups[iGroupIdx].AnimInfo.strAnimName = m_Animations[iAnimIdx]->Get_AnimName();

	if (m_BlendGroups[iGroupIdx].InterpolationInfo.bChainInterpolation)
		m_BlendGroups[iGroupIdx].InterpolationInfo.bChainInterpolation = false;

	m_BlendGroups[iGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

	Reset_InterpolationDstIndices(Abgt(iGroupIdx));
}

void CModel::Ready_ChainInterpolation(Abgt eSrcChainInterpolationGroup, Abgt eDstChainInterpolationGroup, _int iChainInterpolationDstAnimIdx, _bool Setup_BoneSRT)
{
	const _int iSrcGroupIdx = (_int)eSrcChainInterpolationGroup;
	const _int iDstGroupIdx = (_int)eDstChainInterpolationGroup;

	if (Out_Of_BlendGroup(iSrcGroupIdx) || Out_Of_BlendGroup(iDstGroupIdx))
		return;

	if (iChainInterpolationDstAnimIdx == NON)
	{
#ifdef _DEBUG

		assert(false, iChainInterpolationDstAnimIdx == NON);
#endif 
		return;
	}
		
	// 보보간을 시작하기 전, 현재 그룹 보보간 정보 설정
	// 보간 목적지 애니매이션 인덱스 설정
	m_BlendGroups[iSrcGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iDstGroupIdx] = iChainInterpolationDstAnimIdx;

	// 보간 TrackPos초기화
	m_BlendGroups[iSrcGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

	// 혹시나 보보간 이상의 설정이 true라면 false로 만든다.
	if (!m_BlendGroups[iSrcGroupIdx].InterpolationInfo.bChainInterpolation)
		m_BlendGroups[iSrcGroupIdx].InterpolationInfo.bChainInterpolation = true;

	Set_InterpolationData(m_BlendGroups[iSrcGroupIdx].AnimInfo.iAnimIdx, iChainInterpolationDstAnimIdx, m_BlendGroups[iSrcGroupIdx].InterpolationInfo);
	Cal_Interpolation_KeyFrame(iChainInterpolationDstAnimIdx, m_BlendGroups[iSrcGroupIdx].InterpolationInfo);

	if (Setup_BoneSRT)
	{
		if (m_BlendGroups[iSrcGroupIdx].AnimInfo.pAnim)
			m_BlendGroups[iSrcGroupIdx].AnimInfo.pAnim->Setup_ChainInterpolation_BoneSRT();
	}
}

void CModel::Cal_Interpolation_KeyFrame(_int iInterpolationDstAnimIdx, INTERPOLATION_INFO& InterpolationSrcInfo)
{
	if (iInterpolationDstAnimIdx == -1)
	{
		assert(false, "iInterpolationAnimIdx가 -1이다");
		return;
	}

	CAnimation* pTargetAnim = m_Animations[iInterpolationDstAnimIdx];
	if (pTargetAnim)
	{
		_float fTargetAnimDurtaion = pTargetAnim->Get_AnimDuration();
		const _float fSrcInterpolationDis = InterpolationSrcInfo.fInterpolationDis;
		_float fDstTrackPos = fSrcInterpolationDis;

		while (fDstTrackPos > fTargetAnimDurtaion)
			fDstTrackPos -= fTargetAnimDurtaion;

		pTargetAnim->Cal_DstKeyFrame(fTargetAnimDurtaion, fDstTrackPos);
	}
}

void CModel::Adjust_TrackPos_When_EndInterpolation(CAnimation* pDstAnim, _float fDstAnimDuration, _float fSrcInterpolationDis)
{
	if (!pDstAnim)
	{
		assert(false, "Adjust_TrackPos_When_EndInterpolation pDstAnim가 nullptr입니다");
		return;
	}

	_float fDstTrackPos = fSrcInterpolationDis;

	while (fDstTrackPos > fDstAnimDuration)
		fDstTrackPos -= fDstAnimDuration;

	pDstAnim->Set_CurTrackPos(fDstTrackPos);
}

_bool CModel::Is_TotalToTotal_Interpolationing()
{
	_int iSameAnimIdx = -1;
	for (_int i = iLower; i <= iLower + 1; ++i)
	{
		_int iCurDstInterpolationAnimIdx = m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[i];

		// 상체 하체만 검사를 했는데 이전 애니매이션과 현재 애니매이션이 같다. == 상하체의 애니매이션이 같다 -> 이 경우 Total로의 보간으로 수정해야한다.
		if (iCurDstInterpolationAnimIdx != -1 && iSameAnimIdx != -1 && iSameAnimIdx == iCurDstInterpolationAnimIdx)
			return true;
		iSameAnimIdx = m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[i];
	}
	return false;
}

_bool CModel::Is_NotTotalToTotal_Interpolation_Condition()
{
	for (_int i = iLower; i <= iLower + 1; ++i)
	{
		_int iCurDstInterpolationAnimIdx = m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices[iTotal];
		
		// 어떤 그룹에서 tot로의 보간이 있다면 토탈 -> 토탈로의 보간을 수행한다.
		if (iCurDstInterpolationAnimIdx != -1)
			return true;
	}
	return false;
}

_bool CModel::Process_UsingGroups(_int iUsingCnt, _int iRequestCnt, vector<pair<Abgt, _int>>& UsingGroups, vector<pair<Abgt, _int>>& ChangeGroups)
{
	if (!iUsingCnt)
		return true;

	// 전체 -> 전체
	// 전체 -> 상하
	if (iUsingCnt == 1)
	{
		_int iNotTotalGroupIdx = NON;
		_bool bTotal = false;
		for (_int i = iLower; i <= iTotal; ++i)
		{
			if (UsingGroups[i].second != NON && i == iTotal)
				bTotal = true;
			
			if (UsingGroups[i].second != NON)
				iNotTotalGroupIdx = i;
		}

		// 사용중인 그룹이 tot인경우
		if (bTotal)
		{
			_bool bIsRequestGroupTotal = false;
			for (_int i = iLower; i <= iTotal; ++i)
			{
				if (ChangeGroups[i].second != NON && i == iTotal)
					bIsRequestGroupTotal = true;
			}

			// 전체 -> 전체 (요청 그룹이 1개이고(tot로의 변경 요청일 때) 
			if (iRequestCnt == 1 && bIsRequestGroupTotal)
			{
				_int iRequestChangeAnimIdx = ChangeGroups[iTotal].second;
				_int iCurUsingGroupAnimIdx = UsingGroups[iTotal].second;

				// 사용중인 그룹 개수가 1개이고 요청한 그룹 개수도 1개이다 이것은 tot -> tot인 경우이다. 근데 애니매이션 인덱스가 같다면 그냥 true를 리턴한다.
				if (iCurUsingGroupAnimIdx == iRequestChangeAnimIdx)
					return true;

				// 같은 경우가 아니기 때문에 보간한다.
				Ready_Interpolation(Abgt::TOTAL, Abgt::TOTAL, iRequestChangeAnimIdx);

				// 250217
				m_iFinalize_ResetExceptionAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, tot);
			}
			// 전체 -> 상하(그 이상)
			else if (iRequestCnt >= 2 && !bIsRequestGroupTotal)
			{
				_int iCurUsingGroupAnimIdx = UsingGroups[iTotal].second;

				for (_int iGroupIdx = iLower; iGroupIdx < iTotal; ++iGroupIdx)
				{
					_int iRequestAnimIdx = ChangeGroups[iGroupIdx].second;
					if (iRequestAnimIdx == NON)
						continue;

					// 전체에 해당하는 애니매이션이 요청이 온 애니매이션과 같은 경우
					// iGroupIdx에 해당하는 그룹은 애니매이션 재생을 하도록 한다.
					if (iCurUsingGroupAnimIdx == iRequestAnimIdx)
					{
						Ready_ToPlay_Anim(Abgt(iGroupIdx), m_BlendGroups[iTotal].AnimInfo);
					}
					// 다르다면 Total에서 보간을 하도록 설정한다.
					else
					{
						Ready_Interpolation(Abgt::TOTAL, Abgt(iGroupIdx), iRequestAnimIdx);
					}
				}
			}
		}
		// 사용중인 그룹이 total이 아닌경우 상, 하 등 중에서 하나의 그룹이다.
		else
		{
			if (iNotTotalGroupIdx == NON)
				return true;

			// total로의 보간 요청이 아닌 경우
			if (ChangeGroups[iTotal].second == NON)
			{
				_int iCurAnimIdx = m_BlendGroups[iNotTotalGroupIdx].AnimInfo.iAnimIdx;
				_int iReAnimIdx = ChangeGroups[iNotTotalGroupIdx].second;

				// 현재 iNotTotalGroupIdx 그룹에서 재생중인 애니매이션과 변경 요청이 온 애니매이션이 같다면 보간하지 않고 건너뛴다.
				if (iReAnimIdx == NON || iCurAnimIdx == iReAnimIdx)
					return true;

				Ready_Interpolation(Abgt(iNotTotalGroupIdx), Abgt(iNotTotalGroupIdx), iReAnimIdx);
			}
			// tot로의 보간 요청인 경우
			else
			{
				_int iReTotAnimIdx = ChangeGroups[iTotal].second;

				for (_int i = iLower; i < iTotal; ++i)
				{
					if (Is_Use_Condition(m_BlendGroups[i]))
					{
						_int iCurUsingAnimIdx = UsingGroups[i].second;

						if (iCurUsingAnimIdx != iReTotAnimIdx)
						{
							Ready_Interpolation(Abgt(i), Abgt::TOTAL, iReTotAnimIdx);
							continue;
						}
					}
				}
			}
		}
	}
	// 상하 -> 전체
	// 상하 -> 상하
	else
	{
		// 상하 -> 전체
		if (iRequestCnt == 1)
		{
			_int iRequestAnimIdx = ChangeGroups[iTotal].second;

			for (_int iGroupIdx = iLower; iGroupIdx < iTotal; ++iGroupIdx)
			{
				_int iCurGroupAnimIdx = UsingGroups[iGroupIdx].second;
				if (iCurGroupAnimIdx == NON)
					continue;

				// total에 요청이 온 애니매이션 인덱스와 현재 그룹 Lower~Head 까지의 애니매이션 인덱스가 일치하는게 있다면 해당 그룹은 보간하지 않고 특정 그룹의 보간이 끝나면 total로 재생한다
				if (iCurGroupAnimIdx == iRequestAnimIdx)
					continue;

				Ready_Interpolation(Abgt(iGroupIdx), Abgt::TOTAL, iRequestAnimIdx);
			}
		}
		// 상하 -> 상하(그 이상)
		// 아래의 경우 무조건 1대1 대응이다.
		else
		{
			for (_int iGroupIdx = iLower; iGroupIdx < iTotal; ++iGroupIdx)
			{
				_int iCurGroupUsingAnimIdx = UsingGroups[iGroupIdx].second;
				if (iCurGroupUsingAnimIdx == NON)
					continue;

				// 요청이 온 애니매이션과 현재 같은 그룹의 애니매이션이 같다면 건너뛴다.
				_int iRequestAnimIdx = ChangeGroups[iGroupIdx].second;

				if (iRequestAnimIdx == NON)
					continue;

				if (iRequestAnimIdx == iCurGroupUsingAnimIdx)
					continue;

				Ready_Interpolation(Abgt(iGroupIdx), Abgt(iGroupIdx), iRequestAnimIdx);
			}
		}
	}

	return true;
}

// 예외가 발생하는 경우 false를 리턴한다.
// InterpolationingGroups는 보간중인 그룹들만 담겨 있다. 해당 그룹에 담겨 있는 그룹은 보보간 처리를 해준다.
_bool CModel::Process_InterpolationingGroups(_int iInterpolationingGroupCnt, _int iRequestCnt, vector<pair<Abgt, _int>>& InterpolationingGroups, vector<pair<Abgt, _int>>& ChangeGroups)
{
	// 보간 중인 그룹 개수가 하나도 없다면 true를 바로 리턴한다.
	if (!iInterpolationingGroupCnt)
		return true;

	// 요청의 개수가 1개 인 경우 tot로의 변경 요청이다.
	if (iRequestCnt == 1)
	{
		_int iReAnimIdx = ChangeGroups[tot].second;
		if (iReAnimIdx == NON)
			return true;

		// 보간 중인 그룹의 개수가 1개인 경우 tot이거나 tot이 아닌 다른 그룹중 하나이다.
		if (iInterpolationingGroupCnt == 1)
		{
			_bool bIsCurInterpolationingGroupTotal = false;
			for (_int i = iLower; i <= iTotal; ++i)
			{
				// i가 Total이고 -1이 아니라면 토탈에서 부터 보간을 시작한 것이 맞다.
				if (i == iTotal && InterpolationingGroups[i].second != NON)
				{
					bIsCurInterpolationingGroupTotal = true;
					break;
				}
			}

			// 현재 tot 라는 그룹이 보간중인 경우
			if (bIsCurInterpolationingGroupTotal)
			{
				_int iInterpolationingCnt = Get_Dst_Interpolationing_Cnt(Abgt(tot));

				// tot라는 그룹이 보간 중인데 보간 목적지 그룹의 개수가 1개인 경우 ( 보간중인 그룹의 개수가 1개인 경우 tot로의 보간 중이였는지 다른 그룹 하나로의 보간중이였는지 확인한다. )
				if (iInterpolationingCnt == 1)
				{
					_int iCheckAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, tot);

					// iCheckAnimIdx == NON 이면 tot -> 에서 tot가 아닌 특정 그룹 하나로의 보간중임을 의미한다.
					if (iCheckAnimIdx == NON)
					{
						// tot가 보간중인 특정 그룹 하나를 찾는다.
						_int iCurDstInterpolationingGroupIdx = NON; // tot가 보간중인 특정 그룹 하나
						for (_int i = low; i < tot; ++i)
						{
							if (Get_Dst_InterAnimIdx(Abgt::TOTAL, i) != NON)
							{
								iCurDstInterpolationingGroupIdx = i;
								break;
							}
						}

						// 예외 처리
						if (iCurDstInterpolationingGroupIdx == NON)
						{
							assert(false, "tot가 아닌 특정 그룹 하나로의 보간 중임을 못 찾았다");
							return false;
						}

						// tot 라는 그룹이 보간중인 애니매이션 인덱스
						_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, iCurDstInterpolationingGroupIdx);

						// tot 이라는 그룹이 tot이 아닌 특정 그룹으로의 보간중이고, 요청은 tot로의 보간 요청이 온 상태이다.

						// 변경 요청 애니매이션과 현재 tot 그룹이 보간 중인 애니매이션 인덱스가 다르다면 보보간을 수행하도록 한다.
						if (iReAnimIdx != iCurDstInterpolationingAnimIdx)
						{
							// tot -> iCurDstInterpolationingGroupIdx 그룹으로의 보간을 NON으로 밀어버린다.
							Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt(iCurDstInterpolationingGroupIdx));

							// tot -> A -> tot로의 보보간 요청을 한다.
							Ready_ChainInterpolation(Abgt::TOTAL, Abgt::TOTAL, iReAnimIdx);
						}
					}
					// iCheckAnimIdx != NON 이면 tot -> tot로의 보간중임을 의미한다.
					else
					{
						// 현재 보간중인 애니매이션
						_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, Abgti(tot));
						
						if (iReAnimIdx != iCurDstInterpolationingAnimIdx)
						{
							Ready_ChainInterpolation(Abgt::TOTAL, Abgt::TOTAL, iReAnimIdx);
						}
					}
				}
				// tot라는 그룹이 보간 중인데 보간 목적지 그룹의 개수가 2개 이상인 경우
				else
				{
					// tot -> AB 로의 보간 중인 경우이다.
					for (_int i = low; i < tot; ++i)
					{
						_int iCurDstInterpolationingGroupIdx = i;
						_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, iCurDstInterpolationingGroupIdx);
						if (iCurDstInterpolationingAnimIdx == NON)
							continue;

						// 같다면 tot 그룹이 AB로 보간을 수행하도록 건너뛴다.
						if (iReAnimIdx == iCurDstInterpolationingAnimIdx)
							continue;

						// tot -> A로의 보간을 없앤다.
						Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt(iCurDstInterpolationingGroupIdx));
						
						Ready_ChainInterpolation(Abgt::TOTAL, Abgt::TOTAL, iReAnimIdx);
					}
				}
			}
			// 현재 tot 라는 그룹이 아닌 상하 등등 그룹들 중 하나가 보간 중인 경우
			else 
			{
				// iInterpolationingGroupIdx : tot가 아닌 보간중인 특정 그룹 하나를 뜻한다.
				// tot 그룹이 아닌 상하 팔 등 중에서 보간 중인 그룹 하나를 찾는다.
				_int iCurInterpolationingGroupIdx = NON;
				for (_int i = low; i < tot; ++i)
				{
					if (Is_Interpolation_Condition(m_BlendGroups[i]))
					{
						iCurInterpolationingGroupIdx = i;
						break;
					}
				}

				// 예외 처리
				if (iCurInterpolationingGroupIdx == NON)
				{
					assert(false, "iCurInterpolationingGroupIdx가 NON이다. 특정 그룹 하나는 무조건 찾아야 하는데 못 찾았다");
					return false;
				}

				// iCurInterpolationingGroupIdx는 tot그룹이 아닌 특정 그룹 하나를 뜻한다.
				// iCurInterpolationingGroupIdx 그룹이 어느 그룹으로 보간중인지 확인한다. 
				// tot가 아니기 때문에 1대1 로의 보간중이거나 tot로의 보간중이다.
				_int iCheckTotAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);

				// iCurInterpolationingGroupIdx 그룹은 tot 그룹으로의 보간중이 아니다.
				if (iCheckTotAnimIdx == NON)
				{
					// iCurInterpolationingGroupIdx 그룹이 어떤 그룹으로의 보간중인지 안 찾는다. 1대1 대응 보간중이다.
					_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), iCurInterpolationingGroupIdx);

					// 변경 요청 애니매이션과 현재 보간중인 애니매이션이 다른 경우에만 보보간을 수행할 수 있도록 한다.
					if (iReAnimIdx != iCurDstInterpolationingAnimIdx)
					{
						Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(iCurInterpolationingGroupIdx), iReAnimIdx);

					}
				}
				// iCurInterpolationingGroupIdx 그룹은 tot 그룹으로의 보간중이다.
				else
				{
					// 현재 iCurInterpolationingGroupIdx 그룹이 tot 그룹으로의 보간중인 애니매이션 인덱스
					_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);

					if (iCurDstInterpolationingAnimIdx == NON)
					{
						assert(false, "iCurDstInterpolationingAnimIdx == NON");
						return false;
					}

					// 변경 요청 애니매이션과 현재 보간중인 애니매이션이 다른 경우에만 보보간을 수행할 수 있도록 한다.
					if (iReAnimIdx != iCurDstInterpolationingAnimIdx)
					{
						Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt::TOTAL, iReAnimIdx);
					}
				}
			}
		}
		// 보간 중인 그룹의 개수가 2개 이상인 경우
		else
		{
			for (_int i = low; i <= tot; ++i)
			{
				if (InterpolationingGroups[i].second == NON)
					continue;

				// 보간중인 그룹을 찾았다.
				_int iCurInterpolationingGroupIdx = i;
				
				// 보간중인 그룹이 tot인 경우
				if (iCurInterpolationingGroupIdx == tot)
				{
					// 보간중인 그룹이 어떤 그룹으로의 보간중이였는지 확인한다.
					for (_int j = low; j <= tot; ++j)
					{
						_int iCurInterpolationingDstGroupIdx = j;
						_int iCurInterpolationingGroupAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), iCurInterpolationingDstGroupIdx);
						if (iCurInterpolationingGroupAnimIdx == NON)
							continue;

						// iCurInterpolationingGroupIdx : 보간중인 그룹을 찾았고
						// iCurInterpolationingDstGroupIdx : iCurInterpolationingGroupIdx가 보간중인 목적지 그룹도 찾았다.
						// iCurInterpolationingGroupAnimIdx과 변경 요청이 온 애니매이션이 다른 경우에 보보간 이상을 수행할 수 있도록 해준다.
						if (iReAnimIdx == iCurInterpolationingGroupAnimIdx)
							continue;

					}
				}
				// 보간중인 그룹이 tot가 아닌 경우, 상하 둘중 하나이다.
				else
				{
					// 상하 이런 그룹은 1대1로의 보간중이거나 tot로의 보간중이거나 둘중 하나만 확인하면 된다.
					_int iCheckTotAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);

					// 상하.. 그룹이 tot로의 보간중이 아니다.
					if (iCheckTotAnimIdx == NON)
					{
						// 상하.. 그룹이 tot로의 보간중이 아니기 때문에 1대1 대응 보간 중이다.
						_int iCurDstSameGroupInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), iCurInterpolationingGroupIdx);
						
						if (iCurDstSameGroupInterpolationingAnimIdx != iReAnimIdx)
						{

							Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(iCurInterpolationingGroupIdx), iReAnimIdx);
						}
					}
					// 상하.. 그룹이 tot로의 보간중이다.
					else
					{
						_int iCurDstSameGroupInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);
						
						// 변경 요청이 온 애니매이션과 현재 보간중인 애니매이션이 다르기 때문에 보보간을 수행할 수 있도록 요청한다.
						if (iReAnimIdx != iCurDstSameGroupInterpolationingAnimIdx)
						{


							Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt::TOTAL, iReAnimIdx);

						}
					}
				}
			}
		}
	}
	// 요청의 개수가 2개 이상인 경우 tot로의 변경 요청이 아니다.
	else
	{
		// 현재 보간중인 그룹 개수가 1개인 경우
		if (iInterpolationingGroupCnt == 1)
		{
			_int iCurInterpolationingGroupIdx = NON;
			for (_int i = low; i <= tot; ++i)
			{
				if (InterpolationingGroups[i].second != NON)
				{
					iCurInterpolationingGroupIdx = i;
					break;
				}
			}

			// 예외 처리
			if (iCurInterpolationingGroupIdx == NON)
			{
				assert(false, "iCurInterpolationingGroupIdx == NON");
				return false;
			}

			// iCurInterpolationingGroupIdx 보간중인 그룹
			// iCurInterpolationingGroupIdx 그룹이 어떤 그룹으로 보간중인지 찾는다.
			if (iCurInterpolationingGroupIdx == tot)
			{
				// 현재 보간중인 그룹이 tot 그룹인데 변경 요청은 2개이다.

				_int iCurInterpolationingCnt = Get_Dst_Interpolationing_Cnt(Abgt::TOTAL);
				
				// tot 그룹이 보간중인데 보간 중인 그룹의 개수가 1개인 경우 tot -> tot 보간중이거나 tot -> A, B 이런식의 보간중이다.
				if (iCurInterpolationingCnt == 1)
				{
					_int iCheckAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, tot);
					
					// NON이라면 tot -> tot 보간중이 아니라 특정 그룹 하나로의 보간 중이다.
					if (iCheckAnimIdx == NON)
					{
						// tot가 아닌 특정 보간중인 그룹 하나를 찾는다.
						_int iDstInterpolationingGroupIdx = NON;
						for (_int i = low; i < tot; ++i)
						{
							_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, i);
							if (iCurDstInterpolationingAnimIdx != NON)
							{
								iDstInterpolationingGroupIdx = i;
								break;
							}
						}

						// 예외 처리
						if (iDstInterpolationingGroupIdx == NON)
						{
							assert(false, "iDstInterpolationingGroupIdx == NON");
							return false;
						}

						_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), iDstInterpolationingGroupIdx);

						// 예외 처리
						if (iCurDstInterpolationingAnimIdx == NON)
						{
							assert(false, "iCurDstInterpolationingAnimIdx == NON");
							return false;
						}

						// tot -> A 보간중인데 AB 이런식의 변경 요청이 왔다.
						// 변경 요청 그룹을 순회하면서 현재 tot -> A 보간중인 A에 해당하는 애니매이션과 일치한다면 건너뛰고 다르다면 보보간을 수행해준다.						
						for (_int i = low; i < tot; ++i)
						{
							_int iReAnimIdx = ChangeGroups[i].second;
							if (iReAnimIdx == NON)
								continue;

							if (iCurDstInterpolationingAnimIdx == iReAnimIdx)
								continue;

							if (Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), i) != NON)
								Reset_DstInterpolationIdx(Abgt(iCurInterpolationingGroupIdx), Abgt(i));

							Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(i), iReAnimIdx);
						}
					}
					// NON이 아니라면 tot -> tot로의 보간중이다. (250215 수정) , 보간 요청 개수는 2개이다.
					else
					{
						// A : 변경 요청은 2개 이고, tot -> tot로 보간중인 경우이다. 변경 요청은 2개이기 때문에 무조건 '상하'(그 이상)로의 변경 요청임을 알 수 있다.
						// A의 경우 A -> B -> BC(상하)로 보간을 수행할 경우 tot -> 하체(B -> C)로의 보간은 수행되지만 상체에 대한 애니매이션 재생여부가 없기 때문에 굳는다.
						_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, tot);

						for (_int i = low; i < tot; ++i)
						{
							if (ChangeGroups[i].second == NON)
								continue;

							_int iReGroupIdx = i;
							_int iReAnimIdx = ChangeGroups[i].second;

							if (iCurDstInterpolationingAnimIdx != iReAnimIdx)
							{
								Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt::TOTAL);
								Ready_ChainInterpolation(Abgt::TOTAL, Abgt(i), iReAnimIdx);
							}
						}

						// =====================================================================================================================================================
						// EX-T (예외처리, 250215)
						{
							// A의 예외 처리
							// 모든 그룹을 확인해본다. A -> B -> BC(상하)의 경우 
							// 토탈 -> 하체는 넘기고 상체의 애니매이션이 다르기 때문에 토탈 -> 하체로 보간을 한다. 
							// 이렇게 되면 현재 재생되는 애니매이션은 0개, 보간 하는 애니매이션 1개이다. 토탈 -> 하체로의 보간은 수행되지만 상체는 애니매이션을 재생하지 못한다.
							// 그렇기 때문에 현재 사용중인 그룹 개수(iUsingGroupCnt)를 확인한다. 
							// iUsingGroupCnt가 0개이고 tot에서 재생하던 애니매이션과 변경 요청이 온 애니매이션이 같은 경우 해당 그룹에 애니매이션 정보를 넘겨준다.
							// ex) A -> B, BC(상하 요청) 의 경우 1. [전체(B) -> 하체(C) 보간을 수행] 2. [상체(없음) -> 상체(B)를 전달받아 그대로 재생] 

							Process_TotGroup_Exception(iCurDstInterpolationingAnimIdx, ChangeGroups);


							//_int iUsingGroupCnt = 0;
							//for (_int i = low; i < tot; ++i)
							//{
							//	if (Is_Use_Condition(m_BlendGroups[i]))
							//		++iUsingGroupCnt;
							//}

							//// 사용중인 그룹이 0개 이면 확인해서 어떤 그룹 재생 해주어야할지 결정해준다.
							//if (!iUsingGroupCnt)
							//{
							//	for (_int i = low; i < tot; ++i)
							//	{
							//		if (ChangeGroups[i].second == NON) // i번 요청 그룹이 NON이면 건너뛴다.
							//			continue;

							//		_int iReGroupIdx = i;
							//		_int iReAnimIdx = ChangeGroups[i].second;

							//		// i번 요청 그룹의 애니매이션이 현재 보간중인 애니매이션과 같다면
							//		// A -> B | BC 경우 상체에 대한 부분을 처리 해준다.
							//		if (iCurDstInterpolationingAnimIdx == iReAnimIdx) 
							//		{
							//			Ready_ToPlay_Anim(Abgt(iReGroupIdx), m_BlendGroups[tot].AnimInfo);
							//		}
							//	}
							//}
						}
						// =====================================================================================================================================================

					}
				}
				// tot 그룹에서 보간중인데 보간 중인 그룹의 개수가 2개인 경우 tot -> AB 이런식의 보간 중인 상황이다.
				else
				{
					for (_int i = low; i < tot; ++i)
					{
						_int iCurDstInterpolationingGroupIdx = i;
						_int iCurDstInterpolationingGroupAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, i);
						if (iCurDstInterpolationingGroupAnimIdx == NON)
							continue;

						// tot 그룹에서 보간중인 목적지 그룹을 찾았다 : iCurDstInterpolationingGroupIdx
						for (_int j = i; j < tot; ++j)
						{
							if (ChangeGroups[j].second == NON)
								continue;
							
							// tot -> AB 보간중인데 AB보간중인 그룹과 변경 요청이 온 애니매이션 그룹과 같은경우
							if (iCurDstInterpolationingGroupIdx == j)
							{
								_int iReAnimIdx = ChangeGroups[j].second;

								if (iReAnimIdx == iCurDstInterpolationingGroupAnimIdx)
									break;

								Ready_ChainInterpolation(Abgt::TOTAL, Abgt(j), iReAnimIdx);
								break;

							}
							// tot -> AB 보간중인데 보간중인 그룹 AB와 다른경우? 있을 수 없다.
							else
							{
								assert(false);
							}
						}
					}
				}
			}
			else
			{
				// 현재 보간중인 그룹이 tot가 아닌 경우, A B 둘중 하나이다.

				// iCurInterpolationingGroupIdx 그룹이 어떤 그룹으로 보간중인지 확인한다.
				_int iCheckAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);

				// NON이라면 1대1 보간중이다.
				if (iCheckAnimIdx == NON)
				{
					_int iReAnimIdx = ChangeGroups[iCurInterpolationingGroupIdx].second;
					_int iCurInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), iCurInterpolationingGroupIdx);
					
					if (iReAnimIdx != iCurInterpolationingAnimIdx)
					{
						Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(iCurInterpolationingGroupIdx), iReAnimIdx);
					}
				}
				// NON이 아니라면 tot로 보간중이다.
				// tot가 아닌 그룹(하체 : iCurInterpolationingGroupIdx)는 tot로 가거나 하체로 가거나 둘중 하나이다.
				else
				{
					_int iCheckAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);
					
					// tot가 아닌 그룹(하체)는 tot로의 보간중이 아니다. 1대1 보간중이다.
					if (iCheckAnimIdx == NON)
					{
						_int iReAnimIdx = ChangeGroups[iCurInterpolationingGroupIdx].second;

						if (iCheckAnimIdx != iReAnimIdx && iReAnimIdx != NON)
						{
							Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(iCurInterpolationingGroupIdx), iReAnimIdx);
						}
					}
					// tot가 아닌 그룹(하체)는 tot로 보간중이다.
					else
					{
						_int iCurInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);

						if (iCurInterpolationingAnimIdx != NON && ChangeGroups[iCurInterpolationingGroupIdx].second != iCurInterpolationingAnimIdx)
						{
							Reset_DstInterpolationIdx(Abgt(iCurInterpolationingGroupIdx), Abgt::TOTAL);
							Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(iCurInterpolationingGroupIdx), ChangeGroups[iCurInterpolationingGroupIdx].second);
						}
					}
				}
			}
		}
		// 현재 보간중인 그룹 개수가 2개 이상인 경우 (보간 요청 개수는 2개이다)
		else
		{
			// 보간중인 그룹에 tot가 포함되어 있는지 확인한다.
			_int  iCheckAnimIdx = InterpolationingGroups[tot].second;

			// 보간중인 그룹에 tot가 포함되어 있다.
			if (iCheckAnimIdx != NON)
			{
				for (_int i = low; i <= tot; ++i)
				{
					// 보간중인 그룹을 찾는다.
					_int iCurInterpolationingGroupIdx = i;
					
					// 보간중인 그룹이 tot인 경우
					if (iCurInterpolationingGroupIdx == tot)
					{
						_int iCurInterpolationingGroupCnt = Get_Dst_Interpolationing_Cnt(Abgt::TOTAL);
						
						// tot 그룹이 보간 중인 그룹 개수가 1개인 경우
						// tot -> A or tot -> tot이다.
						if (iCurInterpolationingGroupCnt == 1)
						{
							_int iCheckAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, tot);

							// tot -> tot로의 보간중이다.
							// tot -> tot -> AB 요청인 경우
							if (iCheckAnimIdx != NON)
							{
								_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);
								
								for (_int j = low; j < tot; ++j)
								{
									if (ChangeGroups[j].second == NON)
										continue;

									_int iReAnimIdx = ChangeGroups[j].second;
									
									if (iReAnimIdx == iCurDstInterpolationingAnimIdx)
										continue;

									Reset_DstInterpolationIdx(Abgt(iCurInterpolationingGroupIdx), Abgt(tot));
									Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(j), iReAnimIdx);
								}
							}
							// tot -> A, B 로의 보간중이다.
							else
							{
								for (_int j = low; j < tot; ++j)
								{
									_int iReAnimIdx = ChangeGroups[j].second;
									if (iReAnimIdx == NON)
										continue;

									if (Get_Dst_InterAnimIdx(Abgt::TOTAL, j) != NON && iReAnimIdx != Get_Dst_InterAnimIdx(Abgt::TOTAL, j))
									{
										Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt(j));
										Ready_ChainInterpolation(Abgt::TOTAL, Abgt(j), iReAnimIdx);

										break;
									}
								}
							}
						}
						// tot -> AB 보간중이다.
						else
						{
							// tot 그룹이 어떤 그룹으로 보간 중인지 확인한다.
							for (_int j = low; j < tot; ++j)
							{
								_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, j);
								if (iCurDstInterpolationingAnimIdx == NON)
									continue;

								_int iReAnimIdx = ChangeGroups[j].second;
								if (iReAnimIdx == NON)
									continue;

								if (iCurDstInterpolationingAnimIdx != iReAnimIdx)
								{
									Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt(j));
									Ready_ChainInterpolation(Abgt::TOTAL, Abgt(j), iReAnimIdx);

								}
							}
						}
					}
					// 보간중인 그룹이 tot가 아닌 경우
					else
					{
						// 보간중인 그룹 개수가 2개 이고 보간중인 그룹이 tot가 아닌 경우 AB가 보간중이다.
						for (_int j = low; j < tot; ++j)
						{
							if (InterpolationingGroups[j].second == NON)
								continue;

							// 현재 보간중인 tot가 아닌 그룹 인덱스
							_int iCurInterpolationingGroupIdx = j;
							
							_int iCheckAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);

							// NON 이라면 tot로의 보간중이 아니다. AB 그룹이 1대1 대응 보간중이다.
							if (iCheckAnimIdx == NON)
							{
								_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), iCurInterpolationingGroupIdx);
								if (iCurDstInterpolationingAnimIdx == NON)
								{
									assert(false, "iCurDstInterpolationingAnimIdx == NON");
									return false;
								}

								_int iReAnimIdx = ChangeGroups[iCurInterpolationingGroupIdx].second;
								if (iReAnimIdx == NON)
									continue;

								if (iCurDstInterpolationingAnimIdx == iReAnimIdx)
									continue;

								Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(iCurInterpolationingGroupIdx), iReAnimIdx);

							}
							// NON이 아니라면 tot로의 보간중이다. A 그룹이 tot로의 보간중이다. 근데 변경 요청으로 2개가 들어온 상황이다. A -> tot 보간중인데 CD 이런식의 변경 요청
							else
							{
								// A -> TOT로 보간중인 애니매이션 인덱스
								_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);
								
								// 같다면 건너뛴다.
								_int iReAnimIdx = ChangeGroups[iCurInterpolationingGroupIdx].second;
								if (iReAnimIdx == iCurDstInterpolationingAnimIdx)
									continue;

								Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(iCurInterpolationingGroupIdx), iReAnimIdx);

							}
						}
					}
				}
			}
			// 보간중인 그룹에 tot가 포함되어 있지 않다.
			else
			{
				// low ~ tot -1 그룹중에 어떤 그룹이 보간중인지 확인한다.
				for (_int i = low; i < tot; ++i)
				{
					_int iCurInterpolationingGroupIdx = i;
					if (InterpolationingGroups[iCurInterpolationingGroupIdx].second == NON)
						continue;

					// iCurDstInterpolationingAnimIdx : 보간중인 그룹을 찾았다. 이제 iCurDstInterpolationingAnimIdx 그룹이 어떤 그룹으로 보간중인지 확인한다.
					// A -> tot 인지 1대1 대응 보간인지 확인한다.
					_int iCheckAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), tot);

					// 상 -> 토탈 로의 보간이 아니다. 1대1 보간중이다.
					if (iCheckAnimIdx == NON)
					{
						_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurInterpolationingGroupIdx), iCurInterpolationingGroupIdx);
						if (iCurDstInterpolationingAnimIdx == NON)
						{
							assert(false, "iCurDstInterpolationingAnimIdx == NON");
							return false;
						}

						_int iReAnimIdx = ChangeGroups[iCurInterpolationingGroupIdx].second;
						if (iCurDstInterpolationingAnimIdx != iReAnimIdx)
						{
							Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(iCurInterpolationingGroupIdx), iReAnimIdx);
						}
					}
					// 상 -> 토탈 로의 보간중이다.
					else
					{
						for (_int j = low; j < tot; ++j)
						{
							if (ChangeGroups[j].second == NON)
								continue;

							if (i == j && ChangeGroups[j].second != iCheckAnimIdx)
							{
								Reset_DstInterpolationIdx(Abgt(iCurInterpolationingGroupIdx), Abgt::TOTAL);
								Ready_ChainInterpolation(Abgt(iCurInterpolationingGroupIdx), Abgt(j), ChangeGroups[j].second);

							}
						}
					}
				}
			}
		}
	}

	// 보간중인 그룹의 개수가 1개 인경우 tot, 이거나 tot이 아닌 그룹중 하나이다.
//	if (iInterpolationingGroupCnt == 1)
//	{
//		_bool bIsCurInterpolationingGroupTotal = false;
//		for (_int i = iLower; i <= iTotal; ++i)
//		{
//			// i가 Total이고 -1이 아니라면 토탈에서 부터 보간을 시작한 것이 맞다.
//			if (i == iTotal && InterpolationingGroups[i].second != NON)
//			{
//				bIsCurInterpolationingGroupTotal = true;
//				break;
//			}
//		}
//		
//		// 보간중인 그룹의 개수가 1개이고 bIsCurInterpolationingGroupTotal가 true라면 tot 그룹이 현재 보간중에 있다는 뜻이다.
//		// 현재 tot 라는 그룹이 보간중인 경우
//		if (bIsCurInterpolationingGroupTotal)
//		{
//			// 변경 요청이 1개인 경우 tot로의 변경 요청이다. (tot 에서 보간을 시작했는데 변경 요청이 tot로의 요청인 경우)
//			// 현재 보간중인 그룹 tot에서 보간 목적지 그룹의 애니매이션을 확인하고 변경 요청이 온 애니매이션과 비교한다.
//			// 같으면 그대로 건너뛰어서 보간을 이어가도록 하고 다르다면 보보간 준비를 해준다.
//			if (iRequestCnt == 1)
//			{
//				// total로의 변경 요청이 온 애니매이션 인덱스
//				_int iReAnimIdx = ChangeGroups[iTotal].second;
//				if (iReAnimIdx == NON) // 예외 처리
//				{
//					assert(false, "tot -> tot 로의 보보간 요청인데 iReAnimIdx가 NON이다");
//					return false;
//				}
//
//				// tot 그룹이 1. [tot -> tot로의 보간 중, tot -> A or B 보간 중] 이였는지 2. [tot -> 상하] 등으로 보간 중이였는지 확인한다.
//				_int iCurInterpolationingGroupCnt = Get_Dst_Interpolationing_Cnt(Abgt(tot));
//				
//				// iCurInterpolationingGroupCnt 가 하나라 하더라도 tot -> tot 보간이 아닐 수 있다.
//
//				// 1.[tot->tot로의 보간 중, tot -> A or B 보간 중]
//				if (iCurInterpolationingGroupCnt == 1)
//				{
//					// 1개라고해서 무조건 tot -> tot는 아닐 수 있다. 그래서 확인한다.
//					// 만약 iCurDstInterpolationingAnimIdx가 NON 이라면 tot -> tot로의 보간이 아닌 경우 이다.
//					_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, tot);
//
//					// tot -> tot 보간중
//					if (iCurDstInterpolationingAnimIdx != NON)
//					{
//						if (iReAnimIdx == iCurDstInterpolationingAnimIdx)
//							return true;
//
//						Ready_ChainInterpolation(Abgt::TOTAL, Abgt::TOTAL, iReAnimIdx);
//					}
//					// tot -> A or B로 보간중
//					else
//					{
//						for (_int i = low; i < tot; ++i)
//						{
//							// 현재 보간중인 애니매이션
//							_int iCurDstInterAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, i);
//							if (iCurDstInterAnimIdx == NON)
//								continue;
//
//							// 변경 요청이 온 애니매이션과 보간 중인 애니매이션이 같으면 건너 for문 나간다.
//							if (iReAnimIdx == iCurDstInterAnimIdx)
//								break;
//
//							Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt(i));
//							Ready_ChainInterpolation(Abgt::TOTAL, Abgt(i), iReAnimIdx);
//						}
//					}
//				}
//				// 2. [tot -> 상하]
//				else
//				{
//					for (_int i = low; i < tot; ++i)
//					{
//						// iCurDstInterAnimIdx : tot 그룹에서 i그룹으로의 보간중인 애니매이션
//						_int iCurDstInterAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, i);
//						if (iCurDstInterAnimIdx == NON)
//							continue;
//
//						if (iReAnimIdx == iCurDstInterAnimIdx)
//							continue;
//
//						Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt(i));
//						Ready_ChainInterpolation(Abgt::TOTAL, Abgt::TOTAL, iReAnimIdx);
//					}
//				}
//
//#pragma region "이전 코드"
//				
//				{
//					//// Total그룹에서 기존에 보간 중이던 목록들을 임시 보관한다.
//					//vector<_int> vecSrcInterpolationIndices(iTotal + 1, -1);
//					//for (_int i = iLower; i <= iTotal; ++i)
//					//{
//					//	if (m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[i] == -1)
//					//		continue;
//					//	vecSrcInterpolationIndices[i] = m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[i];
//					//}
//
//					//// Total그룹 보간 목적지 전부 -1로 만든다.
//					//Reset_InterpolationDstIndices(Abgt(iTotal));
//
//					//// 현재 보간중인 그룹들을 순회 하면서 보간의 목적지 애니매이션과 현재 변경 요청 애니매이션을 비교해서 같다면 보보간을 하지 않고 다르다면 보보간을 수행한다.
//					//for (_int iGroupIdx = iLower; iGroupIdx <= iTotal; ++iGroupIdx)
//					//{
//					//	if (vecSrcInterpolationIndices[iGroupIdx] == -1)
//					//		continue;
//
//					//	// 보간 중이던 애니매이션과 요청 받은 애니매이션이 같은 경우 건너뛴다.
//					//	if (vecSrcInterpolationIndices[iGroupIdx] == iRequestAnimIdx)
//					//		continue;
//
//					//	Ready_ChainInterpolation(Abgt::TOTAL, Abgt(iGroupIdx), iRequestAnimIdx);
//					//}
//
//					//// true가 나온 경우 상체 하체가 같은 애니매이션 이다. 그렇기 때문에 Total로의 보간이라 수정해야한다.
//					//// - a -> bcde -> a인 경우도 있다. 그래도 상체 하체만 검사를 하면 끝난다.
//					//if (Is_TotalToTotal_Interpolationing())
//					//{
//					//	_int iDstInterpolationingAnimIdx = -1;
//					//	for (_int i = iLower; i <= iTotal; ++i)
//					//	{
//					//		if (iDstInterpolationingAnimIdx != -1)
//					//		{
//					//			Reset_InterpolationDstIndices(Abgt::TOTAL);
//					//			break;
//					//		}
//					//		if (m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[i] != -1)
//					//			iDstInterpolationingAnimIdx = m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[i];
//					//	}
//
//					//	Ready_ChainInterpolation(Abgt::TOTAL, Abgt::TOTAL, iDstInterpolationingAnimIdx);
//					//}
//				}
//
//#pragma endregion
//
//			}
//			// 요청이 온 그룹 개수가 2개 이상인 경우 (무조건 전체에서 시작)
//			// A -> B -> AB (전체 휘두르기 -> 전체 달리기 -> 상체 휘두르기 & 하체 달리기)
//			else
//			{
//				_int iReCnt = iRequestCnt; // 요청 개수 저장
//
//				for (_int i = iLower; i <= iTotal; ++i)
//				{
//					if (m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[i] == NON)
//						continue;
//
//					_int iDstInterpolationingGroupIdx = i;
//					_int iDstInterpolationingGroupAnimIdx = m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[i]; // 보간중인 애니매이션 인덱스
//
//					// A -> B -> ?? 인 경우 
//					// 보간 목적지가 total인 경우
//					if (iDstInterpolationingGroupIdx == iTotal)
//					{
//						// [][][][][A]	: 시작
//						// [][][][][B]	: 보간중
//						// [?][?][][][]	: 요청
//						// 변경 요청이 2개 이상이기 때문에 tot는 절대 포함될 수 없다.
//
//						m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[iTotal] = NON;
//
//						for (_int j = iLower; j < iTotal; ++j)
//						{
//							if (ChangeGroups[j].second == -1)
//								continue;
//
//							_int iReGroupIdx = j;
//							_int iReAnimIdx = ChangeGroups[j].second;
//
//							// [][][][][A]	: 시작
//							// [][][][][B]	: 보간중
//							// [B][?][][][]	: 요청
//							// 현재 보간중인 애니매이션과 변경 요청이 온 애니매이션 인덱스가 같은 경우 iReGroupIdx에서 
//							if (iReAnimIdx == iDstInterpolationingGroupAnimIdx)
//								Ready_ChainInterpolation(Abgt::TOTAL, Abgt(iReGroupIdx), iReAnimIdx);
//							else
//							{
//								// 현재 보간중인 애니매이션과 변경 요청이 온 애니매이션이 다른 경우 보보간을 해준다.
//								// 전체 -> iReGroupIdx 그룹의로의 보보간으로 설정한다.
//								// m_BlendGroups[iTotal].InterpolationInfo.InterpolationDstGroupIdices[iTotal] = NON;
//								Ready_ChainInterpolation(Abgt::TOTAL, Abgt(iReGroupIdx), iReAnimIdx);
//							}
//						}
//					}
//					// A -> BC -> ?? 인 경우
//					// 전체 -> 토탈이 아닌 경우 (요청이 온 것도 토탈일 수 없다. iRequestCnt가 2개 이기때문에)
//					// 현재 보간 중인 것과 요청이 온 것은 무조건 1대1 대응 관계이다.
//					else
//					{
//						if (ChangeGroups[iDstInterpolationingGroupIdx].second == NON)
//							continue;
//
//						_int iReGroupIdx = iDstInterpolationingGroupIdx;
//						_int iReAnimIdx = ChangeGroups[iDstInterpolationingGroupIdx].second;
//
//						if (iReGroupIdx != iDstInterpolationingGroupIdx)
//						{
//							assert(false, "1대1 대응관계가 아니다");
//							break;
//						}
//
//						// 보간중인 애니매이션과 변경 요청 애니매이션이 같은 경우 보보간 하지 않고 보간을 이어가도록 건너뛴다.
//						// A -> AB -> AB
//						if (iDstInterpolationingGroupAnimIdx == iReAnimIdx)
//							continue;
//
//						// A -> BC -> DE (문제 없음)
//						Ready_ChainInterpolation(Abgt::TOTAL, Abgt(iReGroupIdx), iReAnimIdx);
//
//						--iReCnt;
//					}
//				}
//			}
//		}
//		else
//		{
//			// 현재 보간 중인 그룹이 토탈인 경우가 아니다.
//			// [X][A][X][X][X] : 이런 경우 일 수 있다.
//			// 토탈이 아닌 그룹중에서 현재 보간중인 그룹을 찾고 변경 요청이 온 애니매이션과 비교하여 보보간을 수행할지 그대로 보간을 이어갈지 결정을 해야한다.
//			for (_int i = low; i < tot; ++i)
//			{
//				if (InterpolationingGroups[i].second == NON)
//					continue;
//				
//				_int iSrcInterpolationingGroupIdx = i;
//				
//				// tot가 아닌 그룹에서 tot 로의 변경 요청이다.
//				if (iRequestCnt == 1)
//				{
//					for (_int j = low; j < tot; ++j)
//					{
//						_int iCurDstInterAnimIdx = Get_Dst_InterAnimIdx(Abgt(iSrcInterpolationingGroupIdx), j);
//						if (iCurDstInterAnimIdx == NON)
//							continue;
//
//						_int iReAnimIdx = ChangeGroups[iTotal].second;
//						if (iReAnimIdx == NON)
//							continue;
//
//						// 토탈로의 변경 요청이 왔는데 현재 보간중인 애니매이션 인덱스와 일치하는 경우 보간을 수행하도록 건너 뛴다.
//						if (iCurDstInterAnimIdx == iReAnimIdx)
//							continue;
//
//						m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices[i] = -1;
//						Ready_ChainInterpolation(Abgt(i), Abgt::TOTAL, iReAnimIdx);
//						continue;
//					}
//				}
//				// 토탈로의 변경 요청이 아니다.
//				else
//				{
//					_int iReAnimIdx = ChangeGroups[iSrcInterpolationingGroupIdx].second;
//
//					// iSrcInterpolationingGroupIdx 그룹이 현재 보간 중인 같은 그룹으로의 애니매이션
//					// iCurDstInterAnimIdx가 NON이라면 같은 그룹으로의 1대1 대응 보간 중이 아니라 tot로의 보간 중이고
//					// iCurDstInterAnimIdx가 NON이 아니라면(값이 있다면) 1대1 대응 보간 중이다.
//					_int iCurDstInterAnimIdx = Get_Dst_InterAnimIdx(Abgt(iSrcInterpolationingGroupIdx), iSrcInterpolationingGroupIdx);
//					
//					// 아래의 경우 1대1 대응으로 보간중이 아니다.
//					if (iCurDstInterAnimIdx == NON)
//					{
//						_int iCurDstInterAnimIdx = Get_Dst_InterAnimIdx(Abgt(iSrcInterpolationingGroupIdx), tot);
//
//						if (iCurDstInterAnimIdx == iReAnimIdx)
//							continue;
//
//						// iSrcInterpolationingGroupIdx 그룹은 아마 tot로의 보간 중이였을 것이다.
//						Reset_DstInterpolationIdx(Abgt(iSrcInterpolationingGroupIdx), Abgt::TOTAL);
//						Ready_ChainInterpolation(Abgt(iSrcInterpolationingGroupIdx), Abgt(iSrcInterpolationingGroupIdx), iReAnimIdx);
//					}
//					// 아래의 경우 1대1 대응으로 보간 중이다.
//					else
//					{
//						if (iCurDstInterAnimIdx == iReAnimIdx)
//							continue;
//
//						Ready_ChainInterpolation(Abgt(iSrcInterpolationingGroupIdx), Abgt(iSrcInterpolationingGroupIdx), iReAnimIdx);
//					}
//
//					//// 보간중인 애니매이션과 변경 요청이 온 애니매이션이 같다면 보간을 이어가도록 건너뛴다.
//					//if (iCurDstInterAnimIdx == iReAnimIdx)
//					//	continue;
//
//					//Ready_ChainInterpolation(Abgt(iSrcInterpolationingGroupIdx), Abgt(iSrcInterpolationingGroupIdx), iReAnimIdx);
//					break;
//				}
//			}
//		}
//	}
//	// 보간중인 그룹 개수가 여러개인 경우
//	// 즉 보간 자체를 total에서 시작한 경우가 아닌 경우
//	else
//	{
//		_int iInterGroupCnt = iInterpolationingGroupCnt;
//		_int iReCnt = iRequestCnt;
//
//		// iRequestCnt가 1이라는 것은 무조건 total로의 변경 요청이다.
//		if (iRequestCnt == 1)
//		{
//			_int iRequestAnimIdx = ChangeGroups[iTotal].second;
//
//			// InterpolationingGroups를 순회하면서 현재 보간 중인 애니매이션과 비교하여 동일 하다면 보보간 하지 않고 다르면 보보간을 수행하도록 한다.
//			for (_int i = iLower; i <= iTotal; ++i)
//			{
//				if (!iInterGroupCnt)
//					break;
//
//				if (InterpolationingGroups[i].second == NON)
//					continue;
//
//				// 보간 중인 그룹을 발견했으니 iInterGroupCnt를 하나 감소시킨다.
//				--iInterGroupCnt;
//
//				_int iSrcInterpolationingGroupIdx = i; // 현재 보간 시작 점 그룹 인덱스
//				_int iSrcInterpolationingGroupAnimIdx = InterpolationingGroups[i].second; // 보간 시작 점 그룹 애니매이션 인덱스
//
//				// iSrcInterpolationingGroupIdx그룹의 보간 목적지를 순회 해준다.
//				for (_int j = iLower; j <= iTotal; ++j)
//				{
//					if (m_BlendGroups[iSrcInterpolationingGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[j] == NON)
//						continue;
//
//					_int iDstInterpolationingGroupIdx = j; // 보간 목적지 그룹 인덱스
//					_int iDstInterpolationingAnimIdx = m_BlendGroups[iSrcInterpolationingGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[j]; // 보간 목적지 애니매이션 인덱스
//
//					// 보간중인 목적지 인덱스를 찾았다. 상체 -> 상체 & 상체 -> 전체 인지 알빠 아니다. total로의 변경 요청이기 때문에 변경 요청이 온 애니매이션 인덱스와 같은지만 확인하고
//					// 만약 같다면 보보간을 하지 않고 건너 뛰고 다르다면 보보간 준비를 해준다.
//					if (iRequestAnimIdx == iDstInterpolationingAnimIdx)
//						break;
//
//					// 다르다면 현재 보간 목적지 그룹을 -1로 밀어버리고, total로의 보보간 준비를 해준다.
//					m_BlendGroups[iSrcInterpolationingGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[j] = NON;
//
//					Ready_ChainInterpolation(Abgt(iSrcInterpolationingGroupIdx), Abgt::TOTAL, iRequestAnimIdx);
//				}
//			}
//		}
//		// iRequestCnt가 2이상이라는 것은 아래 경우의 요청인 경우 이다.
//		// ab -> ?? -> ab
//		// ab -> ?? -> ac
//		// ab -> ?? -> cd
//		else
//		{
//			// 여기 왔다는 것은 보간중 이라는 뜻이다. 여기서 보보간을 할지 말지 결정해야한다.
//			for (_int i = iLower; i < iTotal; ++i)
//			{
//				if (InterpolationingGroups[i].second == NON)
//					continue;
//				
//				_int iSameGroupInterpolationAnimIdx = m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices[i];
//
//				// AB 에서 시작했는데 1대1 대응인 경우
//				if (iSameGroupInterpolationAnimIdx != -1)
//				{
//					for (_int j = iLower; j < iTotal; ++j)
//					{
//						if (m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices[j] == NON)
//							continue;
//
//						_int iDstInterpolatioingGroupIdx = j;
//						_int iDstInterpolationingAnimIdx = m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices[j];
//
//						// 같은 그룹으로의 1대1 대응 보보간일 때
//						if (ChangeGroups[iDstInterpolatioingGroupIdx].second != NON)
//						{
//							_int iReAnimIdx = ChangeGroups[iDstInterpolatioingGroupIdx].second;
//
//							// 현재 보간중인 애니매이션 인덱스와 변경 요청이 온 애니매이션 인덱스가 같은 경우 보보간을 하지 않고 보간을 이어 가도록 건너뛴다.
//							if (iDstInterpolationingAnimIdx == iReAnimIdx)
//								continue;
//
//							// 보간중인 애니매이션과 변경 요청이 온 애니매이션이 다르기 때문에 보보간을 수행한다.
//							Ready_ChainInterpolation(Abgt(i), Abgt(iDstInterpolatioingGroupIdx), iReAnimIdx);
//						}
//					}
//				}
//				// AB에서 시작했는데 1대1 대응이 아닌 경우
//				// 분명 AB -> C -> ?? 인 경우이다.
//				else
//				{
//					_int iDstInterpolationingAnimIdx = m_BlendGroups[i].InterpolationInfo.InterpolationDstGroupIdices[iTotal];
//					if (iDstInterpolationingAnimIdx == NON)
//					{
//						assert(false, "보간할 대상이 없다");
//						return false;
//					}
//
//					Reset_InterpolationDstIndices(Abgt(i));
//					_int iReAnimIdx = ChangeGroups[i].second;
//					Ready_ChainInterpolation(Abgt(i), Abgt(i), iReAnimIdx);
//				}
//			}
//		}
//	}

	
	return true;
}

_bool CModel::Process_ChainInterpolationGroups(_int iChainInterpolationingGroupCnt, _int iRequestCnt, vector<pair<Abgt, _int>>& ChainInterpolationingGroups, vector<pair<Abgt, _int>>& ChangeGroups)
{
	if (!iChainInterpolationingGroupCnt)
		return true;

	// 보보간 이상을 수행중인 그룹이 1개인 경우
	if (iChainInterpolationingGroupCnt == 1)
	{
		// 보보간 이상을 수행중인 그룹이(1개) tot인지 아닌지 먼저 확인한다.
		_bool bIsCurChainInterpolationGroupTot = false;
		if (ChainInterpolationingGroups[iTotal].second != NON)
			bIsCurChainInterpolationGroupTot = true;

		// 보보간 이상을 수행중인 그룹이 1개인데 이 그룹이 tot이다.
		if (bIsCurChainInterpolationGroupTot)
		{
			Process_When_TotGroup_ChainInterpolationing(iChainInterpolationingGroupCnt, iRequestCnt, ChainInterpolationingGroups, ChangeGroups);
		}
		// 보보간 이상을 수행중인 그룹이 tot가 아닌 그룹들 중 하나이다 (상 하 둘중 하나이다) 상하중 어떤 그룹인지 찾는다.
		else
		{
			_int iCurChainInterpolationingGroupIdx = NON; // 상하.. 중 어떤 그룹인지 찾는다.
			for (_int i = low; i < tot; ++i)
			{
				if (ChainInterpolationingGroups[i].second != NON)
				{
					iCurChainInterpolationingGroupIdx = i;
					break;
				}
			}

			// 예외처리
			if (iCurChainInterpolationingGroupIdx == NON)
			{
				assert(false, "iCurChainInterpolationingGroupIdx == NON");
				return false;
			}

			// 변경 요청이 1개인 경우
			if (iRequestCnt == 1)
			{
				_int iReGroupIdx = NON;
				for (auto Pair : ChangeGroups)
				{
					if (Pair.second != NON)
					{
						iReGroupIdx = (_int)Pair.first;
						break;
					}
				}

				_int iReAnimIdx = ChangeGroups[iReGroupIdx].second;

				_int iCurDstInterpolationGroupIdx = NON;
				for (_int i = low; i <= tot; ++i)
				{
					if (Get_Dst_InterAnimIdx(Abgt(iCurChainInterpolationingGroupIdx), i) != NON)
					{
						iCurDstInterpolationGroupIdx = i;
						break;
					}
				}
				
				// iCurChainInterpolationingGroupIdx 라는 그룹이 iCurDstInterpolationGroupIdx 그룹으로 보간중이다.
				_int iCurInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurChainInterpolationingGroupIdx), iCurDstInterpolationGroupIdx);
				if (iReAnimIdx != iCurInterpolationingAnimIdx)
				{
					Reset_DstInterpolationIdx(Abgt(iCurChainInterpolationingGroupIdx), Abgt(iCurDstInterpolationGroupIdx));
					Ready_ChainInterpolation(Abgt(iCurChainInterpolationingGroupIdx), Abgt(iReGroupIdx), iReAnimIdx, true);
				}
			}
			// iCurChainInterpolationingGroupIdx 그룹하나만 보보간 이상을 수행중인데 변경 요청이 2개 이상인 경우 상하로의 보간 요청이다.
			else
			{
				_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurChainInterpolationingGroupIdx), iCurChainInterpolationingGroupIdx);
				
				// iCurChainInterpolationingGroupIdx 그룹이 보보간 이상을 수행중인데 tot로의 보보간 이상을 수행중이다.
				if (iCurDstInterpolationingAnimIdx == NON)
				{
					iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iCurChainInterpolationingGroupIdx), tot);
					
					_int iReAnimIdx = ChangeGroups[iCurChainInterpolationingGroupIdx].second;
					if (iReAnimIdx != NON && iReAnimIdx != iCurDstInterpolationingAnimIdx)
					{
						Reset_DstInterpolationIdx(Abgt(iCurChainInterpolationingGroupIdx), Abgt::TOTAL);
						Ready_ChainInterpolation(Abgt(iCurChainInterpolationingGroupIdx), Abgt(iCurChainInterpolationingGroupIdx), iReAnimIdx, true);
					}
				}
				// iCurChainInterpolationingGroupIdx 그룹이 보보간 이상을 수행중인데 1대1 보보간 이상을 수행중이다.
				else
				{
					_int iReAnimIdx = ChangeGroups[iCurChainInterpolationingGroupIdx].second;
					if (iReAnimIdx != NON && iReAnimIdx != iCurDstInterpolationingAnimIdx)
					{
						Ready_ChainInterpolation(Abgt(iCurChainInterpolationingGroupIdx), Abgt(iCurChainInterpolationingGroupIdx), iReAnimIdx, true);
					}
				}
			}
		}
	}
	// 보보간 이상을 수행중인 그룹이 2개 이상인 경우
	else
	{
		vector<_int> vecCurChainInterpolationingGroupIndices; // 보보간 이상을 수행중인 벡터
		for (_int i = low; i <= tot; ++i)
		{
			if (ChainInterpolationingGroups[i].second != NON)
				vecCurChainInterpolationingGroupIndices.push_back(i);
		}

		// 변경 요청이 1개인 경우 tot로의 변경 요청
		if (iRequestCnt == 1)
		{
			_int iReGroupIdx = NON;
			_int iReAnimIdx = NON;
			for (auto Pair : ChangeGroups)
			{
				if (Pair.second != NON)
				{
					iReGroupIdx = Abgti(Pair.first);
					iReAnimIdx = Pair.second;
					break;
				}
			}

			for (_int iGroupIdx : vecCurChainInterpolationingGroupIndices)
			{
				for (_int i = low; i <= tot; ++i)
				{
					_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iGroupIdx), i);
					if (iCurDstInterpolationingAnimIdx == NON)
						continue;

					if (iReAnimIdx != iCurDstInterpolationingAnimIdx)
					{
						Reset_DstInterpolationIdx(Abgt(iGroupIdx), Abgt(i));
						Ready_ChainInterpolation(Abgt(iGroupIdx), Abgt(iReGroupIdx), iReAnimIdx, true);
					}
				}
			}
		}
		// 변경 요청이 2개 이상인 경우 tot로의 변경 요청이 아니다.
		else
		{
			for (_int iGroupIdx : vecCurChainInterpolationingGroupIndices)
			{
				if (iGroupIdx != tot)
				{
					_bool bFind = false;
					for (_int i = low; i <= tot; ++i)
					{
						if (bFind)
							break;

						_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt(iGroupIdx), i);
						if (iCurDstInterpolationingAnimIdx == NON)
							continue;

						for (auto Pair : ChangeGroups)
						{
							_int iReGroupIdx = (_int)Pair.first;
							_int iReAnimIdx = Pair.second;
							if (iReAnimIdx == NON)
								continue;

							if (iReAnimIdx != iCurDstInterpolationingAnimIdx && iReGroupIdx == iGroupIdx)
							{
								Reset_DstInterpolationIdx(Abgt(iGroupIdx), Abgt(i));
								Ready_ChainInterpolation(Abgt(iGroupIdx), Abgt(Pair.first), iReAnimIdx, true);
								bFind = true;
								break;
							}
						}
					}
				}
				
				// else ??


			}
		}
	}

	return true;
}

void CModel::Process_When_TotGroup_ChainInterpolationing(_int iChainInterpolationingGroupCnt, _int iRequestCnt, vector<pair<Abgt, _int>>& ChainInterpolationingGroups, vector<pair<Abgt, _int>>& ChangeGroups)
{
	// tot 그룹이 보보간 이상을 수행중인데, 변경 요청이 1개인 경우 무조건 tot에 대한 변경 요청이다.
	if (iRequestCnt == 1)
	{
		_int iReAnimIdx = ChangeGroups[iTotal].second; // 변경 요청이 온 애니매이션
		if (iReAnimIdx == NON)
			return;

		// 현재 tot 그룹이 어떤 그룹으로의 보보간 이상중인지 확인한다.
		_int iCurInterpolationingCnt = Get_Dst_Interpolationing_Cnt(Abgt(tot));

		// tot 그룹이 보보간 이상 수행 중인데 목적지 그룹이 1개인 경우, tot -> ... -> tot로의 보보간 이상중인지, tot -> ... -> A 로의 보보간 이상중인지 확인.
		if (iCurInterpolationingCnt == 1)
		{
			_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, tot);

			// tot -> ... -> tot로의 보보간 이상 수행중인 경우
			if (iCurDstInterpolationingAnimIdx != NON)
			{
				if (iReAnimIdx != iCurDstInterpolationingAnimIdx)
				{
					Ready_ChainInterpolation(Abgt::TOTAL, Abgt::TOTAL, iReAnimIdx, true);
				}
			}
			// tot -> ... -> A 로의 보보간 이상 수행중인 경우
			else
			{
				// A가 어떤 그룹인지 찾는다.
				_int iFindSingleDstInterpolationingGroupIdx = NON;
				for (_int i = low; i < tot; ++i)
				{
					if (Get_Dst_InterAnimIdx(Abgt::TOTAL, i) != NON)
					{
						iFindSingleDstInterpolationingGroupIdx = i;
						break;
					}
				}

				// 예외처리
				if (iFindSingleDstInterpolationingGroupIdx == NON || iFindSingleDstInterpolationingGroupIdx == tot)
				{
					assert(false);
					return;
				}

				_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, iFindSingleDstInterpolationingGroupIdx);
				if (iCurDstInterpolationingAnimIdx != NON && iCurDstInterpolationingAnimIdx != iReAnimIdx)
				{
					Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt(iFindSingleDstInterpolationingGroupIdx));
					Ready_ChainInterpolation(Abgt::TOTAL, Abgt::TOTAL, iReAnimIdx, true);

					for (_int i = low; i < tot; ++i)
					{
						Reset_InterpolationDstIndices(Abgt(i));
					}
				}
			}
		}
		// tot 그룹이 보보간 이상 수행 중인데 목적지 그룹이 2개 이상인 경우, tot -> ... -> AB 로의 보보간 중이다.
		else
		{
			for (_int i = low; i < tot; ++i)
			{
				if (ChangeGroups[i].second == NON)
					continue;

				_int iCurInterAnimIdx = Get_Dst_InterAnimIdx(Abgt(tot), i);

				if (iCurInterAnimIdx == ChangeGroups[i].second)
					continue;

				Ready_ChainInterpolation(Abgt::TOTAL, Abgt(i), ChangeGroups[i].second, true);
			}

			// EX-T
			{
				Process_TotGroup_Exception(ChainInterpolationingGroups, ChangeGroups);
			}
		}
	}
	// tot 그룹이 보보간 이상을 수행중인데, 변경 요청이 2개 이상인 경우
	else
	{
		// 현재 tot 그룹이 어떤 그룹으로의 보보간 이상중인지 확인한다.
		_int iCurInterpolationingCnt = Get_Dst_Interpolationing_Cnt(Abgt(tot));

		// tot 그룹이 보보간 이상 수행 중인데 목적지 그룹이 1개인 경우, tot -> ... -> tot로의 보보간 이상중인지, tot -> ... -> A 로의 보보간 이상중인지 확인.
		if (iCurInterpolationingCnt == 1)
		{
			_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, tot);

			// tot -> ... -> tot로의 보보간 이상 수행중인 경우
			if (iCurDstInterpolationingAnimIdx != NON)
			{
				for (_int i = low; i < tot; ++i)
				{
					if (ChangeGroups[i].second == NON)
						continue;

					_int iReAnimIdx = ChangeGroups[i].second;
					
					if (iReAnimIdx != iCurDstInterpolationingAnimIdx)
					{
						Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt::TOTAL);
						Ready_ChainInterpolation(Abgt::TOTAL, Abgt(i), iReAnimIdx, true);
					}
				}
			}
			// tot -> ... -> A 로의 보보간 이상 수행중인 경우
			else
			{
				// A가 어떤 그룹인지 찾는다.
				_int iFindSingleDstInterpolationingGroupIdx = NON;
				for (_int i = low; i < tot; ++i)
				{
					if (Get_Dst_InterAnimIdx(Abgt::TOTAL, i) != NON)
					{
						iFindSingleDstInterpolationingGroupIdx = i;
						break;
					}
				}

				// 예외처리
				if (iFindSingleDstInterpolationingGroupIdx == NON || iFindSingleDstInterpolationingGroupIdx == tot)
				{
					assert(false);
					return;
				}

				// AB 요청이 왔다.
				_int iReAnimIdx = ChangeGroups[iFindSingleDstInterpolationingGroupIdx].second;

				_int iCurDstInterpolationingAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, iFindSingleDstInterpolationingGroupIdx);
				if (iCurDstInterpolationingAnimIdx != NON && iCurDstInterpolationingAnimIdx != iReAnimIdx && iReAnimIdx != NON)
				{
					Reset_DstInterpolationIdx(Abgt::TOTAL, Abgt(iFindSingleDstInterpolationingGroupIdx));
					Ready_ChainInterpolation(Abgt::TOTAL, Abgt(iFindSingleDstInterpolationingGroupIdx), iReAnimIdx, true);
				}
			}
		}
		// tot 그룹이 보보간 이상 수행중인데 목적지 그룹이 2개 이상이고, 변경 요청이 2개 이상이다.
		else
		{
			_int iCurDstInterpolationAnimIdx = NON;

			// tot 그룹이 어떤 그룹들로 보보간 이상 수행중인지 그룹들을 찾는다.
			for (_int i = low; i < tot; ++i)
			{
				_int iCurDstInterpolationGroupIdx = i;
				iCurDstInterpolationAnimIdx = Get_Dst_InterAnimIdx(Abgt::TOTAL, iCurDstInterpolationGroupIdx);
				if (iCurDstInterpolationAnimIdx == NON)
					continue;

				// 같은 그룹에 변경요청이 온 애니매이션 인덱스를 확인한다.
				_int iReAnimIdx = ChangeGroups[i].second;
				if (iReAnimIdx == NON)
					continue;

				if (iReAnimIdx == iCurDstInterpolationAnimIdx)
					continue;

				Ready_ChainInterpolation(Abgt::TOTAL, Abgt(iCurDstInterpolationGroupIdx), iReAnimIdx, true);
			}

			// EX-T (예외 처리, 250216)
			{
				Process_TotGroup_Exception(iCurDstInterpolationAnimIdx, ChangeGroups);
			}

		}
	}
}

void CModel::Process_TotGroup_Exception(_int iCurTotInterpolationingAnimIdx, vector<pair<Abgt, _int>>& ChangeGroups)
{
	_int iUsingGroupCnt = 0;
	for (_int i = low; i < tot; ++i)
	{
		if (Is_Use_Condition(m_BlendGroups[i]))
			++iUsingGroupCnt;
	}

	// 사용중인 그룹이 0개 이면 확인해서 어떤 그룹 재생 해주어야할지 결정해준다.
	if (!iUsingGroupCnt)
	{
		for (_int i = low; i < tot; ++i)
		{
			if (ChangeGroups[i].second == NON) // i번 요청 그룹이 NON이면 건너뛴다.
				continue;

			_int iReGroupIdx = i;
			_int iReAnimIdx = ChangeGroups[i].second;

			// i번 요청 그룹의 애니매이션이 현재 보간중인 애니매이션과 같다면
			// A -> ... -> B | BC 경우 상체에 대한 부분을 처리 해준다.
			if (iCurTotInterpolationingAnimIdx == iReAnimIdx)
			{
				Ready_ToPlay_Anim(Abgt(iReGroupIdx), iCurTotInterpolationingAnimIdx);
			}
		}
	}
}

void CModel::Process_TotGroup_Exception(vector<pair<Abgt, _int>>& InterGroups, vector<pair<Abgt, _int>>& ChangeGroups)
{
	_int iUsingGroupCnt = 0;
	for (_int i = low; i < tot; ++i)
	{
		if (Is_Use_Condition(m_BlendGroups[i]))
			++iUsingGroupCnt;
	}

	// 사용중인 그룹이 0개 이면 확인해서 어떤 그룹 재생 해주어야할지 결정해준다.
	if (!iUsingGroupCnt)
	{
		for (_int i = low; i < tot; ++i)
		{
			if (ChangeGroups[i].second == NON) // i번 요청 그룹이 NON이면 건너뛴다.
				continue;

			_int iReGroupIdx = i;
			_int iReAnimIdx = ChangeGroups[i].second;

			// i번 요청 그룹의 애니매이션이 현재 보간중인 애니매이션과 같다면
			// A -> B | BC 경우 상체에 대한 부분을 처리 해준다.
			if (InterGroups[tot].second == iReAnimIdx)
			{
				Ready_ToPlay_Anim(Abgt(iReGroupIdx), m_BlendGroups[tot].AnimInfo);
			}
		}
	}
}

void CModel::Cache_Tot_AnimIdx()
{
	if (m_Cache_AnimIdx[tot].size() >= 3)
		m_Cache_AnimIdx[tot].clear();
	
	if (m_BlendGroups[tot].AnimInfo.bUse)
	{
		if (!m_Cache_AnimIdx[tot].empty())
			m_Cache_AnimIdx[tot].push_back(m_BlendGroups[tot].AnimInfo.iAnimIdx);
		else
		{
			if (m_Cache_AnimIdx[tot].back() != m_BlendGroups[tot].AnimInfo.iAnimIdx)
				m_Cache_AnimIdx[tot].push_back(m_BlendGroups[tot].AnimInfo.iAnimIdx);
		}
	}
}

_float CModel::Get_CurInterTrackPos()
{
	if (m_bInterpolationing)
		return m_Animations[m_iCurrentAnimIndex]->Get_CurInterTrackPos();
	return 0.f;
}

void CModel::Reset_Anim(_int iAnimIdx)
{
	if (iAnimIdx >= 0 && iAnimIdx < m_iNumAnimations && m_Animations[iAnimIdx])
	{
		m_Animations[iAnimIdx]->Reset_CurKeyFrameIndices();
		m_Animations[iAnimIdx]->Reset_Inter_Anim();
		m_Animations[iAnimIdx]->Reset_TrackPos();
	}
}

_int CModel::Get_AnimIdx(string strAnimName)
{
	for (_int i = 0; i < m_iNumAnimations; ++i)
		if (!strcmp(m_Animations[i]->Get_AnimName(), strAnimName.c_str()))
			return i;
	return -1;
}

string CModel::Get_AnimName(_int iAnimIdx)
{
	for (_int i = 0; i < m_iNumAnimations; ++i)
	{
		if (!strcmp(m_Animations[iAnimIdx]->Get_AnimName(), m_AnimationNames[i].c_str()))
			return m_AnimationNames[i];
	}
	return "None";
}

// 현재 애니매이션을 기준으로 보간을 진행한다.
// A가 끝나서 B로 보간하는 경우 A라는 애니매이션 인덱스 상태에서 B의 애니매이션으로 보간을 진행한다. bEndToFirstInterpolation가 true라면 상황자체가 현재 애니매이션이 끝나고 다음 애니매이션 끝으로 보간한다. 따라서 default를 false로 둠.
void CModel::Change_Anim(_int iDstAnimIdx, _bool bEndToFirstInterpolation)
{
	if (0 > iDstAnimIdx && iDstAnimIdx >= m_iNumAnimations)
	{
#ifdef _DEBUG
		MSG_BOX("Out Of Range iDstAnimIdx!!!");
#endif
		return;
	}

	if (m_iCurrentAnimIndex == iDstAnimIdx)
	{
#ifdef _DEBUG
		MSG_BOX("같은 애니매이션 끼리 보간 불가");
#endif
		return;
	}
	
	if (auto pCurAnim = m_Animations[m_iCurrentAnimIndex])
	{
		// 현재 애니매이션이 보간 중이라면, 즉 보간중에 보간을 한다면
		if (pCurAnim->Get_IsInterpolationing() && !m_bChainInterpolation)
		{
			// 보간 중이던 정보들을 리셋 해준다.
			m_bChainInterpolation = true; // 보보간 설정
			m_fInterTrackPos = 0.f; // 보보간 거리 재설정
			m_iInterDstAnimIdx = iDstAnimIdx;
		}
		else if (pCurAnim->Get_IsInterpolationing() && m_bChainInterpolation)
		{
			assert(false && "보보보간 안됨");
			return;
		}

		// 현재 재생중인 애니매이션이 끝난 경우에 보간하는 경우 초기화를 해준다.
		if (pCurAnim->Get_IsPlayingAnimEnd())
			pCurAnim->Set_IsPlayingAnimEnd(false);
		
		// 보보간이 아니라면 보간할 목적지 애니매이션 인덱스, 보간 거리(시간), 보간 속도 등을 재설정 해준다.
		if (!m_bChainInterpolation)
			m_iInterDstAnimIdx = iDstAnimIdx;
		
		// 보간 거리, 속도 설정
		Set_InterpolationData();

		m_fInterTrackPos = 0.f; // 보간 거리 재설정

		// 보간 중이라고 설정한다.
		if (!Get_Interpolationing())
			Set_Interpolationing(true);

		// 매틱값 받는다. 현재 애니매이션 끝에서 다음 애니매이션 처음으로 보간할지 여부. true인경우 A(끝) -> B(시작), false인경우 A의 어느 시작 점, B 끝 점
		m_bEndToFirst_Interpolation = bEndToFirstInterpolation;
	}
}

void CModel::Change_Anim(string strDstAnim, _bool bEndToFirstInterpolation)
{
	_int iDstAnimIdx = Get_AnimIdx(strDstAnim);
	if (iDstAnimIdx == -1)
		return;
	Change_Anim(iDstAnimIdx, bEndToFirstInterpolation);
}

void CModel::Set_InterpolationData()
{
	// xml 파일로 보간 정보 로드한 경우
	if (Get_LoadedInterInfo())
	{
		// 커스텀 보간 데이터를 사용하는 경우
		if (Get_UseCustumInterData())
		{
			m_fInterDis = m_fCustumInterDis;
			m_fInterSP = m_fCustumInterSP;
		}
		// XML 데이터 사용하는 경우
		else
		{
			if (!m_InterInfos[m_iCurrentAnimIndex].empty())
			{
				_bool bFind = false;
				for (auto& Pair : m_InterInfos[m_iCurrentAnimIndex])
				{
					// 목적지 애니매이션과 xml에 저장된 목적지 인덱스와 같은경우
					if (Pair.first == m_iInterDstAnimIdx)
					{
						m_fInterDis = Pair.second.first;
						m_fInterSP = Pair.second.second;
						bFind = true;
						break;
					}
				}
				if (!bFind)	// 찾지 못한 경우 기본겂을 사용한다.
				{
					m_fInterDis = m_fDefaultInterDis;
					m_fInterSP = m_fDefaultInterSP;
				}

			}
			// 비었다면 기본 값으로 설정한다.
			else
			{
				m_fInterDis = m_fDefaultInterDis;
				m_fInterSP = m_fDefaultInterSP;
			}
		}
	}
	else
	{
		m_fInterDis = m_fDefaultInterDis;
		m_fInterSP = m_fDefaultInterSP;
	}
}

void CModel::Set_InterpolationData(_int iSrcAnimIdx, _int iDstAnimIdx, INTERPOLATION_INFO& SrcInterpolationInfo)
{
	if (Get_LoadedInterInfo())
	{
		if (Get_UseCustumInterData())
		{
			SrcInterpolationInfo.fInterpolationDis = m_fCustumInterDis;
			SrcInterpolationInfo.fInterpolationSP = m_fCustumInterSP;
		}
		else
		{
			if (!m_InterInfos[iSrcAnimIdx].empty())
			{
				_bool bFind = false;
				for (auto& Pair : m_InterInfos[iSrcAnimIdx])
				{
					// 목적지 애니매이션과 xml에 저장된 목적지 인덱스와 같은경우
					if (Pair.first == iDstAnimIdx)
					{
						SrcInterpolationInfo.fInterpolationDis = Pair.second.first;
						SrcInterpolationInfo.fInterpolationSP = Pair.second.second;
						bFind = true;
						break;
					}
				}
				if (!bFind)	// 찾지 못한 경우 기본겂을 사용한다.
				{
					SrcInterpolationInfo.fInterpolationDis = m_fDefaultInterDis;
					SrcInterpolationInfo.fInterpolationSP = m_fDefaultInterSP;
				}

			}
			// 비었다면 기본 값으로 설정한다.
			else
			{
				SrcInterpolationInfo.fInterpolationDis = m_fDefaultInterDis;
				SrcInterpolationInfo.fInterpolationSP = m_fDefaultInterSP;
			}
		}
	}
	else
	{
		SrcInterpolationInfo.fInterpolationDis = m_fDefaultInterDis;
		SrcInterpolationInfo.fInterpolationSP = m_fDefaultInterSP;
	}
}

void CModel::Reset_CurAnim()
{
	if (m_Animations[m_iCurrentAnimIndex])
	{
		m_Animations[m_iCurrentAnimIndex]->Reset_Inter_Anim();
		m_Animations[m_iCurrentAnimIndex]->Reset_TrackPos();
		m_Animations[m_iCurrentAnimIndex]->Reset_CurKeyFrameIndices();
	}
}

_float CModel::Get_Anim_TickPerSecond()
{
	return m_Animations[m_iCurrentAnimIndex]->Get_TickPerSecond();
}

_float CModel::Get_Anim_Duration()
{
	return m_Animations[m_iCurrentAnimIndex]->Get_AnimDuration();
}

_float CModel::Get_Anim_CurTrackPos()
{
	return m_Animations[m_iCurrentAnimIndex]->Get_CurTrackPos();
}

_matrix CModel::Get_Bone_CombiendTransformationMatrix(string strBoneName)
{
	auto iter = find_if(m_Bones.begin(), m_Bones.end(), [&strBoneName](CBone* pBone) 
		{
			if (!strcmp(pBone->Get_Name(), strBoneName.c_str()))
				return true;
			else
				return false;
		}
	);

	if (iter != m_Bones.end())
		return (*iter)->Get_CombinedTransformationMatrix();
	return _matrix{};
}

void CModel::Set_Bone_CombinedTransformationMatrix(string strBoneName, _fmatrix CombinedMatrix)
{
	auto iter = find_if(m_Bones.begin(), m_Bones.end(), [&strBoneName](CBone* pBone)
		{
			if (!strcmp(pBone->Get_Name(), strBoneName.c_str()))
				return true;
			else
				return false;
		}
	);

	if (iter != m_Bones.end())
		(*iter)->Set_Bone_CombinedTransformationMatrix(CombinedMatrix);
	return;
}

HRESULT CModel::Bind_BoneMatrices(CShader * pShader, const _char * pConstantName, _uint iMeshIndex)
{
	return m_Meshes[iMeshIndex]->Bind_BoneMatrices(pShader, pConstantName, m_Bones);	
}

HRESULT CModel::Bind_Material(CShader * pShader, const _char * pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex)
{
	return pShader->Bind_SRV(pConstantName, m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()][eType][iTextureIndex]);
}

string CModel::Get_BoneName(_int iBoneIdx)
{
	return m_Bones[iBoneIdx]->Get_Name();
}

CBone* CModel::Get_Bone(string strBoneName)
{
	for (_int i = 0; i < m_Bones.size(); ++i)
	{
		if (!strcmp(m_Bones[i]->Get_Name(), strBoneName.c_str()))
			return m_Bones[i];
	}
	return nullptr;
}

CBone* CModel::Get_Bone(_int iBoneIdx)
{
	string strBoneName = Get_BoneName(iBoneIdx);
	return Get_Bone(strBoneName);
}

_int CModel::Get_Parent_BoneIdx(string strBoneName)
{
	auto pBone = Get_Bone(strBoneName);
	if (pBone)
		return pBone->Get_Parent_BoneIdx();
	return -1;
}

void CModel::Rotate_UpdateExcludeBone_To_Target(string strUpdateExcludeBoneName, _fmatrix ContainerObjectWorldMatrix, _fmatrix TargetWorldMatrix)
{
	if (strUpdateExcludeBoneName == "" || strUpdateExcludeBoneName == "None" || strUpdateExcludeBoneName == "none")
		return;

	_matrix SrcMatrix = ContainerObjectWorldMatrix;
	_matrix SrcMatrixInv = XMMatrixInverse(nullptr, SrcMatrix);

	// UEB(UpdateExcludeBone)
	_matrix UEBLocalMatrix = XMLoadFloat4x4(&Get_Origin_UpdateExcludeBone_TransformationMatrix());
	_vector UEBLocalPos = UEBLocalMatrix.r[3];
	_vector UEBLocalLookDir = XMVector3Normalize(UEBLocalMatrix.r[2]);

	_matrix BoneParentCombinedMatrix = Get_Parent_CombinedTransformationMatrix_From_UpdateExlcudeBone();
	_matrix BoneParentCombinedMatrixInv = XMMatrixInverse(nullptr, BoneParentCombinedMatrix);

	_matrix NewTargetMatrix = TargetWorldMatrix * SrcMatrixInv * BoneParentCombinedMatrix;
	_vector NewTargetPos = NewTargetMatrix.r[3];

	_vector vBoneLookDir_InRootLocal = XMVector3TransformNormal(UEBLocalLookDir, BoneParentCombinedMatrix);
	_vector vBonePos_InRootLocal = XMVector3TransformCoord(UEBLocalPos, BoneParentCombinedMatrix);
	_vector vBoneToTargetDir_InRootLocal = XMVector3Normalize(NewTargetPos - vBonePos_InRootLocal);

	_float fDot = XMVectorGetX(XMVector3Dot(vBoneToTargetDir_InRootLocal, vBoneLookDir_InRootLocal));
	fDot = max(-1.f, min(1.f, fDot));

	// 회전 축을 계산 (외적 사용)
	_vector vRotAxis = XMVector3Cross(vBoneLookDir_InRootLocal, vBoneToTargetDir_InRootLocal);
	_float axisLengthSq = XMVectorGetX(XMVector3LengthSq(vRotAxis));

	// Gimbal Lock 방지 처리
	if (axisLengthSq < 0.0001f)
	{
		vRotAxis = XMVector3Normalize(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
		if (fabs(XMVectorGetX(XMVector3Dot(vRotAxis, vBoneLookDir_InRootLocal))) > 0.99f)
			vRotAxis = XMVector3Normalize(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	}
	else
		vRotAxis = XMVector3Normalize(vRotAxis);

	// 회전 각도 계산
	_float fAngle = acosf(fDot);

	// 회전 행렬을 생성
	_vector QuaternionRot = XMQuaternionRotationAxis(vRotAxis, fAngle);
	_matrix RotMatrix = XMMatrixRotationQuaternion(QuaternionRot);

	// 행렬곱을 수행하여 회전 시키고 위치를 설정한다.
	_matrix RotMatrix_Local = BoneParentCombinedMatrixInv * RotMatrix * BoneParentCombinedMatrix;
	UEBLocalMatrix = UEBLocalMatrix * RotMatrix_Local;
	UEBLocalMatrix.r[3] = XMVectorSetW(UEBLocalPos, 1.0f);
	Set_UpdateExcludeBone_TransformationMatrix(UEBLocalMatrix);
}

_float CModel::Get_Anim_TickPerSecond(_int iAnimIdx)
{
	if (iAnimIdx < 0 || iAnimIdx >= m_iNumAnimations)
		return 0.f;
	return m_Animations[iAnimIdx]->Get_TickPerSecond();
}

_float CModel::Get_Anim_Duration(_int iAnimIdx)
{
	if (iAnimIdx < 0 || iAnimIdx >= m_iNumAnimations)
		return 0.f;
	return m_Animations[iAnimIdx]->Get_AnimDuration();
}

_float CModel::Get_Anim_CurTrackPos(_int iAnimIdx)
{
	if (iAnimIdx < 0 || iAnimIdx >= m_iNumAnimations)
		return 0.f;
	return m_Animations[iAnimIdx]->Get_CurTrackPos();
}

CAnimation* CModel::Get_Anim(_int iAnimIdx)
{
	if (iAnimIdx < 0 || iAnimIdx >= m_iNumAnimations)
		return nullptr;
	return m_Animations[iAnimIdx];
}

HRESULT CModel::Bind_Event(string strAnimName, string strLayerName, string strEventName, function<void()> CallBack)
{
	_int iAnimIdx = Get_AnimIdx(strAnimName);
	if (iAnimIdx == -1)
		return E_FAIL;

	auto pAnim = Get_Anim(iAnimIdx);
	if (!pAnim) 
		return E_FAIL;

	if (pAnim->Bind_Event(strLayerName, strEventName, CallBack) != S_OK)
		return E_FAIL;
	return S_OK;
}

_fmatrix CModel::Get_Parent_CombinedTransformationMatrix_From_UpdateExlcudeBone()
{
	string strUpdateExcludeBoneName = Get_UpdateExcludeBoneName();
	if (strUpdateExcludeBoneName != "None" && strUpdateExcludeBoneName != "none" && strUpdateExcludeBoneName != "")
	{
		CBone* pUpdateExcludeBone = Get_Bone(strUpdateExcludeBoneName);
		if (pUpdateExcludeBone)
		{
			_int iParentBoneIdx = pUpdateExcludeBone->Get_Parent_BoneIdx();
			if (iParentBoneIdx != -1)
				return Get_Bone_CombiendTransformationMatrix(Get_BoneName(iParentBoneIdx));
			else
				return XMMatrixIdentity();
		}
		else
			return XMMatrixIdentity();
	}
	return XMMatrixIdentity();
}

// 
// 현재 뼈들을 매개 변수로들어온 fTrackPos에 맞게 강제로 뼈들의 상태의 행렬들을 구성해주는 함수이다. -> AnimTool에서 TrackPosBar로 제어한다.
// 현재 애니매이션 Duration을 가져오고 Set_CurTrackPos로 fTrackPos를 넘겨준다. -> 현재 애니매이션 TrackPos설정
// 그럼 이제 fTrackPos에 맞는 행렬을 구성을 해주어야 해당 fTrackPos에 맞는 애니매이션이 나온다.
// 이를 위해 강제로 Update_TransformationMatrices호출을 통해 각 뼈들의 로컬 행렬들을 강제로 구성하고 마지막으로 뼈들에 대해서 Update_CombinedTrasformationMatrix를 통해 최종 행렬도 강제로 만들어준다.
void CModel::Set_CurAnim_TrackPos(_float fTrackPos)
{
	if (auto pCurAnim = Get_CurAnim())
	{
		_float fCurAnimDuration = pCurAnim->Get_AnimDuration();
		if (fTrackPos < 0.f || fTrackPos > fCurAnimDuration)
			return;

		pCurAnim->Set_CurTrackPos(fTrackPos);

		_float fTimeDelta = 0.f;

		m_Animations[m_iCurrentAnimIndex]->Update_TransformationMatrices(m_Bones, m_isLoop, fTimeDelta);


		for (auto& pBone : m_Bones)
			pBone->Update_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix), this);
	}
}

_float4x4* CModel::Get_Bone_TransformationMatrix_Ptr(string strBoneName)
{
	auto iter = find_if(m_Bones.begin(), m_Bones.end(), [&strBoneName](CBone* pBone)
		{
			if (!strcmp(pBone->Get_Name(), strBoneName.c_str()))
				return true;
			else
				return false;
		}
	);

	if (iter != m_Bones.end())
		return (*iter)->Get_TransformationMatrix_Ptr();
	return nullptr;
}

HRESULT CModel::Ready_Meshes()
{
	m_iNumMeshes = m_pAIScene->mNumMeshes;

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh*		pMesh = CMesh::Create(m_pDevice, m_pContext, m_eModelType, this, m_pAIScene->mMeshes[i], XMLoadFloat4x4(&m_PreTransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(const _char* pModelFilePath)
{
	m_iNumMaterials = m_pAIScene->mNumMaterials;

	m_Materials.resize(m_iNumMaterials);

	for (size_t i = 0; i < m_iNumMaterials; i++)
	{
		m_Materials[i].resize(AI_TEXTURE_TYPE_MAX);

		aiMaterial*		pAIMaterial = m_pAIScene->mMaterials[i];

		for (size_t j = 1; j < AI_TEXTURE_TYPE_MAX; j++)
		{
			_uint		iNumTextures = pAIMaterial->GetTextureCount(aiTextureType(j));

			for (size_t k = 0; k < iNumTextures; k++)
			{
				aiString		strTexturePath;

				if (FAILED(pAIMaterial->GetTexture(aiTextureType(j), k, &strTexturePath)))
					break;

				_char		szDrive[MAX_PATH] = "";
				_char		szDirectory[MAX_PATH] = "";
				_char		szFileName[MAX_PATH] = "";
				_char		szExt[MAX_PATH] = "";

				_char		szFullPath[MAX_PATH] = "";

				_splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDirectory, MAX_PATH, nullptr, 0, nullptr, 0);
				_splitpath_s(strTexturePath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szExt, MAX_PATH);

				strcpy_s(szFullPath, szDrive);
				strcat_s(szFullPath, szDirectory);
				strcat_s(szFullPath, szFileName);
				strcat_s(szFullPath, szExt);

				_tchar		szPerfectPath[MAX_PATH] = TEXT("");

				MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szPerfectPath, MAX_PATH);
				
				ID3D11ShaderResourceView*			pSRV = { nullptr };

				if (false == strcmp(szExt, ".dds"))
				{
					if (SUCCEEDED(CreateDDSTextureFromFile(m_pDevice, szPerfectPath, nullptr, &pSRV)))
						m_Materials[i][j].push_back(pSRV);
				}
			

				else if (false == strcmp(szExt, ".tga"))
					int a = 10;
				else 
				{
					if (SUCCEEDED(CreateWICTextureFromFile(m_pDevice, szPerfectPath, nullptr, &pSRV)))
						m_Materials[i][j].push_back(pSRV);
				}
			}			
		}
	}
	return S_OK;
}

HRESULT CModel::Ready_Bones(const aiNode * pAINode, _int iParentBoneIndex)
{
	CBone*		pBone = CBone::Create(pAINode, iParentBoneIndex);

	if (nullptr == pBone)
		return E_FAIL;

	m_Bones.push_back(pBone);

	iParentBoneIndex = m_Bones.size() - 1;

	for (size_t i = 0; i < pAINode->mNumChildren; ++i)
	{
		Ready_Bones(pAINode->mChildren[i], iParentBoneIndex);
	}

	return S_OK;
}

HRESULT CModel::Ready_Animations()
{
	m_iNumAnimations = m_pAIScene->mNumAnimations;

	for (size_t i = 0; i < m_iNumAnimations; i++)
	{
		CAnimation*		pAnimation = CAnimation::Create(m_pAIScene->mAnimations[i], this);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_AnimationNames.push_back(m_pAIScene->mAnimations[i]->mName.C_Str());	// 애니매이션 이름 추가.
		m_Animations.push_back(pAnimation);		
	}

	return S_OK;
}

CModel * CModel::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, MODEL_TYPE eModelType, const _char * pModelFilePath, _fmatrix PreTransformMatrix)
{
	CModel*	pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eModelType, pModelFilePath, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CModel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent * CModel::Clone(void * pArg)
{
	CModel*	pInstance = new CModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& pAnimation : m_Animations)
		Safe_Release(pAnimation);

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);

	m_Bones.clear();

	for (auto& Textures : m_Materials)
	{
		for (auto& SRVs : Textures)
		{
			for (auto& pSRV : SRVs)
				Safe_Release(pSRV);
			
			SRVs.clear();
		}
		Textures.clear();
	}
	m_Materials.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	m_Meshes.clear();

	m_Importer.FreeScene();

}

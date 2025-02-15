#pragma once

#include "Component.h"
#include "Animation.h" // 250125

#include <utility> // for std::pair
#include <type_traits> // for std::is_same_v
#include <array>
#include <tinyxml2.h>


namespace xml = tinyxml2;
BEGIN(Engine)

class CBone;

class ENGINE_DLL CModel final : public CComponent
{

public:
	typedef struct BlendGroupDefaultInfo
	{
		string strBoneName;				// 현재 그룹의 루트본 이름 (변경 X)
		Abgt eBlendGroupType;		// 현재 그룹의 블렌딩 타입 (변경 X)
		_int iMinBoneIdx;				// 현재 그룹에서 사용하는 애니매이션 최소 인덱스 (변경 X)
		_int iMaxBoneIdx;				// 현재 그룹에서 사용하는 애니매이션 최대 인덱스 (변경 X)
	} DEFAULT_INFO; // 250125

	typedef struct BlendGroupAnimInfo
	{
		CAnimation* pAnim;				// 현재 그룹에서 사용하는 애니매이션 이름 (변경 O)
		_int iAnimIdx;					// 현재 그룹에서 사용하는 애니매이션 인덱스 (변경 O)
		string strAnimName;				// 현재 그룹에서 사용하는 애니매이션 이름 (변경 O)
		_bool bUse = false;				// 현재 그룹에서 사용하는 애니매이션의 사용 여부 (변경 O)
		_bool bEnd = false;				// 현재 그룹에서 사용하는 애니매이션 종료 여부(매틱 변경) (변경 O)
		_bool bInterpolationing;		// 현재 그룹에서 사용하는 애니매이션 보간 여부 (변경 O)
		_bool bIsLoop = true;
		_bool bUpdate = true;			// 현재 그룹에서 사용하는 애니매이션 업데이트 여부

	} ANIM_INFO; // 250125
	
	typedef struct BlendGroupInterpolationInfo
	{
		_int InterpolationDstGroupIdices[_int(Abgt::BLEND_END)];		// 현재 그룹에서 사용하는 애니매이션 보간 목적지 인덱스 (변경 O)]
		_float fInterpolationSP = 1.f;
		_float fInterpolationDis = 0.2f;
		_float fInterpolationCurTrackPos = 0.f;
		_bool bChainInterpolation = false; // 250130
	} INTERPOLATION_INFO; // 250125

	typedef struct BlendGroup
	{
		DEFAULT_INFO DefaultInfo;
		ANIM_INFO AnimInfo;
		INTERPOLATION_INFO InterpolationInfo;

	} BLEND_GROUP; // 250125

private:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& Prototype);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const {
		return m_iNumMeshes; 
	}

	_uint Get_BoneIndex(const _char* pBoneName) const;

	const _float4x4* Get_BoneMatrix(const _char* pBoneName) const;

public:
	void SetUp_Animation(_uint iAnimIndex, _bool isLoop) {
		m_iCurrentAnimIndex = iAnimIndex;
		m_isLoop = isLoop;
	}

public:
	virtual HRESULT Initialize_Prototype(MODEL_TYPE eModelType, const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Render(_uint iMeshIndex);

public:
	// 1: DT, 2 : 루트모션 이동량 (루트 애니매이션 이동량 만큼 이동하고 싶다면 포인터로 _matrix넘겨주어 해당 객체의 월드 변환 행렬로 설정한다.
	_bool Play_Animation(_float fTimeDelta);

public:
	const vector<string>& Get_AnimNames() { return m_AnimationNames; }
	class CAnimation* Get_CurAnim();
	const vector<class CMesh*>& Get_Meshes() { return m_Meshes; }
	MODEL_TYPE Get_ModelType() { return m_eModelType; }

public:
	HRESULT Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
	HRESULT Bind_Material(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex = 0);

private:
	Assimp::Importer			m_Importer;
	const aiScene*				m_pAIScene = { nullptr };
	MODEL_TYPE					m_eModelType = { TYPE_END };

private:
	_uint						m_iNumMeshes = { 0 };
	vector<CMesh*>				m_Meshes;

private:
	vector<CBone*>				m_Bones;


private:
	_float4x4					m_PreTransformMatrix = {};

private:
	_uint												m_iNumMaterials = {};
	vector<vector<vector<ID3D11ShaderResourceView*>>>	m_Materials;

private:
	_uint						m_iCurrentAnimIndex = {};
	_bool						m_isLoop = { false };
	_uint						m_iNumAnimations = {};
	vector<CAnimation*>			m_Animations;
	_bool						m_isFinished = { false };


	// ================================================================================
	// NY
	//  - NY 250116
	// ----------------------------------------------------------
public:
	HRESULT Load_Anim_InterXML(const WCHAR* strXMLFilePath);
	HRESULT Load_Anim_RootMotionBoneXML(const WCHAR* strXMLFilePath);
	HRESULT Load_BlendBones(const WCHAR* strXMLFilePath); // 250125
	HRESULT Load_AnimEvents(const WCHAR* strXMLFilePath); // 250202

private:
	HRESULT Bind_AnimEvents();	// 250202

public:
	void Init_BlendGroup_DefaultInfo(Abgt eBlendGroupType, DEFAULT_INFO DefaultInfo);
	void Init_BlendGroup_AnimInfo(Abgt eBlendGroupType, ANIM_INFO AnimInfo);
	void Init_BlendGroup_InterpolationInfo(Abgt eBlendGroupType, INTERPOLATION_INFO InterpolationInfo);

	void SetUp_Animation(string strAnimName, _bool isLoop);

	_bool Update_Interpolation_Animation(_float fTimeDelta); //250124
	void Finialize_Interpolation(); //250124
	_bool Update_Animation(_float fTimeDelta); //250124
	void Update_CheckAnim_End(); //250124
	void Update_Bone_CombinedTransformationMatrix(); //250124

	// -------------------------------------------------------------
	// 플레이어 전용
	void Play_Animation(_float fTimeDelta, _bool bPlayer); // 250126
	_bool Update_Interpolation_Animation(_float fTimeDelta, _bool bPlayer); // 250126
	void Finialize_Interpolation(_bool bPlayer); // 250126
	void Update_Animation(_float fTimeDelta, _bool bPlayer); // 250126
	void Update_CheckAnim_End(_bool bPlayer); // 250126
	void Update_Bone_CombinedTransformationMatrix(_bool bPlayer); // 250126
	void Reset_BlendGroupBone(Abgt eGroupType);

	// 250125
	template <typename... Blend_Group_Pairs>
	void Set_BlendGroup(Blend_Group_Pairs... BlendGroupPairs)
	{
		// 모든 Blend_Group_Pairs가 std::pair<Abgt, _int>인지 정적 타입 확인
		static_assert((std::is_same_v<Blend_Group_Pairs, std::pair<Abgt, _int>> && ...),
			"All arguments must be of type std::pair<Abgt, _int>");

		array<pair<Abgt, _int>, sizeof...(BlendGroupPairs)> BlendGroupArray = { BlendGroupPairs... };

		// 배열의 크기 확인
		if (BlendGroupArray.size() >= 2) // 예외 처리
		{
			// TOTAL이 포함되어 있는지 검사
			const _bool hasTotal = std::any_of(BlendGroupArray.begin(), BlendGroupArray.end(), [](const auto& pair) 
				{
					return pair.first == Abgt::TOTAL;
				});

			// 매개 변수가 2개 이상인데 TOTAL이 있다면 assert
			assert(hasTotal != true);
		}

		if (BlendGroupArray.size() == 1)
		{
			assert(BlendGroupArray.front().first == Abgt::TOTAL);
		}

		// 배열 순회하며 각 pair 처리

		for (const auto& blendGroup : BlendGroupArray)
		{
			if ((_int)blendGroup.first < 0) return;
			auto pAnim = m_Animations[blendGroup.second];
			if (!pAnim) return;
			Init_BlendGroup_AnimInfo(blendGroup.first, CModel::ANIM_INFO{ pAnim, blendGroup.second, pAnim->Get_AnimName(), true, false });
		}
	}

	// 애니매이션 변경시 호출하는 함수
	template <typename... Change_Group_Pairs>
	_bool Change_Anim(Change_Group_Pairs... ChangeGroupPairs)
	{
		// 모든 Blend_Group_Pairs가 std::pair<Abgt, _int>인지 정적 타입 확인
		static_assert((std::is_same_v<Change_Group_Pairs, std::pair<Abgt, _int>> && ...),
			"All arguments must be of type std::pair<Abgt, _int>");

		array<pair<Abgt, _int>, sizeof...(ChangeGroupPairs)> ChangeGroupArray = { ChangeGroupPairs... };

		// 1. 예외 처리 : 변경 요청 그룹의 사이즈가 2이상인데 Total이 포함되어 있는지 검사한다.
		{
			// 사이즈가 2개 이상일 때 TOTAL이 포함되어 있는지 확인한다. (예외처리 하지 않으면 total 재생, 다른 부위(Upper)재생시 Upper가 두배의 속도로 애니매이션이 재생된다. 이를 방지 하기 위함
			if (ChangeGroupArray.size() >= 2)
			{
				// 토탈이 있는지 예외 검사
				{
					_bool bNotFindTotal = true;

					for (_int i = 0; i < ChangeGroupArray.size(); ++i)
					{
						if (ChangeGroupArray[i].first == Abgt::TOTAL)
						{
							bNotFindTotal = false;
							break;
						}
					}

					if (!bNotFindTotal) // total이 포함될 수 없다.
					{
#ifdef _DEBUG
						assert(false && "사이즈가 2이상인데 TOTAL이 포함됨!");
#endif
						return false;
					}
				}
			}
		}

		// 2. 예외 처리 : 변경 요청 그룹의 사이즈가 1인데 Total인지 아닌지 검사한다. (Total이 아니라면 요청을 잘 못한 경우 튕겨낸다)
		{
			if (ChangeGroupArray.size() == 1)
			{
				_int iGroupIdx = (_int)ChangeGroupArray.front().first;
				if (iGroupIdx != iTotal)
				{
#ifdef _DEBUG
					assert(false, "요청한 그룹이 1개 인데 TOTAL이 아닙니다");
#endif
					return false;
				}
			}
		}

		// 3. 예외 처리 : 변경 요청 그룹에서 중복된 요청이 있는지 확인한다. (AA, BB, CCC 이런식으로 요청한 경우의 처리)
		{
			static vector<pair<Abgt, _bool>> vecCheck(_int(Abgt::BLEND_END), make_pair(Abgt::NONE, false));

			for (_int i = iLower; i <= iTotal; ++i)
				vecCheck[i] = make_pair(Abgt::NONE, false);

			for (_int i = 0; i < ChangeGroupArray.size(); ++i)
			{
				_int iGroupIdx = (_int)ChangeGroupArray[i].first;

				if (!vecCheck[iGroupIdx].second)
					vecCheck[iGroupIdx].second = true;
				else
				{
#ifdef _DEBUG
					assert(false, "변경 요청 그룹에 중복된 그룹 항목이 있습니다");
#endif
					return false;
				}
			}
		}
		
		static vector<pair<Abgt, _int>> ChangeGroups(_int(Abgt::BLEND_END), make_pair(Abgt::NONE, -1)); // 변경 요청이 온 벡터
		static vector<pair<Abgt, _int>> UsingGroups(_int(Abgt::BLEND_END), make_pair(Abgt::NONE, -1)); // 사용중인 그룹 벡터
		static vector<pair<Abgt, _int>> InterpolationingGroups(_int(Abgt::BLEND_END), make_pair(Abgt::NONE, -1)); // 보간 중인 그룹 벡터
		static vector<pair<Abgt, _int>> ChainInterpolationingGroups(_int(Abgt::BLEND_END), make_pair(Abgt::NONE, -1)); // 보보간 이상 중인 그룹 벡터

		for (_int i = iLower; i <= iTotal; ++i)
			ChangeGroups[i] = make_pair(Abgt::NONE, -1);

		// 변경 요청 그룹 개수 카운팅
		_int iRequestCnt = 0;
		for (auto Pair : ChangeGroupArray)
		{
			ChangeGroups[(_int)Pair.first] = make_pair(Pair.first, Pair.second);
			if (ChangeGroups[(_int)Pair.first].second != -1)
				++iRequestCnt;
		}

		for (_int i = iLower; i <= iTotal; ++i)
		{
			UsingGroups[i] = make_pair(Abgt::NONE, -1);
			InterpolationingGroups[i] = make_pair(Abgt::NONE, -1);
			ChainInterpolationingGroups[i] = make_pair(Abgt::NONE, -1);
		}

		// 현재 사용중인 그룹 개수 카운팅, 보간 중인 그룹 개수 카운팅, 보보간 이상의 그룹 개수 카운팅
		_int iUsingCnt = 0, iInterpolationingCnt = 0, iChainInterpolationingCnt = 0;

		// 조건에 따라 각 벡터에 담기
		for (_int i = iLower; i <= iTotal; ++i)
		{
			if (Is_Use_Condition(m_BlendGroups[i])) // 사용중인 조건이라면
			{
				++iUsingCnt;
				UsingGroups[i] = make_pair(Abgt(i), m_BlendGroups[i].AnimInfo.iAnimIdx);
			}
			if (Is_Interpolation_Condition(m_BlendGroups[i])) // 보간 중인 조건이라면
			{
				++iInterpolationingCnt;
				InterpolationingGroups[i] = make_pair(Abgt(i), m_BlendGroups[i].AnimInfo.iAnimIdx);
			}
			if (Is_ChainInterpolation_Condition(m_BlendGroups[i])) // 보보간 이상 조건이라면
			{
				++iChainInterpolationingCnt;
				ChainInterpolationingGroups[i] = make_pair(Abgt(i), m_BlendGroups[i].AnimInfo.iAnimIdx);
			}
		}

		// 4. 예외 처리 : 유효한 그룹의 개수 카운팅 (개수가 같다면 유효한 그룹이 하나도 없다고 판단할 수 있다. 문제가 있는 상황이다)
		{
			_int iMaxValidNum = (iTotal - iLower + 1) * 3;
			_int iValidSum = 0;
			for (_int i = iLower; i <= iTotal; ++i)
			{
				if (UsingGroups[i].second == -1)
					++iValidSum;
				if (InterpolationingGroups[i].second == -1)
					++iValidSum;
				if (ChainInterpolationingGroups[i].second == -1)
					++iValidSum;
			}

			if (iValidSum == iMaxValidNum)
			{
#ifdef _DEBUG
				assert(false, "iMaxValidNum과 iValidSum가 일치, 유효한 그룹의 개수가 하나도 없다");
#endif
				return false;
			}
		}
		


#pragma region "보보간 이상인 그룹들 처리"
		



#pragma endregion

		Process_ChainInterpolationGroups(iChainInterpolationingCnt, iRequestCnt, ChainInterpolationingGroups, ChangeGroups);

#pragma region "보간중인 그룹 처리"

		// ==============================================================================================================
		// 
		// InterpolationingGroups
		// 
		// [X][X][X][X][A] : Src
		// 
		//		||
		// 
		// [A][B][X][X][X] : Src (보간중)
		// [A][C][X][X][X] : Dst (요청)
		// 
		//		||
		// 
		// [A][C][X][X][X] : Src A그룹은 보보간하지 않고, B 그룹은 보간을 한다.
		// 
		// 
		// 
		// ==============================================================================================================

#pragma endregion

		// Process_UsingGroups에서 처리 되어서 보간 중인 그룹들에 대해서 조건에 맞는 경우 보보간 처리를 한다.
		Process_InterpolationingGroups(iInterpolationingCnt, iRequestCnt, InterpolationingGroups, ChangeGroups);


#pragma region "UsingGroup 동작 방식 설명"
		// =======================================================================================================================================================
		// 보간 Desc
		// =======================================================================================================================================================
		// 
		// ---------------------------------------------
		// 1. iUsingCnt가 1개 인 경우
		// - 1-1. 전체 -> 전체
		// 
		// 아래 경우 넘긴다.
		// [X][X][X][X][A] : Src
		// [X][X][X][X][A] : Dst
		// 
		// 아래 경우 보간한다.
		// [X][X][X][X][A] : Src
		// [X][X][X][X][B] : Dst
		// 
		// 
		// - 1-2. 전체 -> 상하
		// 
		// 아래 경우 상체 보간하지 않고 애니매이션 재생하고 하체의 경우 보간한다. 하체의 보간이 끝나면 TrackPos를 계산한 것과 Total에 하체의 애니매이션을 같이 넘겨준다
		// [X][X][X][X][A] : Src
		// [A][B][X][X][X] : Dst
		//
		// [X][X][X][X][B] : Src
		// [A][B][X][X][X] : Dst
		// ---------------------------------------------
		// 
		// 
		// 
		// ---------------------------------------------
		// 2. iUsingCnt가 2개 이상인 경우
		// - 2-1. 상하 -> 전체
		// 
		// 아래의 경우 상체는 보간하지 않고 하체는 보간을 한다. 하체의 보간이 끝나고 나면 Total그룹에 하체의 애니매이션을 넘겨주고 상체, 하체는 사용불가로 설정한다.
		// [A][B][X][X][X] : Src
		// [X][X][X][X][A] : Dst
		// 
		// 아래의 경우에는 그냥 보간을 수행한다.
		// [A][B][X][X][X] : Src
		// [X][X][X][X][C] : Dst 
		// 
		// 
		// - 2-2. 상하 -> 상하
		//	
		// 아래의 경우 각 그룹에 대해서 보간을 수행한다.
		// [A][B][X][X][X] : Src
		// [C][D][X][X][X] : Dst 
		// 
		// 
		// 아래의 경우 같은 그룹에 해당하는 애니매이션은 보간하지 않고 다른 경우에만 보간을 수행한다.
		// [A][B][X][X][X] : Src
		// [A][D][X][X][X] : Dst 
		// ---------------------------------------------
		// 
		// 
		// =======================================================================================================================================================
		// 보간 Desc
		// =======================================================================================================================================================
#pragma endregion

		// 사용중인 그룹들 처리를 한다.
		// 보간도 아니고 보보간이상인 경우도 아닌 그룹들은 조건에 맞는 경우 보간 처리를 한다.
		Process_UsingGroups(iUsingCnt, iRequestCnt, UsingGroups, ChangeGroups);

		{
			//// 전체 -> 전체
		//// 전체 -> 상하
		//if (iUsingCnt == 1)
		//{
		//	// 전체 -> 전체
		//	if (iRequestCnt == 1)
		//	{
		//		_int iRequestChangeAnimIdx = ChangeGroups[iTotal].second;
		//		_int iCurUsingGroupAnimIdx = UsingGroups[iTotal].second;

		//		if (iCurUsingGroupAnimIdx == iRequestChangeAnimIdx)
		//			return;

		//		// 같은 경우가 아니기 때문에 보간한다.
		//		Ready_Interpolation(Abgt::TOTAL, Abgt::TOTAL, iRequestChangeAnimIdx);
		//		Cal_Interpolation_KeyFrame(iRequestChangeAnimIdx, m_BlendGroups[iTotal].InterpolationInfo);
		//	}
		//	// 전체 -> 상하(그 이상)
		//	else
		//	{
		//		_int iCurUsingGroupAnimIdx = UsingGroups[iTotal].second;

		//		for (_int iGroupIdx = iLower; iGroupIdx < iTotal; ++iGroupIdx)
		//		{
		//			_int iRequestAnimIdx = ChangeGroups[iGroupIdx].second;
		//			if (iRequestAnimIdx == -1)
		//				continue;

		//			// 전체에 해당하는 애니매이션이 요청이 온 애니매이션과 같은 경우
		//			// iGroupIdx에 해당하는 그룹은 애니매이션 재생을 하도록 한다.
		//			if (iCurUsingGroupAnimIdx == iRequestAnimIdx)
		//			{
		//				Ready_ToPlay_Anim(Abgt(iGroupIdx), m_BlendGroups[iTotal].AnimInfo);
		//			}
		//			// 다르다면 Total에서 보간을 하도록 설정한다.
		//			else
		//			{
		//				Ready_Interpolation(Abgt::TOTAL, Abgt(iGroupIdx), iRequestAnimIdx);
		//				Cal_Interpolation_KeyFrame(iRequestAnimIdx, m_BlendGroups[iTotal].InterpolationInfo);
		//			}
		//		}
		//	}
		//	
		//}
		//// 상하 -> 전체
		//// 상하 -> 상하
		//else
		//{
		//	// 상하 -> 전체
		//	if (iRequestCnt == 1)
		//	{
		//		_int iRequestAnimIdx = ChangeGroups[iTotal].second;

		//		for (_int iGroupIdx = iLower; iGroupIdx < iTotal; ++iGroupIdx)
		//		{
		//			_int iCurGroupAnimIdx = UsingGroups[iGroupIdx].second;
		//			if (iCurGroupAnimIdx == -1)
		//				continue;

		//			// total에 요청이 온 애니매이션 인덱스와 현재 그룹 Lower~Head 까지의 애니매이션 인덱스가 일치하는게 있다면 해당 그룹은 보간하지 않고 특정 그룹의 보간이 끝나면 total로 재생한다
		//			if (iCurGroupAnimIdx == iRequestAnimIdx)
		//				continue;

		//			Ready_Interpolation(Abgt(iGroupIdx), Abgt::TOTAL, iRequestAnimIdx);
		//			Cal_Interpolation_KeyFrame(iRequestAnimIdx, m_BlendGroups[iGroupIdx].InterpolationInfo);
		//		}
		//	}
		//	// 상하 -> 상하(그 이상)
		//	// 아래의 경우 무조건 1대1 대응이다.
		//	else
		//	{
		//		for (_int iGroupIdx = iLower; iGroupIdx < iTotal; ++iGroupIdx)
		//		{
		//			_int iCurGroupUsingAnimIdx = UsingGroups[iGroupIdx].second;
		//			if (iCurGroupUsingAnimIdx == -1)
		//				continue;

		//			// 요청이 온 애니매이션과 현재 같은 그룹의 애니매이션이 같다면 건너뛴다.
		//			_int iRequestAnimIdx = ChangeGroups[iGroupIdx].second;
		//			if (iRequestAnimIdx == iCurGroupUsingAnimIdx)
		//				continue;

		//			Ready_Interpolation(Abgt(iGroupIdx), Abgt(iGroupIdx), iRequestAnimIdx);
		//			Cal_Interpolation_KeyFrame(iRequestAnimIdx, m_BlendGroups[iGroupIdx].InterpolationInfo);
		//		}
		//	}
		//}
		}
		
//		// UsingGroups : 보간 처리
//		for (_int iRequestGroupIdx = iLower; iRequestGroupIdx <= iTotal; ++iRequestGroupIdx)
//		{
//			// 변경 요청이 온 애니매이션 인덱스가 -1이다 == iRequestGroupIdx에 요청이 온 애니매이션이 없다. == 건너 뛴다.
//			_int iRequestChangeAnimIdx = ChangeGroups[iRequestGroupIdx].second;
//			if (iRequestChangeAnimIdx == -1)
//				continue;
//
//			// ----------------------------------- 현재 변경 요청이 온 그룹이 토탈이 아닌 경우 ---------------------------------------------
//			if (iRequestGroupIdx != iTotal)
//			{
//				// Lower그룹의 애니매이션 인덱스가 -1이 아니라는 뜻은 상하 -> 상하의 경우이다.
//				// 변경 요청이 토탈이 아니고 사용중인 그룹이 하체인 경우
//				// [A][B][X][X][X]
//				// [?][?][?][?][?]
//				// 상하 -> 상하 : 1대1 대응의 애니매이션 전환 로직이 필요하다.
//				if (UsingGroups[iLower].second != -1)
//				{
//					_int iCurGroupAnimIdx = UsingGroups[iRequestGroupIdx].second; // iRequestGroupIdx 그룹과 같은, 현재 사용중인 애니매이션 인덱스
//
//					if (iCurGroupAnimIdx == -1)
//					{
//#ifdef _DEBUG
//						assert(false, "UsingGroups[iRequestGroupIdx].second는 -1일 수 없다");
//#endif
//						return;
//					}
//
//					if (iCurGroupAnimIdx == iRequestChangeAnimIdx) // 같다면 보간할 필요 없고 건너 뛴다.
//						continue;
//
//					Ready_Interpolation(Abgt(iRequestGroupIdx), Abgt(iRequestGroupIdx), iRequestChangeAnimIdx);
//					Cal_Interpolation_KeyFrame(iRequestChangeAnimIdx, m_BlendGroups[iRequestGroupIdx].InterpolationInfo);
//				}
//				// 상하 -> 토탈 : 
//				else
//				{
//					_int iCurGroupAnimIdx = UsingGroups[iGroupIdx].second;
//
//					// 현재 사용중인 그룹이 토탈인데 [상체, 하체, 왼팔, 오른팔, 머리] 중 일치하는 애니매이션이 존재하는 경우
//					// 토탈의 애니매이션을 해당 그룹에 넘겨주고 일치하지 않는 부분은 보간을 수행해야한다.
//					// A -> AB의 경우 & A -> BA
//					if (iCurGroupAnimIdx == iRequestChangeAnimIdx)
//					{
//						Ready_ToPlay_Anim(Abgt(iRequestGroupIdx), );
//
//
//					}
//					// A -> BC의 경우
//					else
//					{
//						Ready_Interpolation(Abgt(iGroupIdx), Abgt(iRequestGroupIdx), iRequestChangeAnimIdx);
//						Cal_Interpolation_KeyFrame(iRequestChangeAnimIdx, m_BlendGroups[iGroupIdx].InterpolationInfo);
//
//					}
//
//				}
//
//
//				// UsingGroup을 순회하면서 조건에 맞는지 살펴본다.
//				for (_int iGroupIdx = iLower; iGroupIdx <= iTotal; ++iGroupIdx)
//				{
//					if (UsingGroups[iGroupIdx].second == -1)
//
//					// 토탈 -> 상하
//					if (iGroupIdx == iTotal)
//					{
//						if (!iRequestCnt)
//							break;
//
//						if (UsingGroups[iGroupIdx].second == -1)
//							continue;
//
//						_int iCurGroupAnimIdx = UsingGroups[iGroupIdx].second;
//
//						// 현재 사용중인 그룹이 토탈인데 [상체, 하체, 왼팔, 오른팔, 머리] 중 일치하는 애니매이션이 존재하는 경우
//						// 토탈의 애니매이션을 해당 그룹에 넘겨주고 일치하지 않는 부분은 보간을 수행해야한다.
//						// A -> AB의 경우 & A -> BA
//						if (iCurGroupAnimIdx == iRequestChangeAnimIdx)
//						{
//							m_BlendGroups[iRequestGroupIdx].AnimInfo.bUse = true;
//							m_BlendGroups[iRequestGroupIdx].AnimInfo.bInterpolationing = false;
//							if (m_BlendGroups[iRequestGroupIdx].AnimInfo.bEnd)
//								m_BlendGroups[iRequestGroupIdx].AnimInfo.bEnd = false;
//
//							m_BlendGroups[iRequestGroupIdx].AnimInfo.pAnim = m_BlendGroups[iGroupIdx].AnimInfo.pAnim;
//							m_BlendGroups[iRequestGroupIdx].AnimInfo.iAnimIdx = m_BlendGroups[iGroupIdx].AnimInfo.iAnimIdx;
//							m_BlendGroups[iRequestGroupIdx].AnimInfo.strAnimName = m_BlendGroups[iGroupIdx].AnimInfo.strAnimName;
//
//							if (m_BlendGroups[iRequestGroupIdx].InterpolationInfo.bChainInterpolation)
//								m_BlendGroups[iRequestGroupIdx].InterpolationInfo.bChainInterpolation = false;
//
//							m_BlendGroups[iRequestGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
//							
//							Reset_InterpolationDstIndices(Abgt(iRequestGroupIdx));
//						}
//						// A -> BC의 경우
//						else
//						{
//							Ready_Interpolation(Abgt(iGroupIdx), Abgt(iRequestGroupIdx), iRequestChangeAnimIdx);
//							Cal_Interpolation_KeyFrame(iRequestChangeAnimIdx, m_BlendGroups[iGroupIdx].InterpolationInfo);
//
//						}
//
//						--iRequestCnt;
//					}
//					// 상하 -> 상하
//					// 아래 else의 경우 무조건 1대1 대응 관계이다.
//					else
//					{
//						iUsingCnt != 0 ? --iUsingCnt : iUsingCnt = 0;
//
//						_int iCurGroupAnimIdx = UsingGroups[iSrcIncreaseGroupIdx].second; // iRequestGroupIdx 그룹과 같은, 현재 사용중인 애니매이션 인덱스
//						if (iCurGroupAnimIdx == -1) // 예외 처리
//						{
//							++iSrcIncreaseGroupIdx;
//							continue;
//						}
//
//						if (iCurGroupAnimIdx == iRequestChangeAnimIdx) // 같다면 보간할 필요 없고 건너 뛴다.
//						{
//							++iSrcIncreaseGroupIdx;
//							continue;
//						}
//
//						// iGroupIdx와 iRequestGroupIdx는 같다.
//						Ready_Interpolation(Abgt(iSrcIncreaseGroupIdx), Abgt(iSrcIncreaseGroupIdx), iRequestChangeAnimIdx);
//						Cal_Interpolation_KeyFrame(iRequestChangeAnimIdx, m_BlendGroups[iSrcIncreaseGroupIdx].InterpolationInfo);
//
//						++iSrcIncreaseGroupIdx;
//						break;
//					}
//				}
//			}
//			// ----------------------------------- 현재 변경 요청이 온 그룹이 토탈인 경우 ---------------------------------------------
//			else
//			{
//				_int iUseCnt = 0;
//				for (_int i = iLower; i <= iTotal; ++i)
//				{
//					if (m_BlendGroups[i].AnimInfo.bUse)
//						++iUseCnt;
//				}
//
//				// 현재 사용중인 그룹이 2개 이상인 경우 : 상하 -> 토탈 (상 & 하 & 머리 -> 토탈 등의 경우)
//				if (iUseCnt >= 2)
//				{
//					_int iInterAnimIdx = -1;
//
//					for (_int iGroupIdx = iLower; iGroupIdx <= iTotal; ++iGroupIdx)
//					{
//						if (UsingGroups[iGroupIdx].second == -1)
//							continue;
//
//						// 요청이 온 애니매이션과 사용중인 그룹의 애니매이션이 같다면 보간하지 않는다.
//						if (UsingGroups[iGroupIdx].second == iRequestChangeAnimIdx)
//						{
//							// 다른 그룹에서 보간할 애니매이션 인덱스를 저장한다.
//							iInterAnimIdx = iRequestChangeAnimIdx;
//						}
//						// 요청이 온 애니매이션과 사용중인 그룹의 애니매이션이 다르기 때문에 보간한다.
//						else
//						{
//							Ready_Interpolation(Abgt(iGroupIdx), Abgt(iRequestGroupIdx), iRequestChangeAnimIdx);
//							Cal_Interpolation_KeyFrame(iRequestChangeAnimIdx, m_BlendGroups[iGroupIdx].InterpolationInfo);
//						}
//					}
//				}
//				// 현재 사용중인 그룹이 2개 미만인 경우 : 토탈 - > 토탈
//				else
//				{
//					_int iCurGroupAnimIdx = UsingGroups[iRequestGroupIdx].second;
//					if (iCurGroupAnimIdx == iRequestChangeAnimIdx) // 요청이 온 애니매이션과 같다면 건너 뛴다.
//						continue;
//					
//					Ready_Interpolation_TotalToTotal(iRequestChangeAnimIdx);
//					Cal_Interpolation_KeyFrame(iRequestChangeAnimIdx, m_BlendGroups[iRequestGroupIdx].InterpolationInfo);
//				}
//			}
//		}
//
//		
//		

#pragma region "이전 코드"

		
			// 사용중인 현재 그룹을 돌면서 보간할 것이 있는지 살펴본다.
			// - 현재 사용중인 그룹의 애니매이션과 변경 요청이 온 같은 그룹의 애니매이션이 있는지 확인하고, 있는 경우에 애니매이션이 다르다면 '보간'하고 같다면 그냥 넘긴다.
			// - 현재 사용중인 그룹과 변경 요청이 온 그룹이 다른경우, 변경 요청이 온 그룹의 애니매이션과 현재 그룹의 애니매이션이 같다면 현재 애니매이션 정보를 변경 요청이온 그룹에게 그대로 전달하고, 변경 요청이 그룹의 애니매이션과 현재 그룹의 애니매이션이 다르다면 보간한다.
			//for (_int iCurGroupIdx = iLower; iCurGroupIdx <= iTotal; ++iCurGroupIdx)
			//{
			//	// 보간 중이 아니고 보보간이상인 경우도 아닌 경우
			//	// 즉 '이제 막 Change_Anim을 호출해서 보간을 하려는 상황' 이다.
			//	if (m_BlendGroups[iCurGroupIdx].AnimInfo.bUse && !m_BlendGroups[iCurGroupIdx].AnimInfo.bInterpolationing)
			//	{
			//		// 그냥 보간의 조건이 되는지 먼저 확인한다.
			//		
			//		// 같은 그룹에 해당하는 Pair를 가져온다.
			//		auto SameGroupChangePair = ChangeGroup[(_int)m_BlendGroups[iCurGroupIdx].DefaultInfo.eBlendGroupType];
			//		if (SameGroupChangePair.second != -1) // 같은 그룹에 변경할 애니매이션이 존재한다.
			//		{
			//			if (m_BlendGroups[iCurGroupIdx].AnimInfo.iAnimIdx != SameGroupChangePair.second) // 애니매이션이 다른경우 -> 보간한다
			//			{
			//				m_BlendGroups[iCurGroupIdx].AnimInfo.bUse = false;
			//				m_BlendGroups[iCurGroupIdx].AnimInfo.bInterpolationing = true;
			//				m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[(_int)SameGroupChangePair.first] = SameGroupChangePair.second;
			//				m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation = false;
			//				m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
			//				Set_InterpolationData(m_BlendGroups[iCurGroupIdx].AnimInfo.iAnimIdx, SameGroupChangePair.second, m_BlendGroups[iCurGroupIdx].InterpolationInfo);

			//				if ((_int)SameGroupChangePair.first != iTotal)
			//					ChangeGroup[(_int)SameGroupChangePair.first] = make_pair(Abgt::NONE, -1); // 같으면 그냥 넘긴다.

			//				continue;
			//			}
			//			// 같은 그룹이고 애니매이션도 같은 경우 보간하지 않는다.
			//			else
			//			{
			//				if ((_int)SameGroupChangePair.first != iTotal)
			//					ChangeGroup[(_int)SameGroupChangePair.first] = make_pair(Abgt::NONE, -1); // 같으면 그냥 넘긴다.
			//				continue; // 같은 그룹인데 애니매이션 인덱스도 같은데 보간 요청이 온 경우 continue를 해서 건너 뛰어준다.
			//			}
			//		}
			//		
			//		// 같은 그룹에서 애니매이션 변경이 발생하지 않은 경우
			//		// - 변경 요청이 온 다른 그룹의 애니매이션과 현재 그룹의 애니매이션이 같다면 현재 그룹의 애니매이션 정보를 그대로 넘긴다.
			//		// - 변경 요청이 온 다른 그룹의 애니매이션과 현재 그룹의 애니매이션이 다르다면 현재 그룹에서 다른 그룹의 애니매이션으로 보간한다.
			//		for (_int iGroupIdx = iLower; iGroupIdx <= iTotal; ++iGroupIdx)
			//		{
			//			if (iGroupIdx == iCurGroupIdx)
			//				continue;

			//			if (ChangeGroup[iGroupIdx].second == -1)
			//				continue;

			//			if (m_BlendGroups[iCurGroupIdx].AnimInfo.iAnimIdx == ChangeGroup[iGroupIdx].second) // 현재 그룹과 변경 요청이 온 다른 그룹의 애니매이션이 같다면, 정보를 그대로 넘긴다.
			//			{
			//				// 해당 그룹이 사용중이 아니라면
			//				if (!m_BlendGroups[iGroupIdx].AnimInfo.bUse)
			//				{
			//					m_BlendGroups[iGroupIdx].AnimInfo = m_BlendGroups[iCurGroupIdx].AnimInfo;
			//					m_BlendGroups[iGroupIdx].AnimInfo.bUse = true;
			//					m_BlendGroups[iGroupIdx].AnimInfo.bEnd = false;
			//					m_BlendGroups[iGroupIdx].AnimInfo.bInterpolationing = false;

			//					if (m_BlendGroups[iGroupIdx].InterpolationInfo.bChainInterpolation)
			//						m_BlendGroups[iGroupIdx].InterpolationInfo.bChainInterpolation = false;
			//					m_BlendGroups[iGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
			//					fill(m_BlendGroups[iGroupIdx].InterpolationInfo.InterpolationDstGroupIdices, m_BlendGroups[iGroupIdx].InterpolationInfo.InterpolationDstGroupIdices + iTotal, -1);

			//					continue;
			//				}

			//				if (m_BlendGroups[iCurGroupIdx].AnimInfo.bUse)
			//				{
			//					m_BlendGroups[iCurGroupIdx].AnimInfo.bUse = false;
			//					m_BlendGroups[iCurGroupIdx].AnimInfo.bInterpolationing = true;

			//					if (m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation)
			//						m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation = false;

			//					m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
			//					fill(m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices, m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices + iTotal, -1);
			//				}
			//			}
			//			else // 현재 그룹과 변경 요청이 온 다른 그룹의 애니매이션이 다르다면, 보간한다.
			//			{
			//				// 현재 그룹 사용 false, 현 그룹 보간 true, 현 그룹 보보간 이상 false, 현 그룹 보간 거리 0.f, 현 그룹 보간 목적지에 ChangeGroup[iGroupIdx].second 대입
			//				m_BlendGroups[iCurGroupIdx].AnimInfo.bUse = false;
			//				m_BlendGroups[iCurGroupIdx].AnimInfo.bInterpolationing = true;
			//				m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation = false; // 250130
			//				m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f; // 250130
			//				m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iGroupIdx] = ChangeGroup[iGroupIdx].second;
			//				
			//				memset(m_BlendGroups[iGroupIdx].InterpolationInfo.InterpolationDstGroupIdices, -1, sizeof m_BlendGroups[iGroupIdx].InterpolationInfo.InterpolationDstGroupIdices);

			//				if (iGroupIdx != iTotal)
			//					ChangeGroup[iGroupIdx] = make_pair(Abgt::NONE, -1);

			//				Set_InterpolationData(m_BlendGroups[iCurGroupIdx].AnimInfo.iAnimIdx, ChangeGroup[iGroupIdx].second, m_BlendGroups[iCurGroupIdx].InterpolationInfo);
			//			}
			//		}
			//	}
			//	// 보보간 요청이거나 보보보간 이상의 경우
			//	else if (!m_BlendGroups[iCurGroupIdx].AnimInfo.bUse && m_BlendGroups[iCurGroupIdx].AnimInfo.bInterpolationing)
			//	{
			//		// 보보간 요청인경우
			//		if (!m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation)
			//		{
			//			for (_int iInterGroupIdx = iLower; iInterGroupIdx <= iTotal; ++iInterGroupIdx)
			//			{
			//				// 현재 그룹에서 보간 중이던 애니매이션 인덱스
			//				_int iCurGroupDstInterpolationAnimIdx = m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iInterGroupIdx];
			//				if (iCurGroupDstInterpolationAnimIdx == -1)
			//					continue;

			//				_int iSameGroupInterpolationAnimIdx = ChangeGroup[iInterGroupIdx].second; // 보간중일때 보보간 요청이 온 같은 그룹의 애니매이션 인덱스

			//				if (iSameGroupInterpolationAnimIdx != -1) // -1이 아니라면 같은 그룹에 보보간 요청이 온 애니매이션이 있다는 뜻이다. 같은 그룹끼리의 보보간이다.
			//				{
			//					if (iCurGroupDstInterpolationAnimIdx == iSameGroupInterpolationAnimIdx)
			//					{
			//						// assert(false && "보보간인데 애니매이션 인덱스가 동일하다!");
			//						// return;
			//						continue;
			//					}
			//					else // 애니매이션 인덱스가 다르기 때문에 보보간 해야한다
			//					{
			//						// 현재 그룹의 목적지 보간 애니매이션 인덱스를 변경하고 보간 거리를 초기화 한다.
			//						m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iInterGroupIdx] = iSameGroupInterpolationAnimIdx;
			//						m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f; // 250130

			//						if (!m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation) // 보보간 중이 아니라면 보보간을 한다고 설정한다.
			//							m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation = true;

			//						if (iCurGroupIdx == iTotal)
			//							ChangeGroup[iInterGroupIdx] = make_pair(Abgt::NONE, -1);
			//					}

			//					// 보보간 요청 목록의 애니매이션 인덱스 합이 -6이라면 더이상 요청이 없는 것이기 때문에 끝낸다.
			//					_int iSum = 0;
			//					for (_int i = iLower; i <= iTotal; ++i)
			//						iSum += ChangeGroup[i].second;
			//					if (iSum == -6)
			//						return;
			//				}
			//				// 같은 그룹끼리의 보보간이 아니다
			//				else
			//				{
			//					for (_int iBlendGroupIdx = iLower; iBlendGroupIdx <= iTotal; ++iBlendGroupIdx)
			//					{
			//						// 원래 보간 중이던 보간 목적지 인덱스를 없앤다.
			//						if (m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] != -1)
			//						{
			//							m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = -1;
			//							m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
			//						}
			//					}

			//					for (_int iBlendGroupIdx = iLower; iBlendGroupIdx <= iTotal; ++iBlendGroupIdx)
			//					{
			//						// 보보간 요청이 온 애니매이션 인덱스가 -1이 아닌 경우
			//						if (ChangeGroup[iBlendGroupIdx].second != -1)
			//						{
			//							// 현재 그룹이 TOTAL인 경우
			//							if (iCurGroupIdx == iTotal)
			//							{
			//								m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = ChangeGroup[iBlendGroupIdx].second;
			//							}
			//							// 현재 그룹이 TOTAL이 아닌 경우
			//							else
			//							{
			//								if (iBlendGroupIdx == iTotal)
			//								{
			//									if (iCurGroupIdx == iBlendGroupIdx)
			//										m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = ChangeGroup[iBlendGroupIdx].second;
			//								}
			//								else
			//								{
			//									if (iCurGroupIdx == iBlendGroupIdx)
			//									{
			//										m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = ChangeGroup[iBlendGroupIdx].second;
			//										ChangeGroup[iBlendGroupIdx] = make_pair(Abgt::NONE, -1);
			//									}
			//								}
			//							}

			//							if (!m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation)
			//								m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation = true;

			//							_int iSum = 0;
			//							for (_int i = iLower; i <= iTotal; ++i)
			//								iSum += ChangeGroup[i].second;
			//							if (iSum == -6)
			//								return;
			//						}
			//					}
			//				}
			//			}
			//		}
			//		// 보보보간 이상의 요청인 경우
			//		else if (m_BlendGroups[iCurGroupIdx].InterpolationInfo.bChainInterpolation)
			//		{
			//			for (_int iInterpolationGroupIdx = iLower; iInterpolationGroupIdx <= iTotal; ++iInterpolationGroupIdx)
			//			{
			//				// '현재 그룹에서 보보간 이상이라면'
			//				if (m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iInterpolationGroupIdx] != -1)
			//				{
			//					// 현재 그룹의 보간중인 그룹 애니매이션 인덱스
			//					_int iOriginInterpolationAnimIdx = m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iInterpolationGroupIdx];

			//					// 보간중일때 보보보간 이상의 요청이 온 같은 그룹의 애니매이션 인덱스
			//					_int iSameGroupInterpolationAnimIdx = (_int)ChangeGroup[iInterpolationGroupIdx].second;

			//					if (iSameGroupInterpolationAnimIdx != -1) // -1이 아니라면 같은 그룹에 보보보간 이상의 요청이 온 애니매이션이 있다는 뜻이다. 같은 그룹끼리의 보보간이다.
			//					{
			//						// 보보보간 이상인데 애니매이션 인덱스가 동일한 경우 리턴
			//						if (iOriginInterpolationAnimIdx == iSameGroupInterpolationAnimIdx)
			//							continue;
			//						// 애니매이션 인덱스가 다르기 때문에 보보보간 이상 해야한다
			//						else
			//						{
			//							// 현재 그룹의 목적지 보간 애니매이션 인덱스를 변경하고 보간 거리를 초기화 한다.
			//							m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iInterpolationGroupIdx] = iSameGroupInterpolationAnimIdx;
			//							m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;
			//							m_BlendGroups[iCurGroupIdx].AnimInfo.pAnim->Reset_ChainInterpolation_Bones_SRT();

			//							if (iCurGroupIdx == iTotal)
			//								ChangeGroup[iInterpolationGroupIdx] = make_pair(Abgt::NONE, -1);

			//							continue;
			//						}
			//					}

			//					// 같은 그룹끼리의 보보보간 이상이 아니다

			//					// 보간 목적지 인덱스 싹다 -1로 만든다.
			//					for (_int iBlendGroupIdx = iLower; iBlendGroupIdx <= iTotal; ++iBlendGroupIdx)
			//					{
			//						// 원래 보간 중이던 보간 목적지 인덱스를 없앤다.
			//						if (m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] != -1)
			//							m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = -1;
			//					}

			//					m_BlendGroups[iCurGroupIdx].InterpolationInfo.fInterpolationCurTrackPos = 0.f;

			//					for (_int iBlendGroupIdx = iLower; iBlendGroupIdx <= iTotal; ++iBlendGroupIdx)
			//					{
			//						// 보보간 요청이 온 애니매이션 인덱스가 -1이 아닌 경우
			//						if (ChangeGroup[iBlendGroupIdx].second != -1)
			//						{
			//							// 현재 그룹이 TOTAL인 경우
			//							if (iCurGroupIdx == iTotal)
			//							{
			//								// TOTAL인데 다른 그룹으로의 보보간인 경우
			//								if (iCurGroupIdx != iBlendGroupIdx)
			//									m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = ChangeGroup[iBlendGroupIdx].second;
			//								else // TOTAL인데 같은 그룹으로의 보보간 인 경우
			//									m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = ChangeGroup[iBlendGroupIdx].second;

			//								// 보보간 중이던 SRT 구성
			//								m_BlendGroups[iCurGroupIdx].AnimInfo.pAnim->Reset_ChainInterpolation_Bones_SRT();
			//							}
			//							// 현재 그룹이 TOTAL이 아닌 경우
			//							else
			//							{
			//								if (iBlendGroupIdx == iTotal)
			//								{
			//									// 2 6
			//									if (iCurGroupIdx == iBlendGroupIdx)
			//										m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = ChangeGroup[iBlendGroupIdx].second;
			//									else
			//									{
			//										m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = ChangeGroup[iBlendGroupIdx].second;
			//									}
			//								}
			//								else
			//								{
			//									if (iCurGroupIdx == iBlendGroupIdx)
			//									{
			//										m_BlendGroups[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iBlendGroupIdx] = ChangeGroup[iBlendGroupIdx].second;
			//										ChangeGroup[iBlendGroupIdx] = make_pair(Abgt::NONE, -1);
			//									}
			//								}

			//								// 보보간 중이던 SRT 구성
			//								m_BlendGroups[iCurGroupIdx].AnimInfo.pAnim->Reset_ChainInterpolation_Bones_SRT();
			//							}
			//						}
			//					}
			//				}
			//			}
			//		}
			//		else
			//		{
			//			assert(false, "보간, 보보간, 보보보간 이상 어느 경우에도 해당하지 않는다");
			//			return;
			//		}
			//	}
			//}
		
#pragma endregion
		
		return true;
	}

	vector<pair<Abgt, _int>> Get_AnimEndGroup();

private:
	void Finalize_Interpolation(Abgt eSrcInterpolationGroup, _int iSrcInterpolationAnimIdx, Abgt eDstStartAnimGroup, _int iDstAnimIdx);

	// 시작점이 TOTAL이 아닌경우 보간이 끝났을 때 호출되는 함수
	void Finalize_Interpolation_When_NotTotalTo(Abgt eSrcInterpolationGroup, Abgt eDstStartAnimGroup);			// 250208

	// 시작점이 TOTAL이고 끝 보간 그룹도 TOTAL인 경우
	void Finalize_Interpolation_When_TotalToTotal(Abgt eSrcInterpolationGroup, Abgt eDstStartAnimGroup);		// 250208

	void Finalize_Interpolation_When_TotalToNotTotal(Abgt eSrcInterpolationGroup, Abgt eDstStartAnimGroup);	// 250208

	void Reset_InterpolationDstIndices(Abgt eGroup); // 250209

	void Ready_Interpolation(Abgt eSrcInterpolationGroup, Abgt eDstInterpolationGroup, _int iInterpolationDstAnimIdx); // 250210
	void Ready_Interpolation_TotalToTotal(_int iInterDstAnimIdx);	// 250210
	void Ready_ToPlay_Anim(Abgt eGroup, const ANIM_INFO& SrcAnimInfo); // 250211
	void Ready_ChainInterpolation(Abgt eSrcChainInterpolationGroup, Abgt eDstChainInterpolationGroup, _int iChainInterpolationDstAnimIdx, _bool Setup_BoneSRT = false); // 250211
	
	void Cal_Interpolation_KeyFrame(_int iInterpolationDstAnimIdx, INTERPOLATION_INFO& InterpolationSrcInfo); // 250210

	// 보간이 끝나고 다음 애니매이션 TrackPos(시작 위치)를 맞추는 함수
	void Adjust_TrackPos_When_EndInterpolation(CAnimation* pDstAnim, _float fDstAnimDuration, _float fSrcInterpolationDis); // 250211
	
	_bool Is_Use_Condition(BLEND_GROUP& BG) // 250211
	{
		return BG.AnimInfo.bUse && !BG.AnimInfo.bInterpolationing && !BG.InterpolationInfo.bChainInterpolation;
	}

	_bool Is_Interpolation_Condition(BLEND_GROUP& BG) // 250211
	{
		return !BG.AnimInfo.bUse && BG.AnimInfo.bInterpolationing && !BG.InterpolationInfo.bChainInterpolation;
	}

	_bool Is_ChainInterpolation_Condition(BLEND_GROUP& BG) // 250211
	{
		return !BG.AnimInfo.bUse && BG.AnimInfo.bInterpolationing && BG.InterpolationInfo.bChainInterpolation;
	}

	_bool Is_TotalToTotal_Interpolationing(); // 250212

	// 상하체의 보간 목적지 중에 애니매이션이 같은지 확인하는 함수
	_bool Is_NotTotalToTotal_Interpolation_Condition(); // 250212


	_bool Out_Of_BlendGroup(_int iVal) // 250212
	{
		if (iVal < low || iVal > tot)
			return true;
		return false;
	}

	// 250211
	_bool Process_UsingGroups(_int iUsingCnt, _int iRequestCnt, vector<pair<Abgt, _int>>& UsingGroups, vector<pair<Abgt, _int>>& ChangeGroups);
	_bool Process_InterpolationingGroups(_int iInterpolationingGroupCnt, _int iRequestCnt, vector<pair<Abgt, _int>>& InterpolationingGroups, vector<pair<Abgt, _int>>& ChangeGroups);

	_bool Process_ChainInterpolationGroups(_int iChainInterpolationingGroupCnt, _int iRequestCnt, vector<pair<Abgt, _int>>& ChainInterpolationingGroups, vector<pair<Abgt, _int>>& ChangeGroups);

	void Process_When_TotGroup_ChainInterpolationing(_int iChainInterpolationingGroupCnt, _int iRequestCnt, vector<pair<Abgt, _int>>& ChainInterpolationingGroups, vector<pair<Abgt, _int>>& ChangeGroups); // 250214

	_int Get_Dst_InterAnimIdx(Abgt eSrc, Abgti iDst) // 250213
	{
		if (iDst < low || iDst > tot)
			return -1;
		return m_BlendGroups[Abgti(eSrc)].InterpolationInfo.InterpolationDstGroupIdices[iDst];
	}

	void Reset_DstInterpolationIdx(Abgt SrcGroup, Abgt DstGroup) // 250213
	{
		m_BlendGroups[Abgti(SrcGroup)].InterpolationInfo.InterpolationDstGroupIdices[Abgti(DstGroup)] = NON;
	}

	Abgti Get_Interpolation_AnimIdx(Abgt SrcGroup, Abgt DstGroup)
	{
		return m_BlendGroups[Abgti(SrcGroup)].InterpolationInfo.InterpolationDstGroupIdices[Abgti(DstGroup)];
	}

	_int Get_Dst_Interpolationing_Cnt(Abgt SrcGroup)
	{
		_int iCnt = 0;
		for (_int i = low; i <= tot; ++i)
			if (Get_Dst_InterAnimIdx(SrcGroup, i) != NON)
				++iCnt;
		return iCnt;
	}

public:
	// 250201
	void Set_BlendGroup_Loop(Abgt eBlendGroup, _bool bLoop)
	{
		m_BlendGroups[_int(eBlendGroup)].AnimInfo.bIsLoop = bLoop;
	}

	// 250201
	_bool Get_BlendGroup_Loop(Abgt eBlendGroup)
	{
		return m_BlendGroups[_int(eBlendGroup)].AnimInfo.bIsLoop;
	}

	// 250203
	void Reset_BlendGroupAnim(Abgt eBlendGroup) 
	{
		if (m_BlendGroups[_int(eBlendGroup)].AnimInfo.pAnim)
		{
			m_BlendGroups[_int(eBlendGroup)].AnimInfo.pAnim->Reset_CurKeyFrameIndices();
			m_BlendGroups[_int(eBlendGroup)].AnimInfo.pAnim->Reset_TrackPos();
			m_BlendGroups[_int(eBlendGroup)].AnimInfo.pAnim->Reset_Inter_Anim();
		}
	}

	// 250201
	void Set_Loop(_bool bLoop) 
	{ 
		m_isLoop = bLoop; 
	}

	// 250201
	_bool Get_Loop()
	{
		return m_isLoop;
	}

	// -------------------------------------------------------------


	_bool Get_CurPlayAnimEnd()  { return m_bCurPlayAnimEnd; }
	_bool Get_Interpolationing() { return m_bInterpolationing; }
	
	void Set_InterDis(_float fInterDis) 
	{ 
		if (m_bInterpolationing)
			m_fInterDis = fInterDis;
	}
	_float Get_InterDis() 
	{ 
		if (m_bInterpolationing)
			return m_fInterDis;
		return 0.f;
	}

	_float Get_CurInterTrackPos();
	_float Get_InterSP() 
	{ 
		if (m_bInterpolationing)
			return m_fInterSP; 
		return 0.f;
	}
	void Set_InterSP(_float fInterSP)
	{
		if (m_bInterpolationing)
			m_fInterSP = fInterSP;
	}

	_bool Get_LoadedInterInfo() { return m_bLoadedInterInfo; }
	_int Get_BoneSize() { return m_Bones.size(); }

private:
	void Set_Interpolationing(_bool bInter)  { m_bInterpolationing = bInter; }
	// void Initialize_BlendGroup(CAnimation* pBlendGroupAnim, _int iAnimIdx, BLEND_GROUP& BlendGroup);

	void Reset_Anim(_int iAnimIdx);

public:
	const aiScene* Get_AIScene() { return m_pAIScene; }

public:
	// 보간 애니매이션 인덱스 반환
	_int Get_Inter_AnimIdx() { return m_iInterDstAnimIdx; }
	_int Get_AnimIdx(string strAnimName);
	string Get_AnimName(_int iAnimIdx);

	// 애니매이션 변경 함수
	// - iDstAnimIdx : 보간 목적지 anim idx, fInterDis : 보간 거리(시간), fInterSP : 보간 속도
	// - bEndToFirstInterpolation : true = 현재 애니매이션 끝과 다음 애니매이션 처음을 보간 하겠다. 디폴트 값은 false이다.
	void Change_Anim(_int iDstAnimIdx, _bool bEndToFirstInterpolation = false);
	void Change_Anim(string strDstAnim, _bool bEndToFirstInterpolation = false);

	// 보간 속도, 거리 설정
	void Set_InterpolationData(); // 250127
	void Set_InterpolationData(_int iSrcAnimIdx, _int iDstAnimIdx, INTERPOLATION_INFO& SrcInterpolationInfo); // 250211

	void Reset_CurAnim();

	string Get_RootMotionBoneName() { return m_strRootMotionBoneName; }
	_matrix& Get_RootBoneCombinedTransfomationMatrix() { return m_MatRootBoneCombinedTransfomationMatrix; }

	_float Get_Anim_TickPerSecond();
	_float Get_Anim_Duration();
	_float Get_Anim_CurTrackPos();

	void Set_AnimPlayType(ANIM_PLAY_TYPE eAnimPlayType) { m_eAnimPlayType = eAnimPlayType; } // 250122
	ANIM_PLAY_TYPE Get_AnimPlayType() { return m_eAnimPlayType; }	// 250122
	_bool Get_EndToFirst_Interpolation() { return m_bEndToFirst_Interpolation; } // 250122
	
	_matrix Get_Bone_CombiendTransformationMatrix(string strBoneName); // 250123

	void Set_Bone_CombinedTransformationMatrix(string strBoneName, _fmatrix CombinedMatrix); // 250205

	_bool Get_BlendGroupInterpolationing(Abgt eBlendGroup)
	{
		return m_BlendGroups[_int(eBlendGroup)].AnimInfo.bInterpolationing;
	}

	CAnimation* Get_Anim(_int iAnimIdx); // 250130

public:
	// 이벤트 바인딩
	
	// 멤버 함수는 객체의 인스턴스가 필요하므로 직접 바인딩할 수 없고 std::bind 또는 람다 함수를 사용해야한다.
	// ====================================================================================================
	// 예시 코드
	//	class MyClass
	//	{
	//		public:
	//		void PrintMessage() 
	//		{
	//			std::cout << "Hello from MyClass!" << std::endl;
	//		}
	//	};
	//	int main() 
	//  {
	//		MyClass obj;
	//		// 멤버 함수를 바인딩할 때 객체의 참조를 함께 전달해야 한다.
	//		std::function<void()> callback = std::bind(&MyClass::PrintMessage, &obj);
	//		// 실행
	//		callback(); // "Hello from MyClass!"
	//		return 0;
	//	}
	// ====================================================================================================

	HRESULT Bind_Event(string strAnimName, string strLayerName, string strEventName, function<void()> CallBack); // 매개변수가 없고 반환값도 없는 함수 바인딩


	// IK
public: // 250207 
	// 뼈의 로컬 행렬 반환
	_float4x4* Get_Bone_TransformationMatrix_Ptr(string strBoneName); // 250203

	// 애니매이션 업데이트에서 제외 시킬 특정 뼈 하나를 설정하는 함수
	void Set_UpdateExcludeBoneName(string strExcludeBoneName)  // 250204
	{ 
		m_strUpdateExcludeBoneName = strExcludeBoneName;

		if (m_strUpdateExcludeBoneName != "None" && m_strUpdateExcludeBoneName != "none" && m_strUpdateExcludeBoneName != "")
		{
			Set_Origin_UpdateExcludeBone_TransformationMatrix(m_strUpdateExcludeBoneName);
		}
	}

	// 애니매이션 업데이트에서 제외한 뼈 이름 반환 // 250204
	string Get_UpdateExcludeBoneName() 
	{ 
		return m_strUpdateExcludeBoneName; 
	}	

	// 애니매이션 업데이트에서 제외된 뼈 로컬 행렬(TransformationMatrix) 설정하는 함수
	// 250204
	void Set_UpdateExcludeBone_TransformationMatrix(_fmatrix UpdateExcludeBoneTransformationMatrix)  
	{ 
		XMStoreFloat4x4(&m_UpdateExcludeBone_TransformationMatrix, UpdateExcludeBoneTransformationMatrix);
	} 

	// 애니매이션 업데이트에서 제외된 뼈 로컬 행렬(TransformationMatrix) 반환 하는 함수
	_float4x4 Get_UpdateExcludeBone_TransformationMatrix() 
	{ 
		return m_UpdateExcludeBone_TransformationMatrix;
	}

	// 애니매이션 업데이트에서 제외된 뼈의 부모의 CombinedTransformationMatrix 설정하는 함수 250205
	//void Set_Parent_CombinedTransformationMatrix_From_UpdateExlcudeBone(_fmatrix ParentCombinedMatrix) 
	//{
	//	XMStoreFloat4x4(&m_Parent_CombinedTransformationMatrix_From_UpdateExcludeBone, ParentCombinedMatrix);
	//}

	// 애니매이션 업데이트에서 제외된 뼈의 부모의 CombinedTransformationMatrix 반환하는 함수 250205
	_fmatrix Get_Parent_CombinedTransformationMatrix_From_UpdateExlcudeBone();
	
	
	// 애니매이션 업데이트에서 제외된 뼈의 CombinedTransformationMatrix를 설정하는 함수
	// - IK-LookAtRotation에는 안씀
	void Set_UpdateExcludeBone_CombinedTransformationMatrix(_fmatrix ExBoneCombinedTransformationMatrix)
	{
		XMStoreFloat4x4(&m_UpdateExcludeBone_CombinedTransformationMatrix, ExBoneCombinedTransformationMatrix);
	}

	// 애니매이션 업데이트에서 제외된 뼈의 CombinedTransformationMatrix를 반환 하는 함수
	_float4x4 Get_UpdateExcludeBone_CombinedTransformationMatrix()
	{
		return m_UpdateExcludeBone_CombinedTransformationMatrix;
	}

	// 250208
	string Get_BoneName(_int iBoneIdx);

	// 250208
	CBone* Get_Bone(string strBoneName);

	// 250208
	CBone* Get_Bone(_int iBoneIdx);

	// 250208
	_int Get_Parent_BoneIdx(string strBoneName);

	// 250207
	void Rotate_UpdateExcludeBone_To_Target(string strUpdateExcludeBoneName, _fmatrix ContainerObjectWorldMatrix, _fmatrix TargetWorldMatrix);


private: // 250207
	void Set_Origin_UpdateExcludeBone_TransformationMatrix(string strUpdateExcludeBoneName)
	{
		m_Origin_UpdateExcludeBone_TransformationMatrix = *(Get_Bone_TransformationMatrix_Ptr(strUpdateExcludeBoneName));

		//XMStoreFloat4x4(&m_Origin_UpdateExcludeBone_TransformationMatrix, XMMatrixIdentity());
		//m_Origin_UpdateExcludeBone_TransformationMatrix._41 = (*Get_Bone_TransformationMatrix_Ptr(strUpdateExcludeBoneName))._41;
		//m_Origin_UpdateExcludeBone_TransformationMatrix._42 = (*Get_Bone_TransformationMatrix_Ptr(strUpdateExcludeBoneName))._42;
		//m_Origin_UpdateExcludeBone_TransformationMatrix._43 = (*Get_Bone_TransformationMatrix_Ptr(strUpdateExcludeBoneName))._43;
		//m_Origin_UpdateExcludeBone_TransformationMatrix._43 = 1.f;
	}

	_float4x4 Get_Origin_UpdateExcludeBone_TransformationMatrix()
	{
		return m_Origin_UpdateExcludeBone_TransformationMatrix;
	}


	// 250214
public:
	void Stop_CurAnim() { m_bPlayAnim = false; }
	void Play_CurAnim() { m_bPlayAnim = true; }
	void Set_Group_Update(Abgt Group, _bool bUpdate) // 사용중인 그룹에 대해서 업데이터 여부 결정하는 함수
	{
		if (m_BlendGroups[Abgti(Group)].AnimInfo.bUse)
		{
			m_BlendGroups[Abgti(Group)].AnimInfo.bUpdate = bUpdate;
		}
	}


private:
	vector<string>							m_AnimationNames;
	_bool									m_bPlayAnim = true; // 250214
	_bool									m_bCurPlayAnimEnd = false;
	_int									m_iInterDstAnimIdx = -1;
	_bool									m_bInterpolationing = false;
	_float									m_fDefaultInterDis = 0.2f;
	_float									m_fDefaultInterSP = 1.f;
	_float									m_fInterDis = 0.2f;
	_float									m_fInterSP = 1.f;
	vector<map<_int, pair<_float, _float>>> m_InterInfos; 	// 보간 정보를 담고 있는 컨테이너
	_bool									m_bLoadedInterInfo = false; // xml로 보간 정보들이 로드 되었는지
	string									m_strRootMotionBoneName{};
	_matrix									m_MatRootBoneCombinedTransfomationMatrix{};
	ANIM_PLAY_TYPE							m_eAnimPlayType = ANIM_PLAY_TYPE::ANIM_DEFAULT; // 250122
	_bool									m_bEndToFirst_Interpolation = false;
	BLEND_GROUP								m_BlendGroups[_int(Abgt::BLEND_END)]; // 뼈 블렌딩
	
	_bool									m_bChainInterpolation = false; // 250131 일반 애니매이션 보보간
	_float									m_fInterTrackPos = 0.f; // 250131 일반 애니매이션 보보간

	// map<애니매이션 이름, map<레이어 이름, vector<pair<이벤트 이름, 트랙 포지션>>>> // 250202
	map<string, map<string, vector<pair<string, _float>>>> m_Events;



	


	// IK
private:
	// 애니매이션 업데이트에서 제외할 뼈 이름, 행렬 250204
	string									m_strUpdateExcludeBoneName{}; // 250204
	_float4x4								m_UpdateExcludeBone_TransformationMatrix; // 250204 애니매이션 업데이트에서 제외된 뼈가 사용할 _float4x4 
	// _float4x4								m_Parent_CombinedTransformationMatrix_From_UpdateExcludeBone; // 250205
	_float4x4								m_UpdateExcludeBone_CombinedTransformationMatrix; // 250205
	_float4x4								m_Origin_UpdateExcludeBone_TransformationMatrix{};

// --------------------- Animatioin Tool 에서만 사용 ---------------------


public:
	_bool Get_CurAnim_PlayState() { return m_bPlayAnim; }
	void Set_UseCustumInterData(_bool bUseCustumData) { m_bUseCustumInterData = bUseCustumData; }
	_bool Get_UseCustumInterData() { return m_bUseCustumInterData; }
	void Set_CustumInterDis(_float fCustumInterdis) { m_fCustumInterDis = fCustumInterdis; }
	void Set_CustumInterSP(_float fCustumInterSP) { m_fCustumInterSP = fCustumInterSP; }

	BLEND_GROUP* Get_BlendGroups() { return m_BlendGroups; } // 250127

	_float Get_Anim_TickPerSecond(_int iAnimIdx); // 250130
	_float Get_Anim_Duration(_int iAnimIdx);	// 250130
	_float Get_Anim_CurTrackPos(_int iAnimIdx); // 250130
	void Set_CurAnim_TrackPos(_float fTrackPos);

	vector<class CBone*>* Get_Bones_Ptr() { return &m_Bones; }

	

private:
	_bool m_bUseCustumInterData = false;
	_float m_fCustumInterDis = 0.2f;
	_float m_fCustumInterSP = 1.f;

// ----------------------------------------------------------------------



// =============================================================================================

private:
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	HRESULT Ready_Bones(const aiNode* pNode, _int iParentBoneIndex);
	HRESULT Ready_Animations();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL_TYPE eModelType, const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END;
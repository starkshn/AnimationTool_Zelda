#pragma once

#include "Component.h"
#include "Animation.h" // 250125

#include <utility> // for std::pair
#include <type_traits> // for std::is_same_v
#include <array>
#include <tinyxml2.h>

#include <iostream>
using namespace std;

namespace xml = tinyxml2;
BEGIN(Engine)

class CBone;

class ENGINE_DLL CModel final : public CComponent
{
#ifdef _DEBUG
private:
	constexpr static array<const _char*, tot + 1> AnimTypeStr = { "NON", "LOWER_BODY", "UPPER_BODY", "ARM_LEFT", "ARM_RIGHT", "HEAD", "TOTAL" };
#endif


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

		// 가장 긴 요소의 너비 계산
		const _int maxWidth = 5;


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



	
#pragma region "(디버그 콘솔창 출력) Change_Anim 시작"

#ifdef _DEBUG

		cout << endl;
		cout << endl;
		cout << "====================================================" << m_iChangeAnimCnt << " 번째 " << "Change_Anim 처리 결과 ====================================================" << endl;
		cout << endl;
		cout << endl;

#endif

#pragma endregion


#pragma region "(디버그 콘솔창 출력) 변경 요청 애니매이션"

		cout << endl;
		cout << "-------------------- 변경 요청 애니매이션 정보 --------------------" << endl;
		cout << endl;
		for (_int i = low; i <= tot; ++i)
		{
			if (ChangeGroups[i].second == NON)
				continue;

			cout << setw(30) << "[ Anim Group | " << AnimTypeStr[i] << " ] " << " [ Anim Idx | " << ChangeGroups[i].second << " ] " << " [ Anim Name | " << m_Animations[ChangeGroups[i].second]->Get_AnimName() << " ] " << endl;
		}

		cout << endl;
		cout << "-------------------- 변경 요청 애니매이션 정보 --------------------" << endl;
		cout << endl;

#pragma endregion



		// 보보간 이상에 대한 철;
		if (iChainInterpolationingCnt)
		{

#pragma region "(디버그 콘솔창 출력) 보보간 이상 수행중인 그룹들 변경 요청전 데이터 임시 보관 로직"
#ifdef _DEBUG 

			vector<_int> vecChainInterpolationingGroups(tot + 1, NON); // 보보간 이상 수행중인 그룹들을 담는 벡터
			vector<vector<_int>> vecOriginChainInterpolationingGroups(tot + 1, vector<_int>(tot + 1, NON)); // 보보간 이상 수행중인 그룹들이 보보간 이상중이던 목적지 그룹을 담는 벡터

			for (_int i = low; i <= tot; ++i)
			{
				for (_int j = low; j <= tot; ++j)
					vecOriginChainInterpolationingGroups[i][j] = Get_Dst_InterAnimIdx(Abgt(i), j);

				if (ChainInterpolationingGroups[i].second != NON)
					vecChainInterpolationingGroups[i] = ChainInterpolationingGroups[i].second;
			}

#endif
#pragma endregion

#pragma region "(주석 설명) 보간중인 그룹 처리 로직 설명"

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

			// 보보간 이상 처리
			Process_ChainInterpolationGroups(iChainInterpolationingCnt, iRequestCnt, ChainInterpolationingGroups, ChangeGroups);

#pragma region "(디버그 콘솔창 출력) 보보간 이상인 경우에 대해 변경 요청에 따라 변경된 결과 출력"

#ifdef _DEBUG

			for (_int i = low; i <= tot; ++i)
			{
				// 보보간 이상중인 그룹이 NON이면 건너 뛴다.
				if (vecChainInterpolationingGroups[i] == NON)
					continue;

				
				// Process_ChainInterpolationGroups 를 통과하고난 그룹과 vecTempBG[i] 번 그룹을 비교해서 변경사항이 무엇이 있는지 확인한다.
				_int iOriginChainInterpolationingGroupIdx = i; // 보보간 이상을 수행중인던 그룹 인덱스
				_int iOriginChainInterpolationingCnt = Get_Dst_Interpolationing_Cnt(Abgt(iOriginChainInterpolationingGroupIdx)); // 보보간 이상을 수행중이던 그룹이 향하던 보보간 이상의 그룹 개수

				
				cout << endl;
				cout << "-------------------- 보보간 이상 처리 결과 --------------------" << endl;
				cout << endl;

				
				_int iPrevChainInterpolationingAnimIdx = ChainInterpolationingGroups[i].second;
				string strPrevChainInterpolationingAnimName = m_Animations[i]->Get_AnimName();
				cout << AnimTypeStr[i] << '(' << iOriginChainInterpolationingGroupIdx << ')' << " | 번 그룹은 " << strPrevChainInterpolationingAnimName << '(' << iPrevChainInterpolationingAnimIdx << ')' << " 애니매이션 에서부터" << "보보간 이상을 수행중이였다." << endl;

				cout << AnimTypeStr[i] << '(' << iOriginChainInterpolationingGroupIdx << ')' << " | 번 그룹이 보보간 이상을 수행중인 목적지 그룹 개수 | " << iOriginChainInterpolationingCnt << endl;
				cout << AnimTypeStr[i] << '(' << iOriginChainInterpolationingGroupIdx << ')' << " | 번 그룹의 보보간 이상의 수행중과 변경후 결과" << endl;

				// 수행중이던 보간 목적지 출력
				for (_int j = low; j <= tot; ++j)
				{
					cout << setw(maxWidth * 2) << AnimTypeStr[j];
						
						
					cout << '[';

					if (vecOriginChainInterpolationingGroups[i][j] != NON)
						cout << setw(maxWidth) << vecOriginChainInterpolationingGroups[i][j];
					else
						cout << setw(maxWidth) << 'X';

					cout << ']' << " | ";
				}
				cout << endl;

				// 수행후인 보간 목적지(결과) 출력



				for (_int j = low; j <= tot; ++j)
				{
					cout << setw(maxWidth * 2) << AnimTypeStr[j];

					cout << '[';

					if (Get_Dst_InterAnimIdx(Abgt(i), j) != NON)
						cout << setw(maxWidth) << Get_Dst_InterAnimIdx(Abgt(i), j);
					else
						cout << setw(maxWidth) << 'X';

					cout << ']' << " | ";
				}
				cout << endl;


				cout << endl;
				cout << "-------------------- 보보간 이상 처리 결과 --------------------" << endl;
				cout << endl;

				
			}

#endif
#pragma endregion

		}


		// 보간 이상에 대한 처리
		if (iInterpolationingCnt)
		{

#pragma region "(디버그 콘솔창 출력) 보간 이상 수행중인 그룹들 변경 요청전 데이터 임시 보관 로직"
#ifdef _DEBUG 

			vector<_int> vecInterpolationingGroups(tot + 1, NON);
			vector<vector<_int>> vecOriginInterpolationingGroups(tot + 1, vector<_int>(tot + 1, NON));

			for (_int i = low; i <= tot; ++i)
			{
				for (_int j = low; j <= tot; ++j)
					vecOriginInterpolationingGroups[i][j] = Get_Dst_InterAnimIdx(Abgt(i), j);

				if (InterpolationingGroups[i].second != NON)
					vecInterpolationingGroups[i] = InterpolationingGroups[i].second;
			}

#endif
#pragma endregion

			// Process_UsingGroups에서 처리 되어서 보간 중인 그룹들에 대해서 조건에 맞는 경우 보보간 처리를 한다.
			Process_InterpolationingGroups(iInterpolationingCnt, iRequestCnt, InterpolationingGroups, ChangeGroups);

#pragma region "(디버그 콘솔창 출력) 보간 이상인 경우에 대해 변경 요청에 따라 변경된 결과 출력"

#ifdef _DEBUG

			for (_int i = low; i <= tot; ++i)
			{
				// 보보간 이상중인 그룹이 NON이면 건너 뛴다.
				if (vecInterpolationingGroups[i] == NON)
					continue;


				// Process_ChainInterpolationGroups 를 통과하고난 그룹과 vecTempBG[i] 번 그룹을 비교해서 변경사항이 무엇이 있는지 확인한다.
				_int iOriginInterpolationingGroupIdx = i; // 보보간 이상을 수행중인던 그룹 인덱스
				_int iOriginInterpolationingCnt = Get_Dst_Interpolationing_Cnt(Abgt(iOriginInterpolationingGroupIdx)); // 보보간 이상을 수행중이던 그룹이 향하던 보보간 이상의 그룹 개수


				cout << endl;
				cout << "-------------------- 보간 이상 처리 결과 --------------------" << endl;
				cout << endl;


				_int iPrevInterpolationingAnimIdx = InterpolationingGroups[i].second;
				string strPrevInterpolationingAnimName = m_Animations[i]->Get_AnimName();
				cout << AnimTypeStr[i] << '(' << iOriginInterpolationingGroupIdx << ')' << " | 번 그룹은 " << strPrevInterpolationingAnimName << '(' << iPrevInterpolationingAnimIdx << ')' << " 애니매이션 에서부터" << "보간 이상을 수행중이였다." << endl;

				cout << AnimTypeStr[i] << '(' << iOriginInterpolationingGroupIdx << ')' << " | 번 그룹이 보간 이상을 수행중인 목적지 그룹 개수 | " << iOriginInterpolationingCnt << endl;
				cout << AnimTypeStr[i] << '(' << iOriginInterpolationingGroupIdx << ')' << " | 번 그룹의 보간 이상의 수행중과 변경후 결과" << endl;

				// 수행중이던 보간 목적지 출력
				for (_int j = low; j <= tot; ++j)
				{
					cout << setw(maxWidth * 2) << AnimTypeStr[j];


					cout << '[';

					if (vecOriginInterpolationingGroups[i][j] != NON)
						cout << setw(maxWidth) << vecOriginInterpolationingGroups[i][j];
					else
						cout << setw(maxWidth) << 'X';

					cout << ']' << " | ";

				}
				cout << endl;

				// 수행후인 보간 목적지(결과) 출력



				for (_int j = low; j <= tot; ++j)
				{
					cout << setw(maxWidth * 2) << AnimTypeStr[j];

					cout << '[';

					if (Get_Dst_InterAnimIdx(Abgt(i), j) != NON)
						cout << setw(maxWidth) << Get_Dst_InterAnimIdx(Abgt(i), j);
					else
						cout << setw(maxWidth) << 'X';

					cout << ']' << " | ";

				}
				cout << endl;


				cout << endl;
				cout << "-------------------- 보간 이상 처리 결과 --------------------" << endl;
				cout << endl;


			}

#endif
#pragma endregion

		}


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


		if (iUsingCnt)
		{

#pragma region "(디버그 콘솔창 출력) 애니매이션 수행중인 그룹들 변경 요청전 데이터 임시 보관 로직"
#ifdef _DEBUG 

			vector<_int> vecUsingGroups(tot + 1, NON);
			vector<vector<_int>> vecOriginUsingGroups(tot + 1, vector<_int>(tot + 1, NON));

			// 사용중인 그룹들만 담는다.
			for (_int i = low; i <= tot; ++i)
			{
				for (_int j = low; j <= tot; ++j)
					vecOriginUsingGroups[i][j] = Get_Dst_InterAnimIdx(Abgt(i), j);

				if (UsingGroups[i].second != NON)
					vecUsingGroups[i] = UsingGroups[i].second;
			}

#endif
#pragma endregion


			// 사용중인 그룹들 처리를 한다.
			// 보간도 아니고 보보간이상인 경우도 아닌 그룹들은 조건에 맞는 경우 보간 처리를 한다.
			Process_UsingGroups(iUsingCnt, iRequestCnt, UsingGroups, ChangeGroups);


#pragma region "(디버그 콘솔창 출력) 애니매이션 수행중인 그룹들 변경 요청에 따라 변경된 결과 출력"

#ifdef _DEBUG

			for (_int i = low; i <= tot; ++i)
			{
				if (vecUsingGroups[i] == NON)
					continue;

				cout << endl;
				cout << "-------------------- 사용중인 그룹 처리 결과 --------------------" << endl;
				cout << endl;


				_int iPrevUsingAnimIdx = UsingGroups[i].second;
				string strPrevUsingAnimName = m_Animations[i]->Get_AnimName();
				cout << AnimTypeStr[i] << '(' << i << ')' << " | 번 그룹은 " << strPrevUsingAnimName << '(' << iPrevUsingAnimIdx << ')' << " 애니매이션 수행중이였다." << endl;

				cout << AnimTypeStr[i] << '(' << i << ')' << " | 번 그룹의 변경후 결과" << endl;

				// 수행중이던 그룹의 변경전 출력
				for (_int j = low; j <= tot; ++j)
				{
					cout << setw(maxWidth * 2) << AnimTypeStr[j];

					cout << '[';

					if (vecOriginUsingGroups[i][j] != NON)
						cout << setw(maxWidth) << vecOriginUsingGroups[i][j];
					else
						cout << setw(maxWidth) << 'X';

					cout << ']' << " | ";

				}
				cout << endl;

				// 수행후인 그룹의 변경후 보간결과 출력
				for (_int j = low; j <= tot; ++j)
				{
					cout << setw(maxWidth * 2) << AnimTypeStr[j];

					cout << '[';

					if (Get_Dst_InterAnimIdx(Abgt(i), j) != NON)
						cout << setw(maxWidth) << Get_Dst_InterAnimIdx(Abgt(i), j);
					else
						cout << setw(maxWidth) << 'X';

					cout << ']' << " | ";
				}
				cout << endl;


				cout << endl;
				cout << "-------------------- 사용중인 그룹 처리 결과 --------------------" << endl;
				cout << endl;


			}

#endif
#pragma endregion

		}

#pragma region "(디버그 콘솔창 출력) Change_Anim 끝"
#ifdef _DEBUG

	cout << endl;
	cout << endl;
	cout << "====================================================" << m_iChangeAnimCnt << " 번째 " << "Change_Anim 처리 결과 ====================================================" << endl;
	cout << endl;
	cout << endl;

#endif
#pragma endregion

	m_iChangeAnimCnt++;

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

#pragma region "Set_BlendGroup_Loop"

#ifdef _DEBUG

		cout << endl;
		cout << AnimTypeStr[_int(eBlendGroup)] << "그룹에 대해서 루프를 [" << bLoop << "] 로 설정했습니다." << endl;
		cout << endl;

#endif

#pragma endregion

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

	_int m_iChangeAnimCnt = 1;
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
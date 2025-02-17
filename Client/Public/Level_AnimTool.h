#pragma once
#include "Client_Defines.h"
#include "Level.h"

// ------------------------------------------------
// ImGui
// ------------------------------------------------
#include <commdlg.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// ------------------------------------------------
// Client
// ------------------------------------------------
#include "AT_Obj.h"
#include "AT_PartAnimObj.h"
#include "AT_Square.h"
#include "AT_Mouse.h"
#include "AT_Stair.h"
#include "AT_Ray.h"

// ------------------------------------------------
// Engine
// ------------------------------------------------
#include "Model.h"
#include "Engine_Defines.h"


// ------------------------------------------------
// C++
// ------------------------------------------------
#include <iostream>
#include <filesystem>
#include <tinyxml2.h>
#include <sstream>
#include <tuple>
namespace xml = tinyxml2;


BEGIN(Client)

// Example: Implement dual list box storage and interface
struct ExampleDualListBox
{
	ImVector<ImGuiID>           Items[2];               // ID is index into m_AnimNamesCStr
	ImGuiSelectionBasicStorage  Selections[2];          // Store ExampleItemId into selection
	std::vector<const char*>    m_AnimNamesCStr;        // Replaces ExampleNames
	bool                        OptKeepSorted = true;

	// Blend 리스트에 아이템이 하나만 들어갈 수 있도록 제한
	void AddToBlend(ImGuiID item_id)
	{
		if (Items[1].Size > 0) // 이미 Blend 리스트에 아이템이 있다면 제거
		{
			Items[1].clear();
			Selections[1].Clear();
		}
		Items[1].push_back(item_id); // 새 아이템 추가
	}

	// Blend 리스트 아이템 이름 가져오기
	const char* GetBlendItemName() const
	{
		if (Items[1].Size == 0)
			return "None"; // Blend 리스트가 비어 있으면 "None" 반환
		return m_AnimNamesCStr[Items[1][0]]; // 첫 번째 아이템 이름 반환
	}

	void MoveAll(int src, int dst)
	{
		IM_ASSERT((src == 0 && dst == 1) || (src == 1 && dst == 0));

		if (dst == 1 && Items[1].Size > 0) // Blend 리스트가 이미 비어있지 않다면 모든 이동을 막음
			return;

		for (ImGuiID item_id : Items[src])
		{
			if (dst == 1 && Items[1].Size >= 1) // Blend 리스트에 1개만 허용
				break;
			Items[dst].push_back(item_id);
		}

		Items[src].clear();
		SortItems(dst);
		Selections[src].Swap(Selections[dst]);
		Selections[src].Clear();
	}

	void MoveSelected(int src, int dst)
	{
		if (dst == 1 && Items[1].Size > 0) // Blend 리스트에 이미 아이템이 있으면 동작 막음
			return;

		for (int src_n = 0; src_n < Items[src].Size; src_n++)
		{
			ImGuiID item_id = Items[src][src_n];
			if (!Selections[src].Contains(item_id))
				continue;

			if (dst == 1 && Items[1].Size >= 1) // Blend 리스트에 1개만 허용
				break;

			Items[src].erase(&Items[src][src_n]); // FIXME-OPT: Could be implemented more optimally (rebuild src items and swap)
			Items[dst].push_back(item_id);
			src_n--;
		}
		if (OptKeepSorted)
			SortItems(dst);
		Selections[src].Swap(Selections[dst]);
		Selections[src].Clear();
	}

	void ApplySelectionRequests(ImGuiMultiSelectIO* ms_io, int side)
	{
		// In this example we store item id in selection (instead of item index)
		Selections[side].UserData = Items[side].Data;
		Selections[side].AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self, int idx) { ImGuiID* items = (ImGuiID*)self->UserData; return items[idx]; };
		Selections[side].ApplyRequests(ms_io);
	}

	static int IMGUI_CDECL CompareItemsByValue(const void* lhs, const void* rhs)
	{
		const int* a = (const int*)lhs;
		const int* b = (const int*)rhs;
		return (*a - *b) > 0 ? +1 : -1;
	}

	void SortItems(int n)
	{
		qsort(Items[n].Data, (size_t)Items[n].Size, sizeof(Items[n][0]), CompareItemsByValue);
	}

	void Show()
	{
		if (ImGui::BeginTable("split", 3, ImGuiTableFlags_None))
		{
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Source
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);      // Buttons
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Blend
			ImGui::TableNextRow();

			int request_move_selected = -1;
			int request_move_all = -1;

			float list_box_height = ImGui::GetFontSize() * 20; // 리스트 박스의 높이
			float button_area_height = list_box_height;       // 버튼 영역의 높이를 리스트 박스와 동일하게 설정
			float button_height = ImGui::GetFrameHeight();    // 버튼의 높이
			float button_offset = (button_area_height - (button_height * 2)) / 2; // 버튼을 중앙에 정렬하기 위한 오프셋

			for (int side = 0; side < 2; side++)
			{
				ImVector<ImGuiID>& items = Items[side];
				ImGuiSelectionBasicStorage& selection = Selections[side];

				ImGui::TableSetColumnIndex((side == 0) ? 0 : 2);
				ImGui::Text("%s (%d)", (side == 0) ? "Source" : "Blend", items.Size);

				bool child_visible = ImGui::BeginChild(side == 0 ? "0" : "1", ImVec2(-FLT_MIN, list_box_height), ImGuiChildFlags_FrameStyle | (side == 0 ? ImGuiChildFlags_ResizeY : 0));
				if (child_visible)
				{
					ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_None;
					ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, items.Size);
					ApplySelectionRequests(ms_io, side);

					for (int item_n = 0; item_n < items.Size; item_n++)
					{
						ImGuiID item_id = items[item_n];
						bool item_is_selected = selection.Contains(item_id);
						ImGui::SetNextItemSelectionUserData(item_n);

						string strRet = "idx : " + to_string(item_n) + " | " + m_AnimNamesCStr[item_id];
						ImGui::Selectable(strRet.c_str(), item_is_selected, ImGuiSelectableFlags_AllowDoubleClick);
						if (ImGui::IsItemFocused())
						{
							if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
								request_move_selected = side;
							if (ImGui::IsMouseDoubleClicked(0))
								request_move_selected = side;
						}
					}

					ms_io = ImGui::EndMultiSelect();
					ApplySelectionRequests(ms_io, side);
				}
				ImGui::EndChild();
			}

			// 버튼 영역 중앙 정렬
			ImGui::TableSetColumnIndex(1);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_offset); // 첫 번째 버튼 중앙 배치
			ImVec2 button_sz = { ImGui::GetFrameHeight(), ImGui::GetFrameHeight() };

			if (ImGui::Button(">", button_sz))
				request_move_selected = 0;

			ImGui::SetCursorPosY(ImGui::GetCursorPosY()); // 두 번째 버튼 간격 추가
			if (ImGui::Button("<", button_sz))
				request_move_selected = 1;

			if (request_move_all != -1)
				MoveAll(request_move_all, request_move_all ^ 1);
			if (request_move_selected != -1)
				MoveSelected(request_move_selected, request_move_selected ^ 1);

			ImGui::EndTable();
		}

		// Blend 리스트에 있는 아이템 이름 출력
		ImGui::Text("Current Blend Item: %s", GetBlendItemName());
	}
};


typedef struct AnimInterInfo
{
	string strSrcAnimName;
	string strDstAnimName;
	_float fInterDis;
	_float fInterSp;
} ANIM_INTER_INFO;


//// 뼈 정보를 저장하는 구조체
//typedef struct Bone
//{
//	string name;
//	int index;
//	int parentIndex;
//	int numChild;
//	int numTotalChildBone;
//	vector<Bone> children; // 자식 뼈를 저장하는 벡터
//
//	_bool bIsCheckedShow = false;
//	_bool bShowBonePos = false;
//
//} BONE_XML;


class CLevel_AT_Utils
{
public:
	_tchar* CharToWChar(const _char* pStrInput)
	{
		_tchar* pOut = nullptr;

		// 입력 문자열의 길이를 구하여, 출력 버퍼 크기를 계산합니다.
		_int iLen = MultiByteToWideChar(CP_ACP, 0, pStrInput, -1, NULL, 0);

		// 충분한 크기의 메모리 공간을 할당합니다.
		pOut = new _tchar[iLen];  // 메모리 할당

		// MultiByteToWideChar을 사용하여 변환된 문자열을 pStrOutput에 저장합니다.
		MultiByteToWideChar(CP_ACP, 0, pStrInput, static_cast<_int>(strlen(pStrInput) + 1), pOut, iLen);

		return pOut;
	}

	// 사용하고나서 동적할당한 메모리 해제 해야한다.
	_char* WCharToChar(const _tchar* pStrInput)
	{
		_char* pOut = nullptr;

		// 입력 문자열의 길이를 구하여, 출력 버퍼 크기를 계산합니다.
		_int iLen = WideCharToMultiByte(CP_ACP, 0, pStrInput, -1, NULL, 0, NULL, NULL);

		// 충분한 크기의 메모리 공간을 할당합니다.
		pOut = new _char[iLen];  // 메모리 할당

		// WideCharToMultiByte를 사용하여 변환된 문자열을 pStrOutput에 저장합니다.
		WideCharToMultiByte(CP_ACP, 0, pStrInput, -1, pOut, iLen, 0, 0);

		return pOut;
	}
};

class CLevel_AnimTool final : public CLevel
{
public:
	enum class FBX_LOAD_TYPE { ANIM_MAIN, ANIM_PART, };
	enum BLEND_TYPE { BLEND_ONE, BLEND_TWO, BLEND_THREE, BLEND_FOUR, BLEND_FIVE, BLEND_TYPE_END };
	enum BONE_CONTROLL_TYPE { BONE_DEFAULT, BONE_EXCLUDE };

private:
	CLevel_AnimTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_AnimTool() = default;

public:
	virtual HRESULT							Initialize() override;
	virtual void							Update(_float fTimeDelta) override;
	void									Update_Logs(_float fTimeDelta);
	virtual HRESULT							Render() override;

private:
	void									Update_Key(_float fTimeDelta);

private:
	void									InitData();
	void									InitTransformData();
	void									InitAnimationData();


	// -----------------------------------------------------------------------------------------------
	// Inverse Kinematic IK
	// -----------------------------------------------------------------------------------------------
private:
	// 스크린 좌표로 부터 월드 상의 위치 구하는 함수
	// 스크린 X, 스크린 Y, 레이 방향, 레이 시작 위치
	void									Cal_RayDir_And_RayStartPos(_int iSX, _int SY, _vector& vRayDir, _vector& vRayStartPos);

	// 마우스의 스크린 상의 위치를 반환
	POINT									Get_Mouse_ScreenPos(); 

	void									Show_IK_Options();

	// UpdateExcludeBone를 어떻게 컨트롤 할지 옵션 설정하는 함수
	void									Show_IK_Controll_UpdateExcludeBone();
	
	// 월드 상에 존재하는 마우스를 향해 UpdateExcludeBone가 회전할 때 사용하는 옵션 함수
	void									Show_IK_Controll_UpdateExcludeBone_FollowMouseInWorld();
	void									Rotate_Update_Exclude_Bone_ToMouseWorldPos();
	void									Rotate_Test_Client(); // 클라에서 회전
	void									Rotate_Test_Engine();

	_fmatrix								Get_MouseWorld_Matrix();
	_fmatrix								Get_PlayerWorld_Matrix();
	_fmatrix								Get_UpdateExcludeBone_TransformationMatrix(string strUpdateExcludeBoneName); // 애니매이션 업데이트에서 제외한 뼈의 원본(변하지 않는) 로컬 행렬 반환
	_fmatrix								Get_Parent_CombinedTransformationMatrix_From_UpdateExcludeBone(); // 애님 업데이트에서 제외된 뼈의 부모의 Com Matrix반환
	_fmatrix								Get_Bone_CombinedTransformationMatrix(string strBoneName);


	void									Show_IK_Controll_Legs();
	
	enum BONE_IK_LOOK_DIR_TYPE				{ IK_PLAYER_LOOK, IK_BONE_WORLD_LOOK, IK_BONE_LOCAL_LOOK, IK_WORLD_TO_TARGET, IK_LOOK_END};
	void									Show_IK_Ray(string strBoneName, CAT_Ray* pRay, BONE_IK_LOOK_DIR_TYPE eIKLookType);
	_vector									Get_Bone_WorldPos(_fmatrix ContainerObjectWorldMatrix, string strBoneName);
	_vector									Get_Bone_WorldLook(_fmatrix ContainerObjectWorldMatrix, string strBoneName);

	// -----------------------------------------------------------------------------------------------
	// Level_AnimTool 오브젝트 로드
	// -----------------------------------------------------------------------------------------------
private:
	HRESULT									Ready_ShadowCamera();
	HRESULT									Ready_Layer_AnimTool_Camera(const _wstring& strLayerTag);
	HRESULT									Ready_Layer_AnimTool_Object(const _wstring& strLayerTag);
	HRESULT									Ready_Layer_AnimTool_Terrain(const _wstring& strLayerTag);
	HRESULT									Ready_Layer_AnimTool_Square(const _wstring& strLayerTag);
	HRESULT									Ready_Layer_AnimTool_Mouse(const _wstring& strLayerTag);
	HRESULT									Ready_Layer_AnimTool_Stair(const _wstring& strLayerTag);
	HRESULT									Ready_Layer_AnimTool_Ray(const _wstring& strLayerTag);
	void									Spawn_Ray();

	// -----------------------------------------------------------------------------------------------
	// Level_AnimTool Imgui 창 배치, 고정
	// -----------------------------------------------------------------------------------------------
private:
	ImVec2									ImGui_Get_ViewPort_StartPos() { return ImGui::GetMainViewport()->Pos; }
	void									ImGui_Fix_Window_Pos(ImVec2 vPos);
	void									ImGui_Fix_Window_Scale(ImVec2 vScale);

	// -----------------------------------------------------------------------------------------------
	// Modal창 열고 파일 로드하는 함수들
	// - Open_Modal_Window 호출 후 pRetPath를 Open_AnimFBXFileDialog의 strInFilePath에 전달한다.
	// - 두 함수를 통해 필요한 fbx파일을 로드한다.
	// -----------------------------------------------------------------------------------------------
private:
	_bool									Open_Modal(string strEX, WCHAR* pRetPath); // WCHAR strRetPath[256] 던진다.

	// 그냥 모달창 열때
	_bool									Open_Modal_Window(const WCHAR* strOpenPath, string strEx, WCHAR* pRetPath);
	_bool									Open_AnimFBXFile_Dialog(WCHAR* strInFilePath, _uint iMaxPathLen, OPENFILENAME& ofn, MODEL_TYPE eLoadFbxType);
	_bool									Open_XMLFile_Dialog(WCHAR* strInFilePath, _uint iMaxPathLen, OPENFILENAME& ofn);
	
	_bool									Open_Save_Modal(string strEx, WCHAR* pRetPath);

	// FBX 로드 모달창 여는 함수
	void									Open_LoadAnimModal();

	string									ConvertAbsolutePath_FromRelativePath(string strRelativePath);

	// -----------------------------------------------------------------------------------------------
	// XML 로드 & 내보내기 함수들
	// -----------------------------------------------------------------------------------------------
	void									XML_Add_Comment(xml::XMLDocument* pDoc); // 저장한 날짜, 시간 추가
	void									XML_Add_Comment(xml::XMLDocument* pDoc, string strComment); // 원하는 코멘트 추가
	HRESULT									XML_Save_InterpolationTableData(string strXMLFileName); // 보간 데이터 저장 Export
	void									XML_Load_ParseAnimInterpolationData(const WCHAR* strXMLFilePath); // 보간 데이터 로드
	HRESULT									XML_Save_Bone_Hierachy(string strXMLFileName, string strBoneOwnerName); // 뼈 파싱
	_int									Read_Bone(xml::XMLDocument* pDoc, aiNode* pCurNode, xml::XMLElement* pParentElement);
	HRESULT									XML_Load_BoneHierachy(const WCHAR* strXMLFilePath);
	HRESULT									XML_Load_RootMotionBoneNameData(const WCHAR* strXMLFilePath);
	HRESULT									XML_Save_RootMotionBoneNameData(const WCHAR* pXMLSaveFilePath, string strRootMotionboneName);
	void									XML_ParseBone_ReCursion(xml::XMLElement* pParentEle, BONE_XML& Bone);

	_bool									XML_Show_SaveRootMotionBone_Btn(string strSelectedBoneName);
	HRESULT									XML_Save_Selected_BlendBones_Btn(); // 블렌딩할 뼈 정보 저장히는 함수
	void									XML_Save_Selected_BlendBones(xml::XMLDocument* pDoc, xml::XMLElement* pEleParent, BONE_XML& Bone, string strDstBoneName, _int& iMinBoneIdx, _int& iMaxBoneIdx);
	void									XML_Save_BlendBones_ReCursion(xml::XMLDocument* pDoc, xml::XMLElement* pParentEle, BONE_XML& Bone, string strDstBoneName, _bool bFind, _int& iMinBoneIdx, _int& iMaxBoneIdx);

	HRESULT									XML_Load_BlendBones(); // 블렌딩 뼈 정보 읽는 함수
	void									XML_ParseBone_Recursion(xml::XMLElement* pParentEle, BONE_XML& Bone, _bool bLoadBlendBone);

	// 이벤트 XML 저장
	HRESULT									XML_Save_Events(const WCHAR* strXMLFilePath);
	HRESULT									XML_Load_Events(const WCHAR* strXMLFilePath);

	// -----------------------------------------------------------------------------------------------
	// imgui 문자열 유틸 함수
	// -----------------------------------------------------------------------------------------------
	string									Show_StringInput(const _char* pLabel, const string& strDefaultVal = "");

	
	// -----------------------------------------------------------------------------------------------
	// Level_AnimTool 렌더링
	// -----------------------------------------------------------------------------------------------
private:
	void									Render_ImGui();	
	void									Render_Menu();				// 상단 메뉴바
	void									Render_ShowObjectInfo();	// 현재 보여주고 있는 오브젝트 상태에 대한 것들

private:
	void									Change_CurAnimIdx(_int iCurAnimIdx)
	{
		if (iCurAnimIdx < 0) return;
		m_iCurAnimIdx = iCurAnimIdx;
	}
	_int									Get_CurAnimIdx() { return m_iCurAnimIdx; }

	// Play Anim Test Btn을 누르면 기존 애니매이션 목록을 초기화 하고 선택된 애니매이션으로 새로 재생한다.
	void									PlayAnimTest_StartBtn(_int iSrcAnimIdx, _int iDstAnimIdx, PartAnimObj_Type::ANIM_PLAY_TYPE eType);
	void									PlayAnimTest_StartBtn(_int iSrcAnimIdx, PartAnimObj_Type::ANIM_PLAY_TYPE eType);


	// -----------------------------------------------------------------------------------------------
	// Level_AnimTool 상단 메뉴바 아이템들 기능 출력
	// -----------------------------------------------------------------------------------------------
private:
	void									Show_OpenItems_MenuItem();
	void									Show_OpenItems_Saves();

	// -----------------------------------------------------------------------------------------------
	// Multi Controlls
	// -----------------------------------------------------------------------------------------------
private:
	void									MultiControll_Transform_TreeNode(); // Transform관련 데이터 관리
	void									MultiControll_AnimModel_Animation_TreeNode(); // 애니매이션 
	void									MultiControll_Bone(); // 본
	void									MultiControll_ModelMeshNames_TreeNode(); // 메쉬
	void									TestGizmo();
	void									MultiControll_IK_TreeNode(); // 메쉬

	// -----------------------------------------------------------------------------------------------
	// Animation KeyFrame
	// -----------------------------------------------------------------------------------------------
	void									Begin_Animation_KeyFrame(); // 애니매이션 키 프레임 창 시작 함수

	// Get_Timeine으로 가저온 객체에 라인 그리기
	void									Draw_KeyFrame_TimeLine();

	// 메인 프레임 그리기 (FrameStep반환)
	// 실제 프레임 그리기 | | | 이런 선 그리기
	_float									Draw_TimeLine(ImDrawList* pDrawList, _float fTimeLineWidth, ImVec2 vCursorPos, _float fCurAnimDuration);
	
	// TrackPosBar 전체 컨트롤 함수
	void									Controll_TrackPosBar(ImDrawList* pDrawList, _float* pCurTrackPos, _float fFrameLineStartStartX, _float fFrameLineStartY, _float fFrameLineEndX, _float fTrackPosBarSizeY, _float fFrameStep);

	pair<_bool, _bool>						DrawOn_TrackPosBar(ImDrawList* pDrawList, _float* pCurTrackPos, _float fFrameLineStartStartX, _float fFrameLineStartY, _float fFrameLineEndX, _float fTrackPosBarSizeY, _float fFrameStep);
	// TrackPosBar에 Hover시 발생하는 이벤트들
	void									HoverOn_TrackPosBar(_bool bHover, _float fCurTrackPos);
	void									Draw_Events(_float fFrameLineStartStartX, _float fFrameLineStartY, _float fFrameLineEndX);
	void									Draw_Event(ImDrawList* pDrawList, string strCurAnimName, string strEventLayerName, _float fStartPosY, _float fFrameStep);

	// 타임 라인 시작점, 현재 애니매이션 TrackPos (재생 위치), 현재 타임 라인 프레임에서의 | | | 사이의 간격을 의미
	_float									Cal_TrackPosX(_float fLineStartX, _float fTrackPos, _float fFrameStep)
	{
		return fLineStartX + fTrackPos * fFrameStep;
	}
	

	// -----------------------------------------------------------------------------------------------
	// Transform
	// -----------------------------------------------------------------------------------------------
private:
	_bool									Controll_WorldPos(CTransform* pCom);
	_bool									Controll_Rotation(CTransform* pCom);
	_bool									Controll_Scale(CTransform* pCom);


	// -----------------------------------------------------------------------------------------------
	// Multi Controlls AnimationInfo
	// -----------------------------------------------------------------------------------------------
private:
	_bool									Show_AnimInfo_Text(_int iCurAnimIdx);
	_bool									Show_AnimRootMotionType_RadioBtn();
	_bool									Show_AnimPlayType_ComboBox();
	_bool									Show_InterpolationTable();
	_bool									Show_Anim_KeyFrames();


	// 선택 가능한 애니매이션 목록들을 출력하는함수, 선택하고 현재 애니매이션 변경이 가능하다.
	void									Show_Default_AnimPlayTest();
	void									Show_Selectable_AnimList_ListBox();
	void									Show_AnimInfos_ListBox();	// -- Anim KeyFrame 관련 데이터 출력
	void									Show_PlayAnim_Btn(const _char* leftLabel, const _char* midLabel, const _char* rightLabel, ImVec2 buttonSize);
	void									Show_Loop_Default_AnimPlayTest();
	void									Show_Loop_Interpolation_AnimPlayTest();		// -- Anim 보간 플레이 테스트
	void									Show_Loop_Blending_AnimPlayTest();
	void									Show_Blending_List(ExampleDualListBox& pArr, ANIM_BLEND_GROUP_TYPE eBlendType);
	void									Show_BlendBone_ComboBox();
	void									Move_All_Left(ExampleDualListBox Arr[]);
	void									Show_Loop_Blend_Interpolation();

	// 기능 순서대로 적음
	void									Show_PlayAnimTest_MultiDefault_Second();
	void									Show_PlayAnimTest_MultiDefault_Third();
	void									Show_PlayAnimTest_MultiInterpolation_Second();
	void									Show_Interpolation_UseXMLData();


	// 보간 거리, 속도를 직접 설정하여 테스트하게 해주는 함수.
	void									Show_Interpolation_UseCustumData();
	void									Show_PlayAnimTest_MultiInterpolation_Third();
	void									Show_InterpolationInfo();
	void									Init_InterpolationInfoTable();
	void									Show_InterpolationTableWindow();
	void									Play_Anim(_int iAnimIdx, _bool bLoop = true);


	// -----------------------------------------------------------------------------------------------
	// Bone
	// -----------------------------------------------------------------------------------------------
	void									MenuItem_Save_BoneHierachy();
	_bool									Show_LoadBoneData_Btn();				// 뼈 정보 로드하는 버튼 렌더 함수
	_bool									Show_Load_RootMotionBoneName_Btn();		// 루트 모션 뼈 정보 로트 버튼
	_bool									Begin_Show_BoneHierarchy();				// 뼈 전체 구조 렌더 함수(new ui)
	_bool									Show_BoneWorldPos();					// 선택한 뼈를 월드에 보여주는 함수


	_bool									Show_Clear_AllSelected_ShowBones_Btn();		// 월드에 보여주기로한 뼈 전부 삭제
	_bool									Show_Save_BlendBone_Btn();
	void									Clear_LoadBoneInfo(BONE_XML& Bone);
	string									Show_BoneInfo_Recur_TreeNode(BONE_XML& Bone);
	HRESULT									Spawn_ShowBoneSquare(string strShowBoneName);
	HRESULT									Remove_ShowboneSquare(string strShowBoneName);

	void									Show_BoneControll_Default(string strBoneName);
	void									Show_BoneControll_Exclude(string strBoneName);
	void									Controll_Bone_Matrix(string strBoneName);
	
	void									Active_Ray_AtBone();

	// -----------------------------------------------------------------------------------------------
	// Level_Anim 로드한 오브젝트 메쉬 정보 출력
	// -----------------------------------------------------------------------------------------------
	void									Show_MeshNames_ListBox();			// -- 
	void									Show_LoadPartAnimModel_FBX_Radio(); // -- 


	// 로드할 상대 경로, FBX 로드 타입, 모델 Prototype이름, 객체 Prototype이름(Loader에서 지정한 이름)
	HRESULT									Load_AnimModel_FBX_File(WCHAR* strFBXModelFilePath, ANIM_MODEL_FBX_LOAD_TYPE eFBXLoadType, const _wstring& strPrototypeModelName, const _wstring& strPrototypeGameObjectName);
	HRESULT									Load_AnimMain(_wstring strProtoModelName, _wstring strGameObjectCloneName);
	HRESULT									Load_NonAnimModel_FBX_File(WCHAR* strFBXModelFilePath);

private:
	void									Set_CurShowObject(CAT_Obj* pATO) { m_pAT_Obj = pATO; }
	CAT_Obj*								Get_CurShowObject() { return m_pAT_Obj; }
	

public:
	void									Add_Log(const string& strLog) { m_LogBuffer.push_back(strLog); }
	void									Add_Log(AT_LOG_TYPE eLogType, AT_LOG_JOB eLogJob, string funcMsg, string Msg);
	void									Show_AnimLog();

private:
	_bool									m_bMultiControllsTransform[AT_MC_TRANSFORM_TYPE::AT_MC_TRANSFORM_TYPE_END]{};
	_bool									m_bMultiControllsAnimation[AT_MC_ANIMATION_TYPE::AT_MC_ANIMATION_TYPE_END]{};
	_bool									m_bMultiControllsBone[AT_MC_BONE_TYPE::AT_MC_BONE_TYPE_TYPE_END]{};
	_bool									m_bBeginBoneHierarchy[AT_BONE_HIERARCHY_TYPE::BONE_HIERARCHY_TYPE_END]{};


private:
	CLevel_AT_Utils*						m_pAnimToolUtilObj = nullptr;	
	OPENFILENAME							m_ofn = {}; // 모달 객체

	// 테스트 모델 객체
private:
	CAT_Obj*								m_pAT_Obj = nullptr;
	vector<CAT_PartAnimObj*>				m_pAT_PartAnimObjs;
	CAT_PartAnimObj*						m_pAT_PartAnimObj = nullptr;
	CTransform*								m_pAT_PartAnimTransform = nullptr;
	CModel*									m_pAT_PartAnimObj_Model = nullptr;

	// ------------------------------------------------------------------
	// Tranform
	// ------------------------------------------------------------------
private:
	_float									m_vCurObjPosInput[3] = { 0.f, 0.f, 0.f };
	_vector									m_vRightUpLook[3] = {};
	_float									m_fCurRotVal[3] = { 0.f, 0.f, 0.f };
	_float									m_fPrevRotVal[3] = { 0.f, 0.f, 0.f };
	_float									m_fCurScale[3] = { 1.f, 1.f, 1.f };
	

	// ------------------------------------------------------------------
	// Animation
	// ------------------------------------------------------------------
private:
	ANIM_PLAY_TEST_TYPE						m_eAnimPlayTestType = ANIM_PLAY_TEST_TYPE::DEFAULT; // 재생 테스트 타입
	vector<const _char*>					m_AnimNamesCStr;
	_int									m_iCurAnimIdx = 0;	// 현재 재생 애니매이션 인덱스
	_float									m_fTickPerSecond = 1.f;
	_bool									m_bIsSrcDstPlayType = false;
	string									m_strCurFileName = "";

	// 보간 테이블 관련 멤버 변수
	vector<ANIM_INTER_INFO>					m_InterpolationTableData;
	_bool									m_bSaveInterDataFromTable = false;
	_int									m_iTotInterDataSize = 0;
	_int									m_iCurInterDataSaveCnt = 0;
	_bool									m_bShowInterTable = false;

	_bool									m_bLoadSuccessBlendBones = false; // 250125


	// ------------------------------------------------------------------
	// Bone
	// ------------------------------------------------------------------
private:
	_bool									m_bShowBoneHierachyTable = false;
	vector<_int>							m_BonesIndices;
	BONE_XML								m_RootBone{};
	_bool									m_bBoneLoadSuccess = false;

	deque<string>							m_SelectedShowBonesNames;
	map<string, pair<string, CAT_Square*>>	m_SelectedShowBoneSquare; // 뼈이름, <오브젝트 이름, 실 오브젝트*>

	const _char* m_BlendBoneTypes[tot + 1] =		{ "None", "LowerBody", "UpperBody", "ArmLeft", "ArmRight", "Head", "Total"};

	// 블렌딩 할 뼈 원본을 담당
	deque<pair<string, _int>>				m_BlendBones; // 뼈 이름, 뼈 인덱스

	// 실제 블렌딩 대상의 뼈를 가지고 있음
	deque<pair<string, pair<_int, ANIM_BLEND_GROUP_TYPE>>>			m_SelectedBlendBones; // 뼈 이름, 뼈 인덱스
	_float									m_vInputPosSlider[3] = { 0.f, };

	// 뼈 컨트롤 IK
	BONE_CONTROLL_TYPE						m_eBoneControllType = BONE_DEFAULT;

	// 뼈 로컬 행렬 주소를 담고 있는 컨테이너, 각 뼈의 m_TransformationMatrix를 가지고 있다.
	map<string, _float4x4*>					m_BoneTransformationMatrices;

	// 해당 뼈를 애니매이션 업데이트에서 제외시킬 것인지 아닌지에대한 여부
	map<string, _bool>						m_BoneCheckUpdateExcludeBone;

	// 뼈 로컬 행렬 원본을 담고 있는 컨테이너, 각 뼈의 m_TransformationMatrix의 원본을 가지고 있다.
	map<string, _float4x4>					m_BoneOriginTransformationMatrices;

	map<string, _float[3]>					m_BoneMatrixPosInput;
	map<string, _bool>						m_BoneMatrixSRTChange;
	map<string, _float[3]>					m_BoneMatrixRotInput;



	// 애니매이션 로그 관련 출력 멤버 변수
private:
	vector<string>							m_LogBuffer;
	ImGuiTextFilter							m_LogFilter;

private:
	_int									m_iCurMeshIdx = 0;
	vector<string>							m_MeshNames;
	string									m_strCurMeshName = "";

	// 모달창을 통해서 fbx파일 로드하는 경우 사용하는 멤버 변수들 Open_Modal_Window, Load_AnimModel_FBX_File에 사용
private:
	_int									m_iModelNum = 0;
	WCHAR									m_strModelAbPath[256] = L"../Bin/Resources/Models";
	string									m_strModelEx = "fbx";
	_wstring								m_strModelPrototypeName = L"Prototype_Component_Model_";

private:
	// 보간 테스트 버튼을 눌렸는지 여부
	_bool									m_bKeyDown_PlayDoubleAnimLoopInterpolationTest = false;
	_bool									m_bKeyDown_PlayDoubleAnimLoopInterpolationTestWithXML = false;



	// -----------------------------------------------------------------------------------------------
	// Animation KeyFrame
	// -----------------------------------------------------------------------------------------------
	_float										m_fFrameLineStartX = 0.f;
	_float										m_fFrameLineStartY = 0.f;
	_float										m_fFrameLineLen = 40.f;
	_float										m_fFramePadding = 90.f;
	_float										m_fTimeLinePadding = 30.f;
	_float										m_fTimeLineWidth = 0.f;
	_float										m_fCurAnimDuration = 0.f;
	_float*										m_pCurAnimTrackPos = nullptr;
	deque<_float>								m_CurEventTrackPoses;

	// map<애니매이션 이름, map<이벤트 레이어 이름, vector<이벤트 이름, 시간>>>
	map<string, map<string, vector<pair<string, _float>>>> m_Events;
	// map<string, vector<pair<string, _float>> >>


	// -----------------------------------------------------------------------------------------------
	// IK
	// -----------------------------------------------------------------------------------------------
	CAT_Mouse*									m_pMouse = nullptr;
	CAT_Stair*									m_pST = nullptr;
	CTransform*									m_pSTTransformCom = nullptr;
	CModel*										m_pSTModelCom = nullptr;

	vector<CAT_Ray*>							m_RayVec;

	map<string, _bool>							m_UsingRaysPlayerLook;
	map<string, _bool>							m_UsingRaysBoneLook;
	map<string, _bool>							m_UsingRaysBoneLocalLook;


	map<string, CAT_Ray*>						m_RaysPlayerLook;
	map<string, CAT_Ray*>						m_RaysBoneLook;
	map<string, CAT_Ray*>						m_RaysBoneLocalLook;

	_bool										m_bShowDirBoneToTarget = false;
	pair<string, CAT_Ray*>						m_ShowDirRay = { "None", nullptr };

private:
	const ImVec2							m_ShowInfoWindowScale = { 0.f, g_iWinSizeY - 200.f };
	const ImVec2							m_LogWindowScale = { 0.f, 200.f };

	// Key
private:
	_bool									m_bCtrl = false;


public:
	static CLevel_AnimTool*					Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void							Free() override;
};

END
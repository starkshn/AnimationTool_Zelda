#include "stdafx.h"
#include "..\Public\Level_AnimTool.h"
#include "GameInstance.h"
#include "AT_Cam.h"

#include <windows.h> // Windows API 사용

// ------------------------------------------
// Engine Headers
// ------------------------------------------
#include "Animation.h"
#include "Channel.h"
#include "Bone.h"
#include "Mesh.h"


#pragma warning(disable: 4996)
#define _CRT_SECURE_NO_WARNINGS

namespace fs = std::filesystem;
using namespace std;

// 각 아이콘의 색상과 설명
struct IconInfo
{
    ImVec4 color;    // RGBA 색상
    const char* text; // 설명 텍스트
};

IconInfo icons[] =
{
    { ImVec4(1.0f, 0.0f, 0.0f, 1.0f), " : Red" },
    { ImVec4(0.0f, 1.0f, 0.0f, 1.0f), " : Green" },
    { ImVec4(0.0f, 0.0f, 1.0f, 1.0f), " : Blue" },
    { ImVec4(1.0f, 0.0f, 1.0f, 1.0f), " : Mazenta" }
};

// 아이콘 크기
const float iconSize = 20.0f;


// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}


CLevel_AnimTool::CLevel_AnimTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:
	CLevel(pDevice, pContext)
{
}

HRESULT CLevel_AnimTool::Initialize()
{
    m_pAnimToolUtilObj = new CLevel_AT_Utils;

    // Light
    {
        LIGHT_DESC	LightDesc{};
        ZeroMemory(&LightDesc, sizeof LightDesc);

        LightDesc.eType = LIGHT_DESC::TYPE_DIRECTOINAL;
        LightDesc.vDirection = _float4{ 0.f, -1.f, 0.f, 0.f };
        LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
        LightDesc.vAmbient = _float4(0.5f, 0.5f, 0.5f, 1.f);
        LightDesc.vSpecular = _float4(0.1f, 0.1f, 0.1f, 0.0f);

        if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
            return E_FAIL;
    }

    if (FAILED(Ready_Layer_AnimTool_Camera(TEXT("Layer_AnimToolCamera"))))
        return E_FAIL;

    if (FAILED(Ready_Layer_AnimTool_Object(TEXT("Layer_AnimToolObject"))))
        return E_FAIL;

    if (FAILED(Ready_Layer_AnimTool_Terrain(TEXT("Layer_AnimToolTerrain"))))
        return E_FAIL;

    if (FAILED(Ready_Layer_AnimTool_Square(TEXT("Layer_AnimToolSquare"))))
        return E_FAIL;

    if (FAILED(Ready_Layer_AnimTool_Stair(TEXT("Layer_AnimToolStair"))))
        return E_FAIL;

    if (FAILED(Ready_Layer_AnimTool_Ray(TEXT("Layer_AnimToolRay"))))
        return E_FAIL;
    
    
	return S_OK;
}


void CLevel_AnimTool::Update(_float fTimeDelta)
{


    Update_Key(fTimeDelta);

    // 모든 imgui 렌더
    Render_ImGui();
    Update_Logs(fTimeDelta);
}

void CLevel_AnimTool::Update_Logs(_float fTimeDelta)
{
    if (m_pAT_PartAnimObj)
    {
        if (m_pAT_PartAnimObj->Get_Logs().size())
        {
            for (auto strLog : m_pAT_PartAnimObj->Get_Logs())
                Add_Log(strLog);

            m_pAT_PartAnimObj->Clear_Logs();
        }
    }
}

HRESULT CLevel_AnimTool::Render()
{
    wstring strFrame;
    strFrame = TEXT("Animation Tool");
    SetWindowText(g_hWnd, strFrame.c_str());

	return S_OK;
}

void CLevel_AnimTool::Update_Key(_float fTimeDelta)
{
    if (KEY_DOWN(KEY::CTRL))
        m_bCtrl = true;
    else if (KEY_UP(KEY::CTRL))
        m_bCtrl = false;

    // 단축키


    if (m_bCtrl && KEY_DOWN(KEY::O))
    {
        Open_LoadAnimModal();
    }

    if (m_bCtrl && (KEY_DOWN(KEY::I)))
    {
        WCHAR strFilePath[256] = {};
        Open_Modal("xml", strFilePath);
        XML_Load_ParseAnimInterpolationData(strFilePath);
    }

    // fbx 파일 뼈정보 로드
    if (m_bCtrl && (KEY_DOWN(KEY::B)))
    {
        WCHAR strFilePath[256];
        if (Open_Modal("xml", strFilePath))
        {
            if (FAILED(XML_Load_BoneHierachy(strFilePath)))
            {
                Add_Log(LOG_FAIL, XML_LOAD, "Show_LoadBoneData_Btn", "load from : " + CONVERTTOSTRING(strFilePath));
            }
            else
            {
                Add_Log(LOG_OK, XML_LOAD, "Show_LoadBoneData_Btn", "load from : " + CONVERTTOSTRING(strFilePath));
            }
        }
    }

    // 루트모션 뼈 로드
    if (m_bCtrl && (KEY_DOWN(KEY::R)))
    {
        WCHAR strFilePath[256];
        if (Open_Modal("xml", strFilePath))
        {
            if (FAILED(XML_Load_RootMotionBoneNameData(strFilePath)))
                Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_RootMotionBoneNameData", "load fail");
        }

        Add_Log(LOG_OK, XML_LOAD, "Show_Load_RootMotionBoneName_Btn", "load root motion bone name datas");
    }

    if (m_bCtrl && (KEY_DOWN(KEY::V)))
    {
        if (FAILED(XML_Load_BlendBones()))
        {
            MSG_BOX("Load BlendBones Failed");
        }
    }
}


HRESULT CLevel_AnimTool::Ready_ShadowCamera()
{
    m_pGameInstance->SetUp_Shadow_TransformMatrix(CShadow::D3DTS_VIEW, XMMatrixLookAtLH(XMVectorSet(-10.f, 50.f, -10.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
    m_pGameInstance->SetUp_Shadow_TransformMatrix(CShadow::D3DTS_PROJ, XMMatrixPerspectiveFovLH(XMConvertToRadians(60.f), static_cast<_float>(g_iWinSizeX) / g_iWinSizeY, 0.1f, 1000.f));

    return S_OK;
}

HRESULT CLevel_AnimTool::Ready_Layer_AnimTool_Camera(const _wstring& strLayerTag)
{
    CAnimTool_Camera::CAMERA_FREE_DESC Desc = {};

    Desc.fSpeedPerSec = 10.f;
    Desc.fRotationPerSec = XMConvertToRadians(180.f);
    Desc.fMouseSensor = 0.1f;
    Desc.fFovy = XMConvertToRadians(60.f);
    Desc.fAspect = static_cast<_float>(g_iWinSizeX) / g_iWinSizeY;
    Desc.fNear = 0.1f;
    Desc.fFar = 1000.f;
    Desc.vEye = _float3(0.f, 2.f, 5.f);
    Desc.vAt = _float3(0.f, 0.f, 0.f);

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_ANIMTOOL, TEXT("Prototype_GameObject_AnimToolCamera"), LEVEL_ANIMTOOL, strLayerTag, &Desc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_AnimTool::Ready_Layer_AnimTool_Object(const _wstring& strLayerTag)
{
    
    return S_OK;
}

HRESULT CLevel_AnimTool::Ready_Layer_AnimTool_Terrain(const _wstring& strLayerTag)
{
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_ANIMTOOL, TEXT("Prototype_GameObject_AT_Terrain"), LEVEL_ANIMTOOL, strLayerTag)))
        return E_FAIL;
    return S_OK;
}

HRESULT CLevel_AnimTool::Ready_Layer_AnimTool_Square(const _wstring& strLayerTag)
{
   /* if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_ANIMTOOL, TEXT("Prototype_GameObject_AT_Square"), LEVEL_ANIMTOOL, strLayerTag, nullptr, L"SS")))
        return E_FAIL;*/
    return S_OK;
}

HRESULT CLevel_AnimTool::Ready_Layer_AnimTool_Mouse(const _wstring& strLayerTag)
{
    return S_OK;
}

HRESULT CLevel_AnimTool::Ready_Layer_AnimTool_Stair(const _wstring& strLayerTag)
{
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_ANIMTOOL, TEXT("Prototype_GameObject_AT_Stair"), LEVEL_ANIMTOOL, strLayerTag, nullptr, L"Stair")))
        return E_FAIL;
    return S_OK;
}

HRESULT CLevel_AnimTool::Ready_Layer_AnimTool_Ray(const _wstring& strLayerTag)
{
    /*if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_ANIMTOOL, TEXT("Prototype_GameObject_AT_Ray"), LEVEL_ANIMTOOL, strLayerTag, nullptr, L"Ray")))
        return E_FAIL;*/

    Spawn_Ray();

    return S_OK;
}

void CLevel_AnimTool::Spawn_Ray()
{
    static _int iNum = 0;
    for (_int i = 0; i < 30; ++i)
    {
        wstring strObjName = L"Ray" + to_wstring(iNum++);
        if (m_pGameInstance->Add_GameObject_ToLayer(LEVEL_ANIMTOOL, TEXT("Prototype_GameObject_AT_Ray"), LEVEL_ANIMTOOL, L"Layer_Ray", nullptr, strObjName) != S_OK)
        {
            MSG_BOX("Spawn Fail Ray");
            return;
        }

        CAT_Ray* pRay = static_cast<CAT_Ray*>(m_pGameInstance->Find_Object_By_Name(LEVEL_ANIMTOOL, L"Layer_Ray", strObjName));
        pRay->Set_Use(false);
        m_RayVec.push_back(pRay);
    }

     _int a = 10;
}

void CLevel_AnimTool::Render_ImGui()
{
	Render_Menu();

    if (!m_pAT_Obj)
        return;

    Render_ShowObjectInfo(); // 뼈 사각형 출력
    Active_Ray_AtBone(); // 뼈 레이 출력
}

void CLevel_AnimTool::Render_Menu()
{
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255)); // 흰색 텍스트

    // 창 위치 고정
    {
        // 메인 뷰포트를 가져옴 (프로그램 창의 클라이언트 영역 정보)
        // ImGuiViewport* mainViewport = ImGui::GetMainViewport();

        // 프로그램 창의 좌상단을 기준으로 창 위치 설정
        ImGui_Fix_Window_Pos(ImGui_Get_ViewPort_StartPos());

        // 창 크기 고정 (원하는 크기로 설정 가능)
        ImGui_Fix_Window_Scale(ImVec2(g_iWinSizeX, 30));
    }

    ImGui::Begin("Anim Tool", nullptr, window_flags | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) // 메뉴바 시작
    {

        if (ImGui::BeginMenu("Load")) // "Menu"는 메뉴 이름입니다.
        {
            // 메뉴 내용 추가 가능
            Show_OpenItems_MenuItem();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Save")) // "Examples"는 메뉴 이름입니다.
        {
            Show_OpenItems_Saves();

            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("Help"))
        {
            ImGui::TextLinkOpenURL("https://imported-denim-880.notion.site/AnimTool-1814bccaf9eb802e83f9f8897c49d9ab?pvs=4");

            /*if (ImGui::MenuItem())
            {
                ImGui::BulletText("Read the FAQ at ");
                ImGui::SameLine(0, 0);
                ;
            }*/

            ImGui::EndMenu();
        }


        ImGui::EndMenuBar(); // 메뉴 바 종료
    }

    // ImGui 코드
    ImGui::PopStyleColor();

    ImGui::End();
}

void CLevel_AnimTool::Render_ShowObjectInfo()
{
    ImGui_Fix_Window_Pos(ImVec2{ ImGui_Get_ViewPort_StartPos().x, ImGui_Get_ViewPort_StartPos().y + 30.f});

    if (m_pAT_Obj)
    {
        _bool bOpen = ImGui::Begin("Load Object Info", nullptr, ImGuiWindowFlags_MenuBar | ImGuiTableFlags_SizingStretchProp);
        if (bOpen)
        {
            // ShowObject의 Transform 데이터를 띄운다.
            MultiControll_Transform_TreeNode(); 

            // AnimModel의 경우 애니매이션 정보를 띄운다.
            if (Engine::MODEL_TYPE::TYPE_ANIM == m_pAT_PartAnimObj_Model->Get_ModelType())
                MultiControll_AnimModel_Animation_TreeNode();

            // fbx 본 정보 띄우는 함수
            MultiControll_Bone();

            // 메쉬 정보들을 띄우는 함수
            MultiControll_ModelMeshNames_TreeNode(); 

            // TestGizmo();

            MultiControll_IK_TreeNode();
        }
        ImGui::End();
    }

   
    Show_AnimLog();
}

_bool CLevel_AnimTool::Open_Modal(string strEx, WCHAR* pRetPath)
{
    ZeroMemory(&m_ofn, sizeof(m_ofn));
    WCHAR szFile[256] = { 0 };     // 
    // 선택된 파일 경로를 저장할 버퍼
    m_ofn.lStructSize = sizeof(m_ofn);
    m_ofn.hwndOwner = nullptr;       // 소유자 창 핸들 (nullptr이면 기본 창)
    m_ofn.lpstrFile = szFile;        // 파일 경로를 저장할 버퍼
    m_ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    m_ofn.nFilterIndex = 1;          // 기본 필터 인덱스
    m_ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;       // 파일 존재 조건

    // strEx 값에 따라 lpstrFilter 설정
    if (strEx == "fbx")
    {
        m_ofn.lpstrFilter = L"FBX Files (*.fbx)\0*.fbx\0All Files (*.*)\0*.*\0\0";
    }
    else if (strEx == "png")
    {
        m_ofn.lpstrFilter = L"PNG Files (*.png)\0*.png\0All Files (*.*)\0*.*\0\0";
    }
    else if (strEx == "jpg")
    {
        m_ofn.lpstrFilter = L"JPG Files (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0All Files (*.*)\0*.*\0\0";
    }
    else if (strEx == "xml")
    {
        m_ofn.lpstrFilter = L"FBX Files (*.xml)\0*.xml\0All Files (*.*)\0*.*\0\0";
    }
    else
    {
        // 기본 필터: 모든 파일
        m_ofn.lpstrFilter = L"All Files (*.*)\0*.*\0\0";
    }

    m_ofn.lpstrDefExt = std::wstring(strEx.begin(), strEx.end()).c_str(); // 기본 확장자 설정

    WCHAR strPath[256] = L"../Bin/Resources/Models/";

    // 상대 경로를 절대 경로로 변환
    fs::path relativePath(strPath);
    fs::path absolutePath = fs::absolute(relativePath);

    // 절대 경로를 WCHAR 배열로 변환하여 초기 디렉토리에 설정
    _wstring strInitDir = absolutePath.wstring();
    m_ofn.lpstrInitialDir = strInitDir.c_str();

    // 파일 선택 창 열기
    if (GetOpenFileName(&m_ofn) == TRUE)
    {
        WCHAR strPath[256] = L"";

        // 선택된 파일 경로를 strInFilePath로 복사
        wcsncpy(strPath, m_ofn.lpstrFile, sizeof(strPath) / sizeof(WCHAR) - 1);

        const _char* strOutPath = m_pAnimToolUtilObj->WCharToChar(strPath);

        // 선택된 파일 경로 출력
        cout << "선택된 파일 경로: " << strOutPath << endl;

        delete[] strOutPath;

        // 최종 경로를 복사해준다.
        wcsncpy(pRetPath, strPath, sizeof(strPath) / sizeof(WCHAR) - 1);
        return true;
    }
    return false;
}

_bool CLevel_AnimTool::Open_Modal_Window(const WCHAR* strOpenPath, string strEx, WCHAR* pRetPath)
{
    if (strEx != "fbx")
    {
        MSG_BOX("확장자가 fbx가 아닙니다. (fbx로 매개 변수 전달)");
        return 0;
    }

    ZeroMemory(&m_ofn, sizeof(m_ofn));

    WCHAR szFile[256] = { 0 };     // 선택된 파일 경로를 저장할 버퍼

    m_ofn.lStructSize = sizeof(m_ofn);
    m_ofn.hwndOwner = nullptr;       // 소유자 창 핸들 (nullptr이면 기본 창)
    m_ofn.lpstrFile = szFile;        // 파일 경로를 저장할 버퍼
    m_ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    m_ofn.nFilterIndex = 1;          // 기본 필터 인덱스
    m_ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;       // 파일 존재 조건
    m_ofn.lpstrFilter = L"fbx Files (*.fbx)\0*.fbx\0\0";  // 파일 필터
    m_ofn.lpstrDefExt = std::wstring(strEx.begin(), strEx.end()).c_str(); // 기본 확장자 설정

    // 확장자에 따른 필터 설정
    if (strEx == "fbx")
    {
        m_ofn.lpstrFilter = L"FBX Files (*.fbx)\0*.fbx\0All Files (*.*)\0*.*\0\0";
    }
    



    WCHAR strPath[256] = L"../Bin/Resources/Models/";

    // 상대 경로를 절대 경로로 변환
    fs::path relativePath(strPath);
    fs::path absolutePath = fs::absolute(relativePath);

    // 절대 경로를 WCHAR 배열로 변환하여 초기 디렉토리에 설정
    _wstring strInitDir = absolutePath.wstring();
    m_ofn.lpstrInitialDir = strInitDir.c_str();

    // 파일 선택 창 열기
    if (GetOpenFileName(&m_ofn) == TRUE)
    {
        WCHAR strPath[256] = L"../Bin/Resources/Models/";

        // 선택된 파일 경로를 strInFilePath로 복사
        wcsncpy(strPath, m_ofn.lpstrFile, sizeof(strPath) / sizeof(WCHAR) - 1);

        const _char* strOutPath = m_pAnimToolUtilObj->WCharToChar(strPath);

        // 선택된 파일 경로 출력
        cout << "선택된 파일 경로: " << strOutPath << endl;

        delete[] strOutPath;

        // 최종 경로를 복사해준다.
        wcsncpy(pRetPath, strPath, sizeof(strPath) / sizeof(WCHAR) - 1);

        return true;
    }
    else
        return false;
}

_bool CLevel_AnimTool::Open_AnimFBXFile_Dialog(WCHAR* strInFilePath, _uint iMaxPathLen, OPENFILENAME& ofn, Engine::MODEL_TYPE eLoadFbxType)
{
    return false;
}

_bool CLevel_AnimTool::Open_XMLFile_Dialog(WCHAR* strInFilePath, _uint iMaxPathLen, OPENFILENAME& ofn)
{
    return _bool();
}

_bool CLevel_AnimTool::Open_Save_Modal(string strEx, WCHAR* pRetPath)
{
    ZeroMemory(&m_ofn, sizeof(m_ofn));
    WCHAR szFile[256] = { 0 }; // 저장할 파일 이름 버퍼
    m_ofn.lStructSize = sizeof(m_ofn);
    m_ofn.hwndOwner = nullptr; // 소유자 창 핸들
    m_ofn.lpstrFile = szFile;  // 파일 이름 버퍼
    m_ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    m_ofn.nFilterIndex = 1;    // 기본 필터 인덱스
    m_ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT; // 저장 경로 확인 및 덮어쓰기 확인

    // strEx 값에 따라 lpstrFilter 설정
    if (strEx == "xml")
    {
        m_ofn.lpstrFilter = L"XML Files (*.xml)\0*.xml\0All Files (*.*)\0*.*\0\0";
    }
    else
    {
        m_ofn.lpstrFilter = L"All Files (*.*)\0*.*\0\0";
    }

    m_ofn.lpstrDefExt = std::wstring(strEx.begin(), strEx.end()).c_str(); // 기본 확장자 설정

    WCHAR strPath[256] = L"../Bin/Resources/Models/";

    // 상대 경로를 절대 경로로 변환
    fs::path relativePath(strPath);
    fs::path absolutePath = fs::absolute(relativePath);

    // 절대 경로를 WCHAR 배열로 변환하여 초기 디렉토리에 설정
    _wstring strInitDir = absolutePath.wstring();
    m_ofn.lpstrInitialDir = strInitDir.c_str();

    // 파일 저장 창 열기
    if (GetSaveFileName(&m_ofn) == TRUE)
    {
        wcsncpy(pRetPath, m_ofn.lpstrFile, sizeof(szFile) / sizeof(WCHAR) - 1);
        return true;
    }
    return false;
}

void CLevel_AnimTool::Open_LoadAnimModal()
{
    ZeroMemory(&m_ofn, sizeof(m_ofn));
    {
        WCHAR szFile[256] = { 0 };     // 선택된 파일 경로를 저장할 버퍼
        m_ofn.lStructSize = sizeof(m_ofn);
        m_ofn.hwndOwner = nullptr;       // 소유자 창 핸들 (nullptr이면 기본 창)
        m_ofn.lpstrFile = szFile;        // 파일 경로를 저장할 버퍼
        m_ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
        m_ofn.nFilterIndex = 1;          // 기본 필터 인덱스
        m_ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;       // 파일 존재 조건
        m_ofn.lpstrFilter = L"fbx Files (*.fbx)\0*.fbx\0\0";  // 파일 필터
        m_ofn.lpstrDefExt = L"fbx";     // 기본 확장자
    }

    WCHAR strRetModelPath[256]; // 로드할 모델 최종 경로
    if (Open_Modal_Window(m_strModelAbPath, m_strModelEx, strRetModelPath))
    {
        fs::path fullpath = strRetModelPath;
        _wstring filename = fullpath.stem().wstring(); // stem()은 확장자를 제거한 파일 이름 반환

        // 모델 프로토 타입 이름 생성
        _wstring strModelPrototypeName = m_strModelPrototypeName + filename;

        if (FAILED(Load_AnimModel_FBX_File(strRetModelPath, ANIM_MODEL_FBX_LOAD_TYPE::ANIM_MAIN, strModelPrototypeName, TEXT("Prototype_GameObject_AnimToolObject"))))
        {
            MSG_BOX("Load_AnimModel_FBX_File Error!");
            return;
        }
    }
    else
    {

    }
}

string CLevel_AnimTool::ConvertAbsolutePath_FromRelativePath(string strRelativePath)
{
    // 상대 경로를 절대 경로로 변환
    fs::path relativePath(strRelativePath);
    fs::path absolutePath = fs::absolute(relativePath);
    return absolutePath.string();
}


void CLevel_AnimTool::ImGui_Fix_Window_Pos(ImVec2 vPos)
{
    // 창 위치 고정
    {
        // 메인 뷰포트를 가져옴 (프로그램 창의 클라이언트 영역 정보)
        ImGuiViewport* mainViewport = ImGui::GetMainViewport();

        // 프로그램 창의 좌상단을 기준으로 창 위치 설정
        ImGui::SetNextWindowPos(vPos, ImGuiCond_Always);
    }
}

void CLevel_AnimTool::ImGui_Fix_Window_Scale(ImVec2 vScale)
{
    // 메인 뷰포트를 가져옴 (프로그램 창의 클라이언트 영역 정보)
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    // 창 크기 고정 (원하는 크기로 설정 가능)
    ImGui::SetNextWindowSize(ImVec2(vScale.x, vScale.y), ImGuiCond_Always);
}

void CLevel_AnimTool::Show_OpenItems_MenuItem()
{
    enum MENU_TYPE 
    { LOAD_ANIM_FBX, LOAD_NON_ANIM_FBX, LOAD_ANIM_INTERPOLATION_DATA, LOAD_ANIM_MODEL_BONE_DATA, LOAD_ANIM_MODEL_ROOT_MOTION_BONE, LOAD_ANIM_MODEL_BLEND_BONE, LOAD_END };
    static _bool bLoadOpenMenus[MENU_TYPE::LOAD_END] = { false };

    static _bool bShowDatModal = false;
    static _bool bShowKeyPointData = false;

    // .fbx파일을 로드한다.
    {
        if (ImGui::MenuItem("Load FBX Anim Model", "Ctrl + O"))
            bLoadOpenMenus[LOAD_ANIM_FBX] = true;

        if (ImGui::MenuItem("Load FBX Non Anim Model", "Ctrl + P"))
            bLoadOpenMenus[LOAD_NON_ANIM_FBX] = true;

        if (ImGui::MenuItem("Load XML Anim Interpolation Data", "Ctrl + I"))
            bLoadOpenMenus[LOAD_ANIM_INTERPOLATION_DATA] = true;

        if (ImGui::MenuItem("Load XML Anim Bone", "Ctrl + B"))
            bLoadOpenMenus[LOAD_ANIM_MODEL_BONE_DATA] = true;

        if (ImGui::MenuItem("Load XML Anim Root Motion Bone", "Ctrl + R"))
            bLoadOpenMenus[LOAD_ANIM_MODEL_ROOT_MOTION_BONE] = true;

        if (ImGui::MenuItem("Load XML Anim Blend Bone", "Ctrl + V"))
            bLoadOpenMenus[LOAD_ANIM_MODEL_BLEND_BONE] = true;
    }
   
    {
        // FBX 로드
        if (bLoadOpenMenus[LOAD_ANIM_FBX])
        {
            Open_LoadAnimModal();
            bLoadOpenMenus[LOAD_ANIM_FBX] = false;
        }

        if (bLoadOpenMenus[LOAD_NON_ANIM_FBX])
        {
            // TODO
            bLoadOpenMenus[LOAD_NON_ANIM_FBX] = false;
        }

        // 보간 정보 로드
        if (bLoadOpenMenus[LOAD_ANIM_INTERPOLATION_DATA])
        {
            WCHAR strFilePath[256] = {};
            if (Open_Modal("xml", strFilePath))
                XML_Load_ParseAnimInterpolationData(strFilePath);

            bLoadOpenMenus[LOAD_ANIM_INTERPOLATION_DATA] = false;
        }

        // 뼈 정보 로드
        if (bLoadOpenMenus[LOAD_ANIM_MODEL_BONE_DATA])
        {
            WCHAR strFilePath[256] = {};
            if (Open_Modal("xml", strFilePath))
                XML_Load_BoneHierachy(strFilePath);

            bLoadOpenMenus[LOAD_ANIM_MODEL_BONE_DATA] = false;
        }

        if (bLoadOpenMenus[LOAD_ANIM_MODEL_ROOT_MOTION_BONE])
        {
            WCHAR strFilePath[256];
            if (Open_Modal("xml", strFilePath))
            {
                if (FAILED(XML_Load_RootMotionBoneNameData(strFilePath)))
                    Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_RootMotionBoneNameData", "load fail");
            }

            bLoadOpenMenus[LOAD_ANIM_MODEL_ROOT_MOTION_BONE] = false;
        }
        
        if (bLoadOpenMenus[LOAD_ANIM_MODEL_BLEND_BONE])
        {
            if (FAILED(XML_Load_BlendBones()))
            {
                MSG_BOX("Load BlendBones Failed");
            }
            
            Add_Log(LOG_OK, XML_LOAD, "XML_Load_BlendBones", "load blend bone data");
            bLoadOpenMenus[LOAD_ANIM_MODEL_BLEND_BONE] = false;
        }
    }

    // .dat (바이너리화 된 모델 파일을 로드한다.
    if (ImGui::MenuItem("Load Dat", "Load Bin File"))
    {
        bShowDatModal = 1;
    }

    if (ImGui::MenuItem("Load KeyPointData"))
    {
        bShowKeyPointData = 1;
    }
}

void CLevel_AnimTool::Show_OpenItems_Saves()
{
    // 뼈 구조 XML로 저장
    MenuItem_Save_BoneHierachy();

  
}

HRESULT CLevel_AnimTool::Load_AnimModel_FBX_File(WCHAR* strFBXModelFilePath, ANIM_MODEL_FBX_LOAD_TYPE eFBXLoadType, const _wstring& strPrototypeModelName, const _wstring& strPrototypeGameObjectName)
{
    InitData();

    // PreTransformMatrix : 180도 돌린다.
    _matrix PreTransformMatrix = XMMatrixIdentity();
    PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f)) * XMMatrixTranslation(0.f, 0.f, 0.f);

    // .fbx까지의 경로를 strModelPath에 char로 변환
    const _char* strModelPath = m_pAnimToolUtilObj->WCharToChar(strFBXModelFilePath);

    // 모델 컴포넌트 원형 추가
    {
        string strName = strModelPath;
        fs::path fileName = strFBXModelFilePath;
        fs::path ret = fileName.filename();
        m_strCurFileName = ret.string();

        if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_ANIMTOOL, strPrototypeModelName, CModel::Create(m_pDevice, m_pContext, Engine::MODEL_TYPE::TYPE_ANIM, (strName).c_str(), PreTransformMatrix))))
            return E_FAIL;
    }

    // Link 처럼 메인이 되는 애니매이션 객체
    if (eFBXLoadType == ANIM_MODEL_FBX_LOAD_TYPE::ANIM_MAIN)
    {
        // Loader에서 추가한 원형으로 부터 Clone 생성
        if (FAILED(Load_AnimMain(strPrototypeModelName, TEXT("Prototype_GameObject_AT_Obj"))))
            return E_FAIL;
    }
    else if (eFBXLoadType == ANIM_MODEL_FBX_LOAD_TYPE::ANIM_PART)
    {
        
    }

    delete[] strModelPath;
    strModelPath = nullptr;
    return S_OK;
}

void CLevel_AnimTool::InitData()
{
    // 이전에 생성한 오브젝트가 있다면 삭제한다.
    if (m_pAT_Obj)
    {
        if (m_pGameInstance->Remove_Object(LEVEL_ANIMTOOL, m_pAT_Obj->Get_Name()))
        {
            MSG_BOX("기존 오브젝트 삭제");
        }

        InitTransformData();
        InitAnimationData();



        Clear_LoadBoneInfo(m_RootBone);
        m_bBoneLoadSuccess = false;
        m_SelectedShowBonesNames.clear();
        
        Show_Clear_AllSelected_ShowBones_Btn();
        m_BlendBones.clear();

        m_LogBuffer.clear();
        
        m_iCurMeshIdx = 0;
        m_MeshNames.clear();
        m_strCurMeshName = "";

        m_iModelNum = 0;
        
        m_bKeyDown_PlayDoubleAnimLoopInterpolationTest = false;
        m_bKeyDown_PlayDoubleAnimLoopInterpolationTestWithXML = false;
        
        m_bCtrl = false;



        m_pAT_PartAnimObjs.clear();
        m_pAT_PartAnimObj = nullptr;
        m_pAT_PartAnimTransform = nullptr;
        m_pAT_PartAnimObj_Model = nullptr;
    }
}

void CLevel_AnimTool::InitTransformData()
{
    fill(m_vCurObjPosInput, m_vCurObjPosInput + 3, 0.f);
    fill(m_vRightUpLook, m_vRightUpLook + 3, _vector{});
    fill(m_fCurRotVal, m_fCurRotVal + 3, 0.f);
    fill(m_fPrevRotVal, m_fPrevRotVal + 3, 0.f);
    fill(m_fCurScale, m_fCurScale + 3, 1.f);
}

void CLevel_AnimTool::InitAnimationData()
{
    if (!m_AnimNamesCStr.empty())
        m_AnimNamesCStr.clear();

    if (!m_BlendBones.empty())
        m_BlendBones.clear();
    
    // 애니매이션 플레이 테스트 값 초기화
    m_eAnimPlayTestType = ANIM_PLAY_TEST_TYPE::DEFAULT;
    m_iCurAnimIdx = 0; // 현재 애니매이션 초기화
    m_fTickPerSecond = 1.f;
    m_bIsSrcDstPlayType = false;
    m_strCurFileName = "";
    m_InterpolationTableData.clear();
    m_bSaveInterDataFromTable = false;
    m_iTotInterDataSize = 0;
    m_iCurInterDataSaveCnt = 0;
    m_bShowInterTable = false;
    m_bShowBoneHierachyTable = false;
    m_BonesIndices.clear();
    m_bLoadSuccessBlendBones = false;
}

void CLevel_AnimTool::Cal_RayDir_And_RayStartPos(_int iSX, _int iSY, _vector& vRayDir, _vector& vRayStartPos)
{
    // 뷰, 투영(원근)을 얻어온다.
    _matrix ViewMat = m_pGameInstance->Get_TransformMatrix(CPipeLine::D3DTS_VIEW);
    _matrix ProjMat = m_pGameInstance->Get_TransformMatrix(CPipeLine::D3DTS_PROJ);

    // 마우스 클릭 위치를 NDC로 변환한다.
    _float MouseNDCX = (_float(iSX) / _float(g_iWinSizeX)) * 2.f - 1.f; // -1.f ~ 1.f
    _float MouseNDCY = 1.f - (_float(iSY) / _float(g_iWinSizeY)) * 2.f; // -1.f ~ 1.f

    // 뷰, 투영(원근) 행렬의 역행렬 계산
    _matrix InvView = XMMatrixInverse(nullptr, ViewMat);
    _matrix InvProj = XMMatrixInverse(nullptr, ProjMat);

    // _matrix InvVP = InvProj * InvView; // 역행렬이기 때문에 반대로 곱한다.

    // NDC를 월드 공간으로 변환
    // NDC상의 가장 가까운 곳과 먼곳의 좌표를 잡아준다.
    _vector vNearPos = XMVectorSet(MouseNDCX, MouseNDCY, 0.f, 1.f);
    _vector vFarPos = XMVectorSet(MouseNDCX, MouseNDCY, 1.f, 1.f);

    // NDC상의 가장 가까운점과 먼 점에 투영의 역행렬 -> 뷰 의 역행렬을 곱해 월드 상의 위치를 계산한다.
    // XMVector3TransformCoord는 Z 나누기를 수행한다.
    _vector vViewNearPos = XMVector3TransformCoord(vNearPos, InvProj);
    _vector vViewFarPos = XMVector3TransformCoord(vFarPos, InvProj);

    _vector vWorldNearPos = XMVector3TransformCoord(vViewNearPos, InvView);
    _vector vWorldFarPos = XMVector3TransformCoord(vViewFarPos, InvView);

    // 뷰 행렬의 역행렬의 3행 성분은 카메라의 월드 위치이다. 이것을 레이의 시작점으로 설정해준다.
    _vector vRayNewStartPos = InvView.r[3];
    XMVectorSetW(vRayStartPos, 1.f);

    _float4 vPos{};
    XMStoreFloat4(&vPos, vRayStartPos);

    // Ray방향을 구해준다.
    _vector vRayDiriection = XMVector3Normalize(XMVectorSubtract(vWorldFarPos, vWorldNearPos));

    vRayDir = vRayDiriection;
    vRayStartPos = vRayNewStartPos;
}

POINT CLevel_AnimTool::Get_Mouse_ScreenPos()
{
    return m_pGameInstance->Get_MousePos();
}

void CLevel_AnimTool::Show_IK_Options()
{
    static const _char* pIKOptions[] = { "IK Options", "Controll Update Exclude Bone", "Controll IK Legs" };
    static _int iSelectedIKOptionIdx = 0;
    const _char* pComboSelectIkOptionName = pIKOptions[iSelectedIKOptionIdx];

    if (ImGui::BeginCombo("IK Options", pComboSelectIkOptionName))
    {
        for (int n = 0; n < IM_ARRAYSIZE(pIKOptions); n++)
        {
            const bool is_selected = (iSelectedIKOptionIdx == n);
            if (ImGui::Selectable(pIKOptions[n], is_selected))
            {
                iSelectedIKOptionIdx = n;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    // 애니매이션 업데이트에서 제외한 뼈 컨트롤 하는 옵션
    if (iSelectedIKOptionIdx == 1)
    {
        Show_IK_Controll_UpdateExcludeBone();
    }
    // 팔 컨트롤 IK 옵션
    else if (iSelectedIKOptionIdx == 2)
    {
        // TODO
        Show_IK_Controll_Legs();
    }

    
}

void CLevel_AnimTool::Show_IK_Controll_UpdateExcludeBone()
{
    static const _char* pControllUpdateExcludeBoneOptions[] = { "Options", "Rotate to mouse world pos", "Rotate to mouse screen pos" };
    static _int iSelectedControllUpdateExcludeBoneOptionIdx = 0;
    const _char* pComboCurControllOption = pControllUpdateExcludeBoneOptions[iSelectedControllUpdateExcludeBoneOptionIdx];

    if (ImGui::BeginCombo("Controll Update Exclude Bone Option", pComboCurControllOption))
    {
        for (int n = 0; n < IM_ARRAYSIZE(pControllUpdateExcludeBoneOptions); n++)
        {
            const bool is_selected = (iSelectedControllUpdateExcludeBoneOptionIdx == n);
            if (ImGui::Selectable(pControllUpdateExcludeBoneOptions[n], is_selected))
            {
                iSelectedControllUpdateExcludeBoneOptionIdx = n;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // 마우스 월드 위치를 따라가는 경우
    if (iSelectedControllUpdateExcludeBoneOptionIdx == 1)
    {
        // 월드상에 존재하는 마우스를 따라 회전하는 옵션
        Show_IK_Controll_UpdateExcludeBone_FollowMouseInWorld();
    }



    
}

void CLevel_AnimTool::Show_IK_Controll_UpdateExcludeBone_FollowMouseInWorld()
{
    static _bool bOnceSpawnMouse = false;
    static _bool bActiveMouse = false;

    if (ImGui::Button("Spawn Mouse In World"))
    {
        if (!bOnceSpawnMouse)
        {
            bOnceSpawnMouse = true;

            if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_ANIMTOOL, L"Prototype_GameObject_AT_Mouse", LEVEL_ANIMTOOL, L"AT_Mouse", nullptr, L"Mouse")))
                return;

            auto pMouse = m_pGameInstance->Find_Object_By_Name(LEVEL_ANIMTOOL, L"AT_Mouse", L"Mouse");
            if (!pMouse)
                return;

            m_pMouse = static_cast<CAT_Mouse*>(pMouse);
        }
    }

    ImGui::Checkbox("Rotate Follow Mouse", &bActiveMouse);

    static _float fDis = 5.f;
    if (bActiveMouse)
    {
        if (ImGui::InputFloat("Set distance camera to mouse", &fDis))
        {
            
        }

        POINT vScreenPos = Get_Mouse_ScreenPos();
        _int iSX = vScreenPos.x;
        _int iSY = vScreenPos.y;
        _vector vRayStartPos{}, vRayDir{};
        Cal_RayDir_And_RayStartPos(iSX, iSY, vRayDir, vRayStartPos);


        _vector vMouseWorldPos = vRayStartPos + vRayDir * fDis;

        if (m_pMouse)
            m_pMouse->Get_Transform()->Set_State(CTransform::STATE_POSITION, vMouseWorldPos);

        Rotate_Update_Exclude_Bone_ToMouseWorldPos();
    }
}

void CLevel_AnimTool::Rotate_Update_Exclude_Bone_ToMouseWorldPos()
{
    static _bool bOnce = false;
    if (!bOnce)
    {
        // m_pAT_PartAnimObj_Model
    }

#pragma region "루트 본의 위치를 기준으로 계산 (원점에서 회전이 없는 경우 문제 없음)"

    // Rotate_Test();

#ifdef _DEBUG

    //static _float fAcc = 0.f;
    //fAcc += m_pGameInstance->Get_TimeDelta(L"Timer_60");
    //if (fAcc >= 2.5f)
    //{
    //    fAcc = 0.f;

    //    printf("\n");
    //    printf("==========================================================\n");
    //    printf("\n");

    //    printf("\n");
    //    printf("%-30s : %10.2f\n", "두 벡터 내적의 값", fDot);
    //    printf("\n");


    //    // 목 뼈
    //    {
    //        printf("---------- 목뼈 ----------\n");

    //        _vector vBoneWorldPos = XMVector3TransformCoord(vBonePos_InRootLocal, PlayerWorldMatrix);

    //        printf("%-30s : %10.2f, %10.2f, %10.2f\n", "루트 위치", XMVectorGetX(vBoneLocalPos), XMVectorGetY(vBoneLocalPos), XMVectorGetZ(vBoneLocalPos));
    //        printf("%-30s : %10.2f, %10.2f, %10.2f\n", "루트 뼈를 기준 위치", XMVectorGetX(vBonePos_InRootLocal), XMVectorGetY(vBonePos_InRootLocal), XMVectorGetZ(vBonePos_InRootLocal));
    //        printf("%-30s : %10.2f, %10.2f, %10.2f\n", "월드 기준 위치", XMVectorGetX(vBoneWorldPos), XMVectorGetY(vBoneWorldPos), XMVectorGetZ(vBoneWorldPos));

    //        printf("---------- 목뼈 ----------\n");
    //    }

    //    printf("\n");

    //    // 타겟 (마우스)
    //    {
    //        printf("---------- 타겟 ----------\n");

    //        _vector vTargetWorldPos = MouseWorldMatrix.r[3];
    //        _vector vTargetWorldPosToPlayerLocalPos = XMVector3TransformCoord(vTargetWorldPos, PlayerWorldMatrixInv);
    //        _vector vTargetPlayerLocalPosToRootBonePos = XMVector3TransformCoord(vTargetWorldPosToPlayerLocalPos, BoneParentCombinedMatrix);

    //        printf("%-30s : %10.2f, %10.2f, %10.2f\n", "플레이어 로컬 기준 위치", XMVectorGetX(vTargetWorldPosToPlayerLocalPos), XMVectorGetY(vTargetWorldPosToPlayerLocalPos), XMVectorGetZ(vTargetWorldPosToPlayerLocalPos));
    //        printf("%-30s : %10.2f, %10.2f, %10.2f\n", "루트 뼈를 기준 위치", XMVectorGetX(vTargetPlayerLocalPosToRootBonePos), XMVectorGetY(vTargetPlayerLocalPosToRootBonePos), XMVectorGetZ(vTargetPlayerLocalPosToRootBonePos));
    //        printf("%-30s : %10.2f, %10.2f, %10.2f\n", "월드 기준 위치", XMVectorGetX(vTargetWorldPos), XMVectorGetY(vTargetWorldPos), XMVectorGetZ(vTargetWorldPos));

    //        printf("---------- 타겟 ----------\n");
    //    }

    //    printf("\n");


    //    printf("\n");
    //    printf("==========================================================\n");
    //    printf("\n");

    //}



#endif
  

#pragma endregion   

    Rotate_Test_Client();
    // Rotate_Test_Engine();
}

void CLevel_AnimTool::Rotate_Test_Client()
{
    // 애니메이션 업데이트에서 제외된 뼈의 이름을 가져옴
    string strUpdateExcludeBoneName = m_pAT_PartAnimObj_Model->Get_UpdateExcludeBoneName();

    // 캐릭터 월드 변환 행렬과 그 역행렬
    _matrix SrcMatrix = Get_PlayerWorld_Matrix();
    _matrix SrcMatrixInv = XMMatrixInverse(nullptr, SrcMatrix);

    // 대상 뼈(목뼈)의 로컬 변환 행렬을 가져옴
    _matrix UEBLocalMatrix = Get_UpdateExcludeBone_TransformationMatrix(strUpdateExcludeBoneName);
    _vector UEBLocalPos = UEBLocalMatrix.r[3];      // 로컬 위치
    _vector UEBLocalLookDir = XMVector3Normalize(UEBLocalMatrix.r[2]);  // 로컬 Look 벡터

    // 부모 뼈의 최종 변환 행렬 (월드 변환 포함)
    _matrix BoneParentCombinedMatrix = Get_Parent_CombinedTransformationMatrix_From_UpdateExcludeBone();
    _matrix BoneParentCombinedMatrixInv = XMMatrixInverse(nullptr, BoneParentCombinedMatrix);


    // 마우스의 월드 위치를 가져옴
    _matrix TargetWorldMatrix = Get_MouseWorld_Matrix();

    // 타겟의 월드 좌표를 부모 공간으로 변환
    _matrix NewTargetMatrix = TargetWorldMatrix * SrcMatrixInv * BoneParentCombinedMatrix;
    _vector NewTargetPos = NewTargetMatrix.r[3];

    // 현재 뼈의 Look 벡터와 위치를 부모 공간으로 변환
    _vector vBoneLookDir_InRootLocal = XMVector3TransformNormal(UEBLocalLookDir, BoneParentCombinedMatrix);
    _vector vBonePos_InRootLocal = XMVector3TransformCoord(UEBLocalPos, BoneParentCombinedMatrix);

    // 부모 공간에서 뼈가 바라볼 방향
    _vector vBoneToTargetDir_InRootLocal = XMVector3Normalize(NewTargetPos - vBonePos_InRootLocal);

    // 회전해야 하는 각도 계산
    _float fDot = XMVectorGetX(XMVector3Dot(vBoneToTargetDir_InRootLocal, vBoneLookDir_InRootLocal));
    fDot = max(-1.f, min(1.f, fDot));  // acosf 범위 보정

    // 회전 축 계산 (외적)
    _vector vRotAxis = XMVector3Cross(vBoneLookDir_InRootLocal, vBoneToTargetDir_InRootLocal);
    _float axisLengthSq = XMVectorGetX(XMVector3LengthSq(vRotAxis));

    // Gimbal Lock 방지
    if (axisLengthSq < 0.0001f)
    {
        vRotAxis = XMVector3Normalize(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
        if (fabs(XMVectorGetX(XMVector3Dot(vRotAxis, vBoneLookDir_InRootLocal))) > 0.99f)
            vRotAxis = XMVector3Normalize(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    }
    else
    {
        vRotAxis = XMVector3Normalize(vRotAxis);
    }

    // 회전 행렬 생성
    _float fAngle = acosf(fDot);
    _vector QuaternionRot = XMQuaternionRotationAxis(vRotAxis, fAngle);
    _matrix RotMatrix = XMMatrixRotationQuaternion(QuaternionRot);

    // 부모 공간에서 회전 행렬을 적용
    _matrix RotMatrix_Local = BoneParentCombinedMatrixInv * RotMatrix * BoneParentCombinedMatrix;

    // 새로운 로컬 행렬 적용
    UEBLocalMatrix = UEBLocalMatrix * RotMatrix_Local;
    UEBLocalMatrix.r[3] = XMVectorSetW(UEBLocalPos, 1.0f);

    m_pAT_PartAnimObj_Model->Set_UpdateExcludeBone_TransformationMatrix(UEBLocalMatrix);
}

void CLevel_AnimTool::Rotate_Test_Engine()
{
    _matrix PlayerWorldMatrix = Get_PlayerWorld_Matrix();
    _matrix MouseWorldMatrix = Get_MouseWorld_Matrix();

    m_pAT_PartAnimObj_Model->Rotate_UpdateExcludeBone_To_Target(m_pAT_PartAnimObj_Model->Get_UpdateExcludeBoneName(), PlayerWorldMatrix, MouseWorldMatrix);
}

_fmatrix CLevel_AnimTool::Get_MouseWorld_Matrix()
{
    if (!m_pMouse)
        return XMMatrixIdentity();
    return m_pMouse->Get_Transform()->Get_WorldMatrix();
}

_fmatrix CLevel_AnimTool::Get_PlayerWorld_Matrix()
{
    if (!m_pAT_Obj)
        return XMMatrixIdentity();
    return m_pAT_Obj->Get_Transform()->Get_WorldMatrix();
}

_fmatrix CLevel_AnimTool::Get_UpdateExcludeBone_TransformationMatrix(string strUpdateExcludeBoneName)
{
    auto iter = m_BoneOriginTransformationMatrices.find(strUpdateExcludeBoneName);
    if (iter != m_BoneOriginTransformationMatrices.end())
        return XMLoadFloat4x4(&m_BoneOriginTransformationMatrices[strUpdateExcludeBoneName]);
    return XMMatrixIdentity();
}

_fmatrix CLevel_AnimTool::Get_Bone_CombinedTransformationMatrix(string strBoneName)
{
    if (m_pAT_PartAnimObj_Model)
        return  m_pAT_PartAnimObj_Model->Get_Bone_CombiendTransformationMatrix(strBoneName);
    return XMMatrixIdentity();
}

void CLevel_AnimTool::Show_IK_Controll_Legs()
{
    if (!m_pST)
    {
        m_pST = static_cast<CAT_Stair*>(m_pGameInstance->Find_Object_By_Name(LEVEL_ANIMTOOL, L"Layer_AnimToolStair", L"Stair"));
        if (!m_pST)
        {
            MSG_BOX("Stair is nullptr");
            return;
        }

        m_pSTTransformCom = m_pST->Get_Transform();
        m_pSTModelCom = static_cast<CModel*>(m_pST->Find_Component(L"Com_Model"));
        
        if (!m_pSTModelCom)
        {
            MSG_BOX("Model Com is nullptr");
            return;
        }

    }
}

void CLevel_AnimTool::Show_IK_Ray(string strBoneName, CAT_Ray* pRay, BONE_IK_LOOK_DIR_TYPE eIKLookType)
{
    if (m_pAT_Obj)
    {
        switch (eIKLookType)
        {
        case Client::CLevel_AnimTool::IK_PLAYER_LOOK:
        {
            _matrix CObjWorldMatrix = m_pAT_Obj->Get_Transform()->Get_WorldMatrix();
            _vector vBoneWorldPos = Get_Bone_WorldPos(CObjWorldMatrix, strBoneName);

            _vector vLook = XMVector3Normalize(CObjWorldMatrix.r[2]);
            _vector vTempUp = { 0.f, 1.f, 0.f, 0.f };
            _vector vRight = XMVector3Normalize(XMVector3Cross(vTempUp, vLook));
            _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

            pRay->Get_Transform()->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook));
            pRay->Get_Transform()->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight));
            pRay->Get_Transform()->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp));

            pRay->Get_Transform()->Scaling(1.f, 1.f, 1.f);

            vBoneWorldPos = vBoneWorldPos + vLook / 2.f;
            pRay->Get_Transform()->Set_State(CTransform::STATE_POSITION, vBoneWorldPos);
        }
            break;
        case Client::CLevel_AnimTool::IK_BONE_WORLD_LOOK:
        {
            _matrix CObjWorldMatrix = m_pAT_Obj->Get_Transform()->Get_WorldMatrix();
            _vector vBoneWorldLook = Get_Bone_WorldLook(CObjWorldMatrix, strBoneName);
            _vector vBoneWorldPos = Get_Bone_WorldPos(CObjWorldMatrix, strBoneName);

            _vector vLook = vBoneWorldLook;
            _vector vTempUp = { 0.f, 1.f, 0.f, 0.f };
            _vector vRight = XMVector3Normalize(XMVector3Cross(vTempUp, vLook));
            _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

            pRay->Get_Transform()->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook));
            pRay->Get_Transform()->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight));
            pRay->Get_Transform()->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp));

            pRay->Get_Transform()->Scaling(1.f, 1.f, 1.f);

            vBoneWorldPos = vBoneWorldPos + vLook / 2.f;
            pRay->Get_Transform()->Set_State(CTransform::STATE_POSITION, vBoneWorldPos);
        }
            break;
        case Client::CLevel_AnimTool::IK_BONE_LOCAL_LOOK:
        {
            _matrix CObjWorldMatrix = m_pAT_Obj->Get_Transform()->Get_WorldMatrix();

            _matrix BoneTransformationMatrix = XMLoadFloat4x4(&(*m_pAT_PartAnimObj_Model->Get_Bone_TransformationMatrix_Ptr(strBoneName)));
            _vector vBoneWorldPos = Get_Bone_WorldPos(CObjWorldMatrix, strBoneName);

            _vector vLook = XMVector3Normalize(BoneTransformationMatrix.r[2]);
            _vector vTempUp = { 0.f, 1.f, 0.f, 0.f };
            _vector vRight = XMVector3Normalize(XMVector3Cross(vTempUp, vLook));
            _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

            pRay->Get_Transform()->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook));
            pRay->Get_Transform()->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight));
            pRay->Get_Transform()->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp));

            pRay->Get_Transform()->Scaling(1.f, 1.f, 1.f);

            vBoneWorldPos = vBoneWorldPos + vLook / 2.f;
            pRay->Get_Transform()->Set_State(CTransform::STATE_POSITION, vBoneWorldPos);
        }
        break;
        case Client::CLevel_AnimTool::IK_WORLD_TO_TARGET:
        {
            if (!m_pMouse)
                return;

            // TransformationMatrix와 BoneUpdateExcludeTransformationMatrix는 같다
            {
                //_matrix BoneUpdateExcludeTransformationMatrix = XMLoadFloat4x4(&m_pAT_PartAnimObj_Model->Get_UpdateExcludeBone_TransformationMatrix());
                //_vector vBoneUpdateExcludeTransformationLookDir = XMVector3Normalize(BoneUpdateExcludeTransformationMatrix.r[2]);

                //_matrix CObjWorldMatrix = m_pAT_Obj->Get_Transform()->Get_WorldMatrix();




                //_vector vBoneWorldPos = Get_Bone_WorldPos(CObjWorldMatrix, strBoneName);

                //_vector vLook = vBoneUpdateExcludeTransformationLookDir;
                //_vector vTempUp = { 0.f, 1.f, 0.f, 0.f };
                //_vector vRight = XMVector3Normalize(XMVector3Cross(vTempUp, vLook));
                //_vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

                //pRay->Get_Transform()->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook));
                //pRay->Get_Transform()->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight));
                //pRay->Get_Transform()->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp));

                //pRay->Get_Transform()->Scaling(1.f, 1.f, 1.f);

                //vBoneWorldPos = vBoneWorldPos + vLook / 2.f;
                //pRay->Get_Transform()->Set_State(CTransform::STATE_POSITION, vBoneWorldPos);
            }


            // 위치 - 위치 로 방향 벡터 구한 경우
            {
                _matrix BoneTransformationMatrix = XMLoadFloat4x4(&(*m_pAT_PartAnimObj_Model->Get_Bone_TransformationMatrix_Ptr(strBoneName)));
                _matrix BoneCombinedMatrix = m_pAT_PartAnimObj_Model->Get_Bone_CombiendTransformationMatrix(strBoneName);
                _matrix CObjWorldMatrix = m_pAT_Obj->Get_Transform()->Get_WorldMatrix();

                _matrix BoneWorldMatrix = BoneCombinedMatrix * CObjWorldMatrix;
                _vector vBoneWorldPos = BoneWorldMatrix.r[3];

                _vector vTargetPos = m_pMouse->Get_Transform()->Get_State(CTransform::STATE_POSITION);
                _vector vBoneToTargetDir = XMVector3Normalize(vTargetPos - vBoneWorldPos);

                _vector vLook = vBoneToTargetDir;
                _vector vTempUp = { 0.f, 1.f, 0.f, 0.f };
                _vector vRight = XMVector3Normalize(XMVector3Cross(vTempUp, vLook));
                _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

                pRay->Get_Transform()->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook));
                pRay->Get_Transform()->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight));
                pRay->Get_Transform()->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp));

                pRay->Get_Transform()->Scaling(1.f, 1.f, 1.f);

                vBoneWorldPos = vBoneWorldPos + vLook / 2.f;
                pRay->Get_Transform()->Set_State(CTransform::STATE_POSITION, vBoneWorldPos);
            }

        }
        break;
        case Client::CLevel_AnimTool::IK_LOOK_END:
            break;
        default:
            break;
        }


        
    }
}

_vector CLevel_AnimTool::Get_Bone_WorldPos(_fmatrix ContainerObjectWorldMatrix, string strBoneName)
{
    _matrix BoneCombinedMatrix = m_pAT_PartAnimObj_Model->Get_Bone_CombiendTransformationMatrix(strBoneName);
    _matrix CObjWorldMatrix = m_pAT_Obj->Get_Transform()->Get_WorldMatrix();

    _matrix BoneWorldMatrix = BoneCombinedMatrix * CObjWorldMatrix;
    return BoneWorldMatrix.r[3];
}

_vector CLevel_AnimTool::Get_Bone_WorldLook(_fmatrix ContainerObjectWorldMatrix, string strBoneName)
{
    _matrix BoneCombinedMatrix = m_pAT_PartAnimObj_Model->Get_Bone_CombiendTransformationMatrix(strBoneName);
    _matrix CObjWorldMatrix = m_pAT_Obj->Get_Transform()->Get_WorldMatrix();

    _matrix BoneWorldMatrix = BoneCombinedMatrix * CObjWorldMatrix;
    return XMVector3Normalize(BoneWorldMatrix.r[2]);
}

_fmatrix CLevel_AnimTool::Get_Parent_CombinedTransformationMatrix_From_UpdateExcludeBone()
{
    string strUpdateExcldueBoneName = m_pAT_PartAnimObj_Model->Get_UpdateExcludeBoneName();
    if (strUpdateExcldueBoneName == "None")
        return XMMatrixIdentity();
    
    if (!m_pAT_PartAnimObj_Model)
        return XMMatrixIdentity();

    return m_pAT_PartAnimObj_Model->Get_Parent_CombinedTransformationMatrix_From_UpdateExlcudeBone();
}

HRESULT CLevel_AnimTool::Load_AnimMain(_wstring strProtoModelName, _wstring strGameObjectCloneName)
{
    InitData();

    const static _wstring strLayerTag = L"Layer_AT_Objs";
    static _int iNum = 0;

    CAT_Obj::AT_OBJ_DESC Desc = {};
    Desc.fSpeedPerSec = 1.f;
    Desc.fRotationPerSec = 1.f;
    Desc.strModelProtoName = strProtoModelName;
    Desc.pParentMatrix = nullptr;

    _wstring strObjTag = L"AnimObject" + to_wstring(iNum++);

    // 모델 생성
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_ANIMTOOL, strGameObjectCloneName, LEVEL_ANIMTOOL, strLayerTag, &Desc, strObjTag)))
        return E_FAIL;

    // 생성한 오브젝트를 찾는다.
    auto pObj = m_pGameInstance->Find_Object_By_Name(LEVEL_ANIMTOOL, strLayerTag, strObjTag);
    if (pObj)
    {
        m_pAT_Obj = static_cast<CAT_Obj*>(pObj);
        auto pPartAnimObj = static_cast<CAT_PartAnimObj*>(m_pAT_Obj->Find_Child(m_pAT_Obj->Get_ChildSize() - 1));
        if (pPartAnimObj)
        {
            // PartAimObjs에 추가하고 현재 m_pAT_PartAnimObj로 설정해준다.
            m_pAT_PartAnimObjs.push_back(pPartAnimObj);
            m_pAT_PartAnimObj = m_pAT_PartAnimObjs.back();
        }
        else
        {
            MSG_BOX("CLevel_AT::Load_AnimMain CAT_PartAnimObj nullptr!!!");
            Add_Log(LOG_FAIL, XML_LOAD, "Load_AnimMain", "CAT_PartAnimObj nullptr");
            return E_FAIL;
        }

        m_pAT_PartAnimTransform = m_pAT_PartAnimObj->Get_Transform();
        if (!m_pAT_PartAnimTransform)
        {
            Add_Log(LOG_FAIL, XML_LOAD, "Load_AnimMain", "m_pAT_PartAnimTransform nullptr");
            return E_FAIL;
        }

        // 현재 PartAnimObj의 애니매이션 인덱스 가져오기
        m_iCurAnimIdx = m_pAT_PartAnimObj->Get_CurAnimIdx();

        m_pAT_PartAnimObj_Model = static_cast<CModel*>(m_pAT_PartAnimObj->Find_Component(TEXT("Com_Model")));

        // m_pAT_PartAnimObj_Model->G
        m_pAT_PartAnimObj_Model->Get_AIScene();

        const vector<string>& AnimNames = m_pAT_PartAnimObj_Model->Get_AnimNames();
        for (const auto& name : AnimNames)
        {
            m_AnimNamesCStr.push_back(name.c_str());
        }
    }
    else
    {
        MSG_BOX("CLevel_AnimTool::Load_AnimMain 생성한 오브젝트를 찾을 수 없습니다");
        return E_FAIL;
    }

    string str = "create ok | " + CONVERTTOSTRING(strGameObjectCloneName);
    Add_Log(LOG_OK, XML_LOAD, "Load_AnimMain", str);
    return S_OK;
}

HRESULT CLevel_AnimTool::Load_NonAnimModel_FBX_File(WCHAR* strFBXModelFilePath)
{
    return S_OK;
}

void CLevel_AnimTool::MultiControll_Transform_TreeNode()
{
    if (ImGui::TreeNode("Multi-Controll-Transform"))
    {
        auto pCom = m_pAT_Obj->Get_Transform();
        if (pCom)
        {
            m_bMultiControllsTransform[AT_MC_TRANSFORM_TYPE::POSITION] = Controll_WorldPos(pCom);
            m_bMultiControllsTransform[AT_MC_TRANSFORM_TYPE::ROTATION] = Controll_Rotation(pCom);
            m_bMultiControllsTransform[AT_MC_TRANSFORM_TYPE::SCALE] = Controll_Scale(pCom);
        }
        ImGui::TreePop();
    }
}

void CLevel_AnimTool::MultiControll_AnimModel_Animation_TreeNode()
{
    TestGizmo();

    // 트리노드 누르면 떠진다.
    if (ImGui::TreeNode("Multi-Controll-Animations"))
    {
        // 애니매이션 정보 출력
        m_bMultiControllsAnimation[AT_MC_ANIMATION_TYPE::ANIMINFO_TEXT] = Show_AnimInfo_Text(m_iCurAnimIdx);

        // 루트 애니매이션 플레이 타입 설정
        m_bMultiControllsAnimation[AT_MC_ANIMATION_TYPE::ANIM_ROOT_MOTION_PLAY] = Show_AnimRootMotionType_RadioBtn();

        // 애니매이션 자체 재생 타입
        m_bMultiControllsAnimation[AT_MC_ANIMATION_TYPE::ANIM_PLAY_TYPE] = Show_AnimPlayType_ComboBox();

        // 보간 정보 버튼 띄우기
        m_bMultiControllsAnimation[AT_MC_ANIMATION_TYPE::ANIM_INTERPOLATION] = Show_InterpolationTable();

        // 애니매이션 키 프레임 정보 띄우기
        m_bMultiControllsAnimation[AT_MC_ANIMATION_TYPE::ANIM_KEYFRAME] = Show_Anim_KeyFrames();

        Begin_Animation_KeyFrame();


        ImGui::TreePop();
    }
}

void CLevel_AnimTool::MultiControll_Bone()
{
    // static _bool bOpend = false;

    if (ImGui::TreeNode("Multi-Controll-Bone"))
    {
        // 뼈 전체 정보 로드하는 버튼
        m_bMultiControllsBone[AT_MC_BONE_TYPE::LOAD_BONE_DATA_BTN] = Show_LoadBoneData_Btn();
        if (!m_bMultiControllsBone[AT_MC_BONE_TYPE::LOAD_BONE_DATA_BTN])
        {
            MSG_BOX("Load Bone Data Failed");
            m_bMultiControllsBone[AT_MC_BONE_TYPE::LOAD_BONE_DATA_BTN] = true;
        }

        // 루트모션에 사용할 뼈 이름 로드하는 버튼
        m_bMultiControllsBone[AT_MC_BONE_TYPE::LOAD_ROOTMOTION_BONENAME_DATA] = Show_Load_RootMotionBoneName_Btn();
        if (!m_bMultiControllsBone[AT_MC_BONE_TYPE::LOAD_ROOTMOTION_BONENAME_DATA])
        {
            MSG_BOX("Load Root Motion Bone Data Failed");
            m_bMultiControllsBone[AT_MC_BONE_TYPE::LOAD_ROOTMOTION_BONENAME_DATA] = true;
        }

        if (ImGui::Button("Load Blend Bones"))
        {
            if (FAILED(XML_Load_BlendBones()))
            {
                MSG_BOX("Load BlendBones Failed");
            }
        }

        ImGui::TreePop();
    }

    // 전체 뼈구조 xml파일 로드하는데 성공 했다면
    if (m_bBoneLoadSuccess)
    {
        // 전체 뼈구조 렌더하는 함수
        m_bMultiControllsBone[AT_MC_BONE_TYPE::BONE_HIERARCHY] = Begin_Show_BoneHierarchy();
        if (!m_bMultiControllsBone[AT_MC_BONE_TYPE::BONE_HIERARCHY])
        {
            MSG_BOX("AT_MC_BONE_TYPE::BONE_HIERARCHY Error");
            m_bMultiControllsBone[AT_MC_BONE_TYPE::BONE_HIERARCHY] = true;
        }

        // 보여줄 뼈 선택해서 렌더링 하는 함수
        m_bMultiControllsBone[AT_MC_BONE_TYPE::SHOW_SELECTED_BONE_WORLD] = Show_BoneWorldPos();
        if (!m_bMultiControllsBone[AT_MC_BONE_TYPE::SHOW_SELECTED_BONE_WORLD])
        {
            MSG_BOX("AT_MC_BONE_TYPE::SHOW_SELECTED_BONE_WORLD Error");
            m_bMultiControllsBone[AT_MC_BONE_TYPE::SHOW_SELECTED_BONE_WORLD] = true;
        }
    }
}

void DrawColoredIconsWithText()
{
    // 각 아이콘의 색상과 설명
    struct IconInfo 
    {
        ImVec4 color;    // RGBA 색상
        const char* text; // 설명 텍스트
    };

    IconInfo icons[] = 
    {
        { ImVec4(1.0f, 0.0f, 0.0f, 1.0f), " : Current Active Animation" },
        { ImVec4(1.0f, 1.0f, 0.0f, 1.0f), " : Interpolationing Animation" },
        { ImVec4(0.0f, 0.0f, 1.0f, 1.0f), " : Interpolation Target Animation" }
    };

    // 아이콘 크기
    const float iconSize = 20.0f;

    for (int i = 0; i < 3; ++i)
    {
        // 현재 커서 위치를 가져옴
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();

        // 아이콘을 그리기 위한 DrawList 가져오기
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 네모 그리기
        drawList->AddRectFilled(
            ImVec2(cursorPos.x, cursorPos.y),                       // 네모의 왼쪽 위 좌표
            ImVec2(cursorPos.x + iconSize, cursorPos.y + iconSize), // 네모의 오른쪽 아래 좌표
            ImColor(icons[i].color)                                // 색상
        );

        // 네모 오른쪽으로 텍스트 출력
        ImGui::Dummy(ImVec2(iconSize, iconSize)); // 네모의 크기만큼 공간 확보
        ImGui::SameLine();
        ImGui::Text("%s", icons[i].text);
    }
}

_bool CLevel_AnimTool::Show_AnimInfo_Text(_int iCurAnimIdx)
{
    // 애니매이션 블렌딩 중이 아닌 경우
    if (!m_pAT_PartAnimObj->m_bIsPlayAnimBlend)
    {
        // 현재 애니매이션 정보를 띄운다.
        // - 현재 모델 애니매이션 전체 개수, 현재 애니매이션 인덱스, 현재 애니매이션 이름
        // - duration, trackpos, tickpersecond
        // 현재 재생중인 애니매이션 개수, 애니매이션 인덱스, 애니매이션 이름을 띄운다.
        ImGui::Text("Max Anim Cnt : %d,  Cur Anim Idx : %d,  Cur Anim Name : %s", m_AnimNamesCStr.size(), m_iCurAnimIdx, m_AnimNamesCStr[m_iCurAnimIdx]);

        auto pCurAnim = m_pAT_PartAnimObj_Model->Get_CurAnim();
        ImGui::Text("Cur Anim Duration : %f,  TrackPos : %f,  TickPerSecond : %f", pCurAnim->Get_AnimDuration(), pCurAnim->Get_CurTrackPos(), pCurAnim->Get_TickPerSecond());

        static _float fInterDis = m_pAT_PartAnimObj->Get_InterDis();
        static _float fInterSP = m_pAT_PartAnimObj->Get_InterSP();

        if (m_eAnimPlayTestType == ANIM_PLAY_TEST_TYPE::LOOP_INTERPOLATION)
        {
            _float fInterDis = m_pAT_PartAnimObj_Model->Get_InterDis();
            _float fInterSP = m_pAT_PartAnimObj_Model->Get_InterSP();
            _float fInterTrackPos = m_pAT_PartAnimObj_Model->Get_CurInterTrackPos();
            ImGui::Text("Cur InterDis : %f,  InterSP : %f,  InterTrackPos : %f", fInterDis, fInterSP, fInterTrackPos);
        }
    }
    // 애니매이션 블렌딩인 경우
    else
    {

        DrawColoredIconsWithText();

        const static _int iLower = (_int)ANIM_BLEND_GROUP_TYPE::LOWER_BODY;
        const static _int iTotal = (_int)ANIM_BLEND_GROUP_TYPE::TOTAL;
        const static string strBlendBoneTypes[(_int)ANIM_BLEND_GROUP_TYPE::BLEND_END] = {"None", "LowerBody", "UpperBody" , "ArmLeft" , "ArmRight" , "Head" , "Total" };

        enum BLEND_GROUP_STATE { BGS_DEACTIVE, BGS_ACTIVE, BGS_INTERPOLATION, BGS_INTERPOLATION_TARGET };
        static BLEND_GROUP_STATE strBlendGroupTarget[(_int)ANIM_BLEND_GROUP_TYPE::BLEND_END] = { BGS_DEACTIVE, };

        for (_int iCurGroupIdx = iLower; iCurGroupIdx <= iTotal; ++iCurGroupIdx)
        {
            CModel::BLEND_GROUP* pBlendGroup = m_pAT_PartAnimObj_Model->Get_BlendGroups();
            
            // 보간 타겟이 되는 녀석들 노란색으로 칠한다.
            for (_int i = iLower; i <= iTotal; ++i)
            {
                if (!pBlendGroup[i].AnimInfo.bUse && pBlendGroup[i].AnimInfo.bInterpolationing)
                {
                    for (_int j = iLower; j <= iTotal; ++j)
                        if (pBlendGroup[i].InterpolationInfo.InterpolationDstGroupIdices[j] != -1)
                            strBlendGroupTarget[j] = BGS_INTERPOLATION_TARGET;
                }
            }

            if (pBlendGroup)
            {
                _int iCurAnimIdx = -1;
                _float fDuration = 0.f, fTrackPos = 0.f, fTickPerSecond = 0.f;
                string strAnimName = "None";
                if (pBlendGroup[iCurGroupIdx].AnimInfo.bUse)
                {
                    if (pBlendGroup[iCurGroupIdx].AnimInfo.pAnim)
                    {
                        strAnimName = pBlendGroup[iCurGroupIdx].AnimInfo.strAnimName;
                        iCurAnimIdx = pBlendGroup[iCurGroupIdx].AnimInfo.iAnimIdx;
                        fDuration = pBlendGroup[iCurGroupIdx].AnimInfo.pAnim->Get_AnimDuration();
                        fTrackPos = pBlendGroup[iCurGroupIdx].AnimInfo.pAnim->Get_CurTrackPos();
                        fTickPerSecond = pBlendGroup[iCurGroupIdx].AnimInfo.pAnim->Get_TickPerSecond();
                    }

                    strBlendGroupTarget[iCurGroupIdx] = BGS_ACTIVE;
                }
                else if (!pBlendGroup[iCurGroupIdx].AnimInfo.bUse && pBlendGroup[iCurGroupIdx].AnimInfo.bInterpolationing)
                    strBlendGroupTarget[iCurGroupIdx] = BGS_INTERPOLATION;


                // 보간인 애니매이션 정보들을 넣는다.
                vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecInterpolationIndices;
                for (_int iDiffGroupIdx = iLower; iDiffGroupIdx <= iTotal; ++iDiffGroupIdx)
                {
                    // 현재 그룹이 보간중이고 보간 대상 애니매이션 인덱스가 존재하는경우, 보간 대상 애니매이션 인덱스를 넣는다.
                    if (!pBlendGroup[iCurGroupIdx].AnimInfo.bUse && pBlendGroup[iCurGroupIdx].AnimInfo.bInterpolationing)
                    {
                        if (pBlendGroup[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iDiffGroupIdx] != -1)
                            vecInterpolationIndices.push_back({ ANIM_BLEND_GROUP_TYPE(iDiffGroupIdx), pBlendGroup[iCurGroupIdx].InterpolationInfo.InterpolationDstGroupIdices[iDiffGroupIdx] });
                    }
                }


                if (strBlendGroupTarget[iCurGroupIdx] == BGS_ACTIVE)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
                else if (strBlendGroupTarget[iCurGroupIdx] == BGS_INTERPOLATION)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
                else if (strBlendGroupTarget[iCurGroupIdx] == BGS_INTERPOLATION_TARGET)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 1.f, 1.f));


                ImGui::SeparatorText(strBlendBoneTypes[iCurGroupIdx].c_str());
                ImGui::Text("Cur Anim Name : %s | Cur Anim Idx : %d", strAnimName.c_str(), iCurAnimIdx);
                ImGui::Text("Cur Anim Duration : %f,  TrackPos : %f,  TickPerSecond : %f", fDuration, fTrackPos, fTickPerSecond);
                
                if (vecInterpolationIndices.empty())
                {
                    ImGui::Text("Interpolation Anim Name : None | Interpolation Anim Idx : -1");
                    ImGui::Text("Interpolation Distance : %f,  Interpolation SP : %f ", 0.2f, 1.f);
                }
                else
                {
                    for (_int j = 0; j < vecInterpolationIndices.size(); ++j)
                    {
                        ImGui::Text("Interpolation Anim Name : %s | Interpolation Anim Idx : %d", m_pAT_PartAnimObj_Model->Get_AnimName(vecInterpolationIndices[j].second).c_str(), vecInterpolationIndices[j].second);
                        ImGui::Text("Interpolation Distance : %f,  Interpolation SP : %f", pBlendGroup[iCurAnimIdx].InterpolationInfo.fInterpolationDis, pBlendGroup[iCurAnimIdx].InterpolationInfo.fInterpolationSP);
                    }
                }

                ImGui::SeparatorText("");

                if (strBlendGroupTarget[iCurGroupIdx] != BGS_DEACTIVE)
                    ImGui::PopStyleColor();
                
            }
            
        }

        memset(strBlendGroupTarget, BGS_DEACTIVE, sizeof strBlendGroupTarget);

    }



    return true;
}

_bool CLevel_AnimTool::Show_AnimRootMotionType_RadioBtn()
{
    static _int iRootMotion = 0;
    ImGui::RadioButton("Default Anim Play", &iRootMotion, 0); ImGui::SameLine(); HelpMarker("Play Original Animation");  ImGui::SameLine();
    ImGui::RadioButton("Root Motion", &iRootMotion, 1); ImGui::SameLine(); HelpMarker("Play Root Animation : Fix position at zero, stay rotaion"); ImGui::SameLine();
    ImGui::RadioButton("Identity", &iRootMotion, 2); ImGui::SameLine(); HelpMarker("Play Identity : Fix position, fix rotation"); ImGui::SameLine();
    ImGui::RadioButton("Follow Root Bone", &iRootMotion, 3); ImGui::SameLine(); HelpMarker("Follow Root Bone Data");

    // 원점 재생 Deault
    if (iRootMotion == 0)
    {
        m_pAT_PartAnimObj_Model->Set_AnimPlayType(Engine::ANIM_PLAY_TYPE::ANIM_DEFAULT);

        m_pAT_PartAnimObj->m_bFollowRootBone = false;
        m_pAT_Obj->m_bFollowRootBone = false;
    }
    // 루트 모션인 경우, 회전은 유지하면서 원점에서 재생한다.
    else if (iRootMotion == 1)
    {
        m_pAT_PartAnimObj_Model->Set_AnimPlayType(Engine::ANIM_PLAY_TYPE::ANIM_ROOT_MOTION);

        m_pAT_PartAnimObj->m_bFollowRootBone = false;
        m_pAT_Obj->m_bFollowRootBone = false;
    }
    // 항등 재생인 경우, 위치 회전 모두 0이다.
    else if (iRootMotion == 2)
    {
        m_pAT_PartAnimObj_Model->Set_AnimPlayType(Engine::ANIM_PLAY_TYPE::ANIM_IDENTITY);

        m_pAT_PartAnimObj->m_bFollowRootBone = false;
        m_pAT_Obj->m_bFollowRootBone = false;
    }
    else if (iRootMotion == 3)
    {
        m_pAT_PartAnimObj_Model->Set_AnimPlayType(Engine::ANIM_PLAY_TYPE::ANIM_FOLLOW_ROOT_BONE);
        
        m_pAT_PartAnimObj->m_bFollowRootBone = true;
        m_pAT_Obj->m_bFollowRootBone = true;
    }

    return true;
}

_bool CLevel_AnimTool::Show_AnimPlayType_ComboBox()
{
    static const _char* pAnimPlayTestTypes[] = { "Default", "Loop Default", "Loop Interpolation", "Loop Blending" };
    static _int iSelectedAnimPlayTestTypeIdx = 0;
    const _char* pComboSelectedAnimPlaytestTypeName = pAnimPlayTestTypes[iSelectedAnimPlayTestTypeIdx];

    if (ImGui::BeginCombo("anim play test type combo", pComboSelectedAnimPlaytestTypeName))
    {
        for (int n = 0; n < IM_ARRAYSIZE(pAnimPlayTestTypes); n++)
        {
            const bool is_selected = (iSelectedAnimPlayTestTypeIdx == n);
            if (ImGui::Selectable(pAnimPlayTestTypes[n], is_selected))
            {
                iSelectedAnimPlayTestTypeIdx = n;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    static _int iPrevSelectedAnimPlayTestIdx = 0;

    // 콤보 박스에서 선택한 인덱스에 따라서 어떤 함수를 호출 할지 셋팅한다.

    if (iSelectedAnimPlayTestTypeIdx == 0)
        m_eAnimPlayTestType = ANIM_PLAY_TEST_TYPE::DEFAULT;
    else if (iSelectedAnimPlayTestTypeIdx == 1)
        m_eAnimPlayTestType = ANIM_PLAY_TEST_TYPE::LOOP_DEFAULT;
    else if (iSelectedAnimPlayTestTypeIdx == 2)
        m_eAnimPlayTestType = ANIM_PLAY_TEST_TYPE::LOOP_INTERPOLATION;
    else if (iSelectedAnimPlayTestTypeIdx == 3)
        m_eAnimPlayTestType = ANIM_PLAY_TEST_TYPE::LOOP_BLENDING;

    static _bool bOnce = false;

    // 타입에 따라 다른 애니매이션 재생
    switch (m_eAnimPlayTestType)
    {
    case ANIM_PLAY_TEST_TYPE::DEFAULT:
    {
        if (bOnce)
        {
            for (_int i = _int(ANIM_BLEND_GROUP_TYPE::LOWER_BODY); i <= _int(ANIM_BLEND_GROUP_TYPE::TOTAL); ++i)
                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);
        }

        bOnce = false;
        m_pAT_PartAnimObj->m_bIsPlayAnimBlend = false;
        Show_Default_AnimPlayTest();
    }
        
        break;
    case ANIM_PLAY_TEST_TYPE::LOOP_DEFAULT:
    {
        if (bOnce)
        {
            for (_int i = _int(ANIM_BLEND_GROUP_TYPE::LOWER_BODY); i <= _int(ANIM_BLEND_GROUP_TYPE::TOTAL); ++i)
                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);
        }

        bOnce = false;
        m_pAT_PartAnimObj->m_bIsPlayAnimBlend = false;
        Show_Loop_Default_AnimPlayTest();
    }

        break;
    case ANIM_PLAY_TEST_TYPE::LOOP_INTERPOLATION:
    {
        if (bOnce)
        {
            for (_int i = _int(ANIM_BLEND_GROUP_TYPE::LOWER_BODY); i <= _int(ANIM_BLEND_GROUP_TYPE::TOTAL); ++i)
                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);
        }

        bOnce = false;

        // 애니매이션 src(start) -> dst(end) 보간 제어
        Show_Loop_Interpolation_AnimPlayTest();
        m_pAT_PartAnimObj->m_bIsPlayAnimBlend = false;
    }
    break;
    case ANIM_PLAY_TEST_TYPE::LOOP_BLENDING:
    {
        if (!bOnce)
        {
            for (_int i = _int(ANIM_BLEND_GROUP_TYPE::LOWER_BODY); i <= _int(ANIM_BLEND_GROUP_TYPE::TOTAL); ++i)
                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);
        }

        if (!bOnce)
        {
            bOnce = true;
            m_pAT_PartAnimObj_Model->Reset_CurAnim();
            m_pAT_PartAnimObj->m_bIsPlayAnimBlend = true;
        }

        Show_Loop_Blending_AnimPlayTest();
    }
    break;
    default:
        break;
    }

    // 콤보 타입을 바꾼 경우
    if (iPrevSelectedAnimPlayTestIdx != iSelectedAnimPlayTestTypeIdx)
    {
        iPrevSelectedAnimPlayTestIdx = iSelectedAnimPlayTestTypeIdx;
    }
    
    return true;
}

void CLevel_AnimTool::Show_Default_AnimPlayTest()
{
    // 선택가능한 애니매이션 목록들을 보여주고 선택하면 변경이 가능하도록 해주는 함수
    Show_Selectable_AnimList_ListBox();

    // 애니매이션 duration, tickpersecond(애니매이션 재생 속도) 등 출력 및 제어
    Show_AnimInfos_ListBox();

    // 버튼 크기 설정 (가로 150, 세로 50)
    ImVec2 buttonSize = ImVec2(150, 50);

    // 함수 호출 (왼쪽과 오른쪽 버튼 레이블 지정)
    Show_PlayAnim_Btn("Prev Anim", "Stop Or Play Anim", "Next Anim", buttonSize);
}

void CLevel_AnimTool::Show_AnimInfos_ListBox()
{
    // 현재 모델의 재생중인 애니매이션 객체 Get -> 해당 애니매이션의 영향을 주는 뼈 목록 가져오기, KeyFrame가져오기
    // m_pCurShowObjectModel->Get
    auto pCurAnim = m_pAT_PartAnimObj_Model->Get_CurAnim();
    if (pCurAnim)
    {
        m_fTickPerSecond = pCurAnim->Get_TickPerSecond();
        _float fCustumTickPerSecond = pCurAnim->Get_TickPerSecond();

        ImGui::InputFloat("anim play speed input", &fCustumTickPerSecond);
        ImGui::DragFloat("anim play speed drag", &fCustumTickPerSecond, 0.01f, 0.f, 100.f);
        
        if (m_fTickPerSecond != fCustumTickPerSecond)
            pCurAnim->Set_TickPerSecond(fCustumTickPerSecond);
    }
}

void CLevel_AnimTool::Add_Log(AT_LOG_TYPE eLogType, AT_LOG_JOB eLogJob, string funcMsg, string Msg)
{
    string strRetLogMsg = "";
    switch (eLogType)
    {
    case LOG_OK: 
        strRetLogMsg += "LOG_OK | ";
        break;
    case LOG_FAIL:
        strRetLogMsg += "LOG_FAIL | ";
        break;
    case LOG_:
        strRetLogMsg += "LOG_ | ";
        break;
    case LOG_END:
        break;
    default:
        break;
    }

    switch (eLogJob)
    {
    case XML_LOAD:
        strRetLogMsg += "XML_LOAD | ";
        break;
    case XML_SAVE:
        strRetLogMsg += "XML_SAVE | ";
        break;
    case OPEN:
        strRetLogMsg += "OPEN | ";
        break;
    case CLOSE:
        strRetLogMsg += "CLOSE | ";
        break;
    default:
        break;
    }

    strRetLogMsg += funcMsg;
    strRetLogMsg += " | ";
    strRetLogMsg += Msg;

    m_LogBuffer.push_back(strRetLogMsg);
}

void CLevel_AnimTool::Show_AnimLog()
{
    ImGui_Fix_Window_Pos({ ImGui_Get_ViewPort_StartPos().x, ImGui_Get_ViewPort_StartPos().y + g_iWinSizeY - m_LogWindowScale.y });

    // 창 크기 조정 가능하도록 최소/최대 크기 제한 없음
    ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX));

    ImGui::Begin("Log Window");

    // 필터 UI 추가
    m_LogFilter.Draw("Filter (\"incl,-excl\")");

    // Clear 버튼
    if (ImGui::Button("Clear")) 
    {
        m_LogBuffer.clear();
    }

    ImGui::BeginChild("LogChild", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    for (const auto& log : m_LogBuffer)
    {
        if (m_LogFilter.PassFilter(log.c_str()))
        {
            ImGui::TextUnformatted(log.c_str());
        }
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();

}

void CLevel_AnimTool::Show_PlayAnimTest_MultiDefault_Second()
{
    // 애니메이션 리스트 박스 폭 조정
    _float listBoxWidth = 400.0f; // 원하는 리스트 박스 폭 설정
    _float listBoxHeight = 200.0f; // 원하는 리스트 박스 높이 설정

    static _int iSelectSrcAnimIdx = 0;
    static _int iSelectDstAnimIdx = 0;

    // 시작할 애니매이션
    {
        ImGui::BeginGroup(); // 그룹 시작 (가로 배치를 위한 그룹)

        ImGui::Text("src-anim");
        ImGui::SetNextItemWidth(listBoxWidth);
        if (ImGui::BeginListBox("##Src(Start) Anim List", ImVec2(listBoxWidth, listBoxHeight)))
        {
            static _bool bSelectSrcAnimHighLighten = false;

            for (_int i = 0; i < m_AnimNamesCStr.size(); i++)
            {
                const _bool is_selected = (iSelectSrcAnimIdx == i);
                if (ImGui::Selectable(m_AnimNamesCStr[i], is_selected))
                    iSelectSrcAnimIdx = i;

                if (bSelectSrcAnimHighLighten && ImGui::IsItemHovered())
                    iSelectSrcAnimIdx = i;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
        ImGui::SameLine();
        ImGui::EndGroup(); // 그룹 종료
    }

    ImGui::SameLine();

    // 끝날 애니매이션
    {
        ImGui::BeginGroup(); // 그룹 시작 (가로 배치를 위한 그룹)
        ImGui::Text("dst-anim");
        ImGui::SetNextItemWidth(listBoxWidth);
        if (ImGui::BeginListBox("##Dst(End) Anim List", ImVec2(listBoxWidth, listBoxHeight)))
        {
            static _bool bSelectSrcAnimHighLighten = false;

            for (_int i = 0; i < m_AnimNamesCStr.size(); i++)
            {
                const _bool is_selected = (iSelectDstAnimIdx == i);
                if (ImGui::Selectable(m_AnimNamesCStr[i], is_selected))
                    iSelectDstAnimIdx = i;

                if (bSelectSrcAnimHighLighten && ImGui::IsItemHovered())
                    iSelectDstAnimIdx = i;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndListBox();
        }

        ImGui::SameLine();
        ImGui::EndGroup(); // 그룹 종료
    }

    if (ImGui::Button("play default anim loop test"))
    {
        PlayAnimTest_StartBtn(iSelectSrcAnimIdx, iSelectDstAnimIdx, PartAnimObj_Type::ANIM_PLAY_TYPE::MULTI_DEFAULT);
    }
}

void CLevel_AnimTool::Show_PlayAnimTest_MultiDefault_Third()
{
    return;


    // 애니메이션 리스트 박스 폭 조정
    _float listBoxWidth = 200.0f; // 원하는 리스트 박스 폭 설정

    static _int iSelectSrcAnimIdx = 0;
    static _int iSelectMidAnimIdx = 0;
    static _int iSelectDstAnimIdx = 0;

    // 시작할 애니매이션
    {
        ImGui::BeginGroup(); // 그룹 시작 (가로 배치를 위한 그룹)

        ImGui::Text("src-anim");
        ImGui::SetNextItemWidth(listBoxWidth);
        if (ImGui::BeginListBox("##Src(Start) Anim List"))
        {
            static _bool bSelectSrcAnimHighLighten = false;

            for (_int i = 0; i < m_AnimNamesCStr.size(); i++)
            {
                const _bool is_selected = (iSelectSrcAnimIdx == i);
                if (ImGui::Selectable(m_AnimNamesCStr[i], is_selected))
                    iSelectSrcAnimIdx = i;

                if (bSelectSrcAnimHighLighten && ImGui::IsItemHovered())
                    iSelectSrcAnimIdx = i;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
        ImGui::SameLine();
        HelpMarker("Here we are select start anim."); // 보간을 시작할 애니매이션 선택한다.

        ImGui::EndGroup(); // 그룹 종료
    }

    ImGui::SameLine();

    // 중간 애니매이션
    {
        ImGui::BeginGroup(); // 그룹 시작 (가로 배치를 위한 그룹)
        ImGui::Text("mid-anim");
        ImGui::SetNextItemWidth(listBoxWidth);
        if (ImGui::BeginListBox("##Mid Anim List"))
        {
            static _bool bSelectSrcAnimHighLighten = false;

            for (_int i = 0; i < m_AnimNamesCStr.size(); i++)
            {
                const _bool is_selected = (iSelectMidAnimIdx == i);
                if (ImGui::Selectable(m_AnimNamesCStr[i], is_selected))
                    iSelectMidAnimIdx = i;

                if (bSelectSrcAnimHighLighten && ImGui::IsItemHovered())
                    iSelectMidAnimIdx = i;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndListBox();
        }

        ImGui::SameLine();
        HelpMarker("Here we are select mid anim."); // 보간을 시작할 애니매이션 선택한다.
        ImGui::EndGroup(); // 그룹 종료
    }

    ImGui::SameLine();

    // 끝날 애니매이션
    {
        ImGui::BeginGroup(); // 그룹 시작 (가로 배치를 위한 그룹)
        ImGui::Text("dst-anim");
        ImGui::SetNextItemWidth(listBoxWidth);
        if (ImGui::BeginListBox("##Dst(End) Anim List"))
        {
            static _bool bSelectSrcAnimHighLighten = false;

            for (_int i = 0; i < m_AnimNamesCStr.size(); i++)
            {
                const _bool is_selected = (iSelectDstAnimIdx == i);
                if (ImGui::Selectable(m_AnimNamesCStr[i], is_selected))
                    iSelectDstAnimIdx = i;

                if (bSelectSrcAnimHighLighten && ImGui::IsItemHovered())
                    iSelectDstAnimIdx = i;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndListBox();
        }

        ImGui::SameLine();
        HelpMarker("Here we are select end anim."); // 보간을 시작할 애니매이션 선택한다.
        ImGui::EndGroup(); // 그룹 종료
    }

    if (ImGui::Button("play interpolation test"))
    {
        
    }
}

void CLevel_AnimTool::Show_PlayAnimTest_MultiInterpolation_Second()
{
    // 애니메이션 리스트 박스 폭 조정
    _float listBoxWidth = 400.0f; // 원하는 리스트 박스 폭 설정

    static _int iSelectSrcAnimIdx = 0;
    static _int iSelectDstAnimIdx = 0;

    // 시작할 애니매이션
    {
        ImGui::BeginGroup(); // 그룹 시작 (가로 배치를 위한 그룹)

        ImGui::Text("src-anim");
        ImGui::SetNextItemWidth(listBoxWidth);
        if (ImGui::BeginListBox("##Src(Start) Anim List"))
        {
            static _bool bSelectSrcAnimHighLighten = false;

            for (_int i = 0; i < m_AnimNamesCStr.size(); i++)
            {
                const _bool is_selected = (iSelectSrcAnimIdx == i);
                if (ImGui::Selectable(m_AnimNamesCStr[i], is_selected))
                    iSelectSrcAnimIdx = i;

                if (bSelectSrcAnimHighLighten && ImGui::IsItemHovered())
                    iSelectSrcAnimIdx = i;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
        ImGui::SameLine();
        ImGui::EndGroup(); // 그룹 종료
    }

    ImGui::SameLine();

    // 끝날 애니매이션
    {
        ImGui::BeginGroup(); // 그룹 시작 (가로 배치를 위한 그룹)
        ImGui::Text("dst-anim");
        ImGui::SetNextItemWidth(listBoxWidth);
        if (ImGui::BeginListBox("##Dst(End) Anim List"))
        {
            static _bool bSelectSrcAnimHighLighten = false;

            for (_int i = 0; i < m_AnimNamesCStr.size(); i++)
            {
                const _bool is_selected = (iSelectDstAnimIdx == i);
                if (ImGui::Selectable(m_AnimNamesCStr[i], is_selected))
                    iSelectDstAnimIdx = i;

                if (bSelectSrcAnimHighLighten && ImGui::IsItemHovered())
                    iSelectDstAnimIdx = i;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndListBox();
        }

        ImGui::SameLine();
        ImGui::EndGroup(); // 그룹 종료
    }

    // ImGui::SameLine();

    // 보간 거리, 속도를 XML 에서 로드한 데이터를 사용하는 경우, 직접 테스트 하는 경우
    {
        ImGui::BeginGroup(); // 그룹 시작 (가로 배치를 위한 그룹)

        ImGui::SetNextItemWidth(listBoxWidth);

        // 보간에 사용할 데이터 타입 : XML과 직접 설정한 데이터 중 고른다.
        static const _char* pAnimPlayTestTypes[] = { "Use XML Data", "Use Custum Data"};
        
        static _int iSelectedAnimPlayTestDataTypeIdx = 0;
        const _char* pComboSelectedAnimPlayTestDataTypeName = pAnimPlayTestTypes[iSelectedAnimPlayTestDataTypeIdx];

        if (ImGui::BeginCombo("anim play test data type combo", pComboSelectedAnimPlayTestDataTypeName))
        {
            for (int n = 0; n < IM_ARRAYSIZE(pAnimPlayTestTypes); n++)
            {
                const bool is_selected = (iSelectedAnimPlayTestDataTypeIdx == n);
                if (ImGui::Selectable(pAnimPlayTestTypes[n], is_selected))
                {
                    iSelectedAnimPlayTestDataTypeIdx = n;
                    
                    // XML 데이터 사용하는 경우
                    if (iSelectedAnimPlayTestDataTypeIdx == 0)
                    {
                        m_pAT_PartAnimObj_Model->Set_UseCustumInterData(false);

                    }
                    // 커스텀 데이터 사용하는 경우
                    else if (iSelectedAnimPlayTestDataTypeIdx == 1)
                    {
                        m_pAT_PartAnimObj_Model->Set_UseCustumInterData(true);

                    }
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (iSelectedAnimPlayTestDataTypeIdx == 0)
            Show_Interpolation_UseXMLData();
        else if (iSelectedAnimPlayTestDataTypeIdx == 1)
            Show_Interpolation_UseCustumData();

        ImGui::Spacing();
        ImGui::EndGroup();
    }

    if (ImGui::Button("play anim loop interpolation test"))
    {
        m_bKeyDown_PlayDoubleAnimLoopInterpolationTest = true;

        // 버튼이 눌렸다면 
        PlayAnimTest_StartBtn(iSelectSrcAnimIdx, iSelectDstAnimIdx, PartAnimObj_Type::ANIM_PLAY_TYPE::MULTI_INTERPOLATION);
    }
    else
    {
        m_bKeyDown_PlayDoubleAnimLoopInterpolationTest = false;
    }
  

    Show_InterpolationInfo();
}

void CLevel_AnimTool::Show_Interpolation_UseXMLData()
{
    // xml 을 사용하도록 설정
    // m_pAT_PartAnimObj_Model->Set_UseCustumInterData(false);
}

void CLevel_AnimTool::Show_Interpolation_UseCustumData()
{
    // slider
    // m_pAT_PartAnimObj_Model->Set_UseCustumInterData(true);

    // ImGui::SetNextWindowSizeConstraints(ImVec2(100, 200), ImVec2(600, 400));

    const static _float fCustumInterDisStand = 0.2f;
    const static _float fCustumInterSPStand = 1.f;
    static _float fCustumInterDis = 0.2f;
    static _float fCustumInterSP = 1.f;

    ImGui::InputFloat("Custum Inter Dis Input", &fCustumInterDis, 0.01f);
    ImGui::DragFloat("Cumstum Inter Dis Drag", &fCustumInterDis, 0.01f, 0.01f, 5.f);
    
    ImGui::InputFloat("Custum Inter SP Input", &fCustumInterSP, 0.01f);
    ImGui::DragFloat("Cumstum Inter SP Drag", &fCustumInterSP, 0.01f, 0.1f, 5.f);

    if (fCustumInterDis != fCustumInterDisStand)
    {
        m_pAT_PartAnimObj_Model->Set_CustumInterDis(fCustumInterDis);
    }

    if (fCustumInterSP != fCustumInterSPStand)
    {
        m_pAT_PartAnimObj_Model->Set_CustumInterSP(fCustumInterSP);
    }


}

void CLevel_AnimTool::Show_PlayAnimTest_MultiInterpolation_Third()
{

}

void CLevel_AnimTool::PlayAnimTest_StartBtn(_int iSrcAnimIdx, _int iDstAnimIdx, PartAnimObj_Type::ANIM_PLAY_TYPE eType)
{
    m_pAT_PartAnimObj->Clear_AnimPlayLists();
    m_pAT_PartAnimObj_Model->Reset_CurAnim();

    // 현재 시작 재생 애니매이션을 변경한다.
    Change_CurAnimIdx(iSrcAnimIdx);

    // 재생할 애니매이션 목록을 추가해준다.
    m_pAT_PartAnimObj->Add_AnimIn_AnimPlayList(iSrcAnimIdx, 1);
    m_pAT_PartAnimObj->Add_AnimIn_AnimPlayList(iDstAnimIdx, 1);

    // 재생을 변경해준다.
    Play_Anim(Get_CurAnimIdx());

    // 플레이어의 애니매이션 재생 타입을 변경한다.
    m_pAT_PartAnimObj->Set_AnimPlayType(eType);
}

void CLevel_AnimTool::PlayAnimTest_StartBtn(_int iSrcAnimIdx, PartAnimObj_Type::ANIM_PLAY_TYPE eType)
{
    m_pAT_PartAnimObj->Clear_AnimPlayLists();
    m_pAT_PartAnimObj_Model->Reset_CurAnim();

    // 현재 시작 재생 애니매이션을 변경한다.
    Change_CurAnimIdx(iSrcAnimIdx);

    m_pAT_PartAnimObj->Add_AnimIn_AnimPlayList(iSrcAnimIdx, 1);

    // 애니매이션 재생을 변경해준다.
    Play_Anim(Get_CurAnimIdx());

    // 플레이어의 애니매이션 재생 타입을 변경한다.
    m_pAT_PartAnimObj->Set_AnimPlayType(eType);
}

void CLevel_AnimTool::Show_InterpolationInfo()
{
    ImGui::SeparatorText("interpolation infos");

    const static _float fInterDisMax = 5.f, fInterDisMin = 0.1f;
    const static _float fInterSPMax = 10.f, fInterSPMin = 1.f;
    
   /* static _float fInterDis = m_pAT_PartAnimObj->Get_InterDis();
    static _float fInterSP = m_pAT_PartAnimObj->Get_InterSP();

    _float fInterTrackPos = m_pAT_PartAnimObj_Model->Get_CurInterTrackPos();

    ImGui::Text("Cur InterDis : %f,  InterSP : %f,  InterTrackPos : %f", fInterDis, fInterSP, fInterTrackPos);*/

    //ImGui::BeginGroup();
    //{
    //    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f); // 창 크기에 맞게 폭 조정
    //    ImGui::InputFloat("InterDis input", &fInterDis);
    //    ImGui::DragFloat("InterDis drag", &fInterDis, 0.01f, fInterDisMin, fInterDisMax);
    //    ImGui::PopItemWidth();
    //}
    //ImGui::EndGroup();


    //ImGui::BeginGroup();
    //{
    //    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f); // 창 크기에 맞게 폭 조정
    //    ImGui::InputFloat("fInterSP input", &fInterSP);
    //    ImGui::DragFloat("fInterSP drag", &fInterSP, 0.01f, fInterSPMin, fInterSPMax);
    //    ImGui::PopItemWidth();
    //}
    //ImGui::EndGroup();

    //if (m_pAT_PartAnimObj->Get_InterDis() != fInterDis)
    //    m_pAT_PartAnimObj->Set_InterDis(fInterDis);
    //if (m_pAT_PartAnimObj->Get_InterSP())
    //    m_pAT_PartAnimObj->Set_InterSP(fInterSP);
}

void CLevel_AnimTool::Init_InterpolationInfoTable()
{
    auto& AnimNames = m_pAT_PartAnimObj_Model->Get_AnimNames();

    for (_int i = 0; i < AnimNames.size(); ++i)
    {
        for (_int j = 0; j < AnimNames.size(); ++j)
        {
            if (i == j) continue;

            ANIM_INTER_INFO InterInfo = {};
            InterInfo.strSrcAnimName = AnimNames[i];
            InterInfo.strDstAnimName = AnimNames[j];
            InterInfo.fInterDis = 0.2f;
            InterInfo.fInterSp = 1.f;

            m_InterpolationTableData.push_back(InterInfo);
        }
    }

    Add_Log(LOG_OK, OPEN,  "Show_InterpolationTable", "open ok");
}


_bool CLevel_AnimTool::Show_InterpolationTable()
{
    if (ImGui::TreeNode("interpolation infos settings"))
    {
        ImGui::SeparatorText("interpolation infos settings");

        // 보간 테이블 초기화하고 열기
        if (ImGui::Button("Open Interpolation info table"))
        {
            m_bShowInterTable = true;
            Init_InterpolationInfoTable();
        }

        // 보간 테이블 초기화 하고 닫기
        if (ImGui::Button("Close Interpolation info table"))
        {
            m_bShowInterTable = false;
            m_InterpolationTableData.clear();

            Add_Log(LOG_OK, OPEN, "Show_InterpolationTable", "close ok");
        }

        // 보간 테이블 렌더링 (수가 많으면 프레임 떨어질 수 도 있다)
        if (m_bShowInterTable)
            Show_InterpolationTableWindow();

        ImGui::TreePop();
    }

    

    return true;
}

_bool CLevel_AnimTool::Show_Anim_KeyFrames()
{
    if (m_eAnimPlayTestType == ANIM_PLAY_TEST_TYPE::DEFAULT)
    {
        if (ImGui::TreeNode("Anim Key Frames"))
        {

            if (m_pAT_PartAnimObj_Model)
            {
                string strCurAnimName = m_pAT_PartAnimObj_Model->Get_CurAnim()->Get_AnimName();
                _int iCurAnimIdx = m_pAT_PartAnimObj_Model->Get_AnimIdx(strCurAnimName);
                CAnimation* pCurAnim = m_pAT_PartAnimObj_Model->Get_Anim(iCurAnimIdx);
                if (pCurAnim)
                {
                    ImGui::SeparatorText("Cur Animation Default Infos");

                    // 현재 애니메이션 이름, 인덱스 출력
                    ImGui::Columns(2, "##columns", false); // 2개의 열 생성
                    ImGui::Text("Cur Animation Name"); ImGui::NextColumn();
                    ImGui::Text("%s", strCurAnimName.c_str()); ImGui::NextColumn();
                    ImGui::Text("Cur Animation Index"); ImGui::NextColumn();
                    ImGui::Text("%d", iCurAnimIdx); ImGui::NextColumn();

                    //// 현재 애니매이션 이름, 인덱스 출력
                    //ImGui::Text("Cur Animation Name : %s", strCurAnimName.c_str());
                    //ImGui::Text("Cur Animation Index : %d", iCurAnimIdx);


                    _float fCurAnimDuration = pCurAnim->Get_AnimDuration();
                    _float fCurAnimCurTrackPos = pCurAnim->Get_CurTrackPos();
                    _float fCurAnimTickPerSecond = pCurAnim->Get_TickPerSecond();

                    // 현재 애니매이션 길이, 재생 위치, 속도 출력
                    ImGui::Columns(2, "##columns", false); // 2개의 열 생성
                    ImGui::Text("Cur Animation Duration"); ImGui::NextColumn();
                    ImGui::Text("%f", fCurAnimDuration); ImGui::NextColumn();

                    ImGui::Text("Cur Animation TrackPos"); ImGui::NextColumn();
                    ImGui::Text("%f", fCurAnimCurTrackPos); ImGui::NextColumn();

                    ImGui::Text("Cur Animation TickPerSecond"); ImGui::NextColumn();
                    ImGui::Text("%f", fCurAnimTickPerSecond); ImGui::NextColumn();
                    ImGui::Columns(1); // 열 초기화

                    ImGui::SeparatorText("");


                    ImGui::SeparatorText("Cur Animation Channel Info");


                    // 현재 애니매이션이 가지고 있는 채널 수 출력
                    _int iCurAnimNumChannel = pCurAnim->Get_NumChannels();
                    ImGui::Text("Cur Animation NumOfChannel : %d", iCurAnimNumChannel);

                    if (ImGui::TreeNode("Show Chaanel KeyFrame Infos"))
                    {
                        vector<CChannel*>& vecChannels = pCurAnim->Get_Channels();

                        ImGui::Columns(4, "##columns", false); // 10개의 열 생성

                        ImGui::Text("%s", "Index"); ImGui::NextColumn();
                        ImGui::Text("%s", "KeyFrame Index"); ImGui::NextColumn();
                        ImGui::Text("%s", "Target Bone Index"); ImGui::NextColumn();
                        ImGui::Text("%s", "Target Bone Name"); ImGui::NextColumn();

                        ImGui::Columns(1); // 열 초기화

                        ImGui::SeparatorText("");

                        ImGui::Columns(4, "##columns", false); // 10개의 열 생성

                        vector<_uint>& vecCurAnimKeyFrameIndices = pCurAnim->Get_CurKeyFrameIndices();


                        for (_int i = 0; i < iCurAnimNumChannel; ++i)
                        {
                            ImGui::Text("%d", i); ImGui::NextColumn(); // CH Index
                            ImGui::Text("%d", vecCurAnimKeyFrameIndices[i]); ImGui::NextColumn(); // CH KeyFrame Index

                            _int iTargetBoneIdx = vecChannels[i]->Get_BoneIdx(); // 해당 채널이 가르키는 실제 뼈 인덱스
                            ImGui::Text("%d", iTargetBoneIdx); ImGui::NextColumn();


                            string strTargetBoneName = m_pAT_PartAnimObj_Model->Get_BoneName(iTargetBoneIdx); // 해당 채널이 가르키는 실제 뼈 인덱스 뼈의 이름
                            ImGui::Text("%s", strTargetBoneName.c_str()); ImGui::NextColumn();


                        }


                        ImGui::Columns(1); // 열 초기화

                        ImGui::TreePop();
                    }
                   

                    ImGui::SeparatorText("");




                    //for (_int i = 0; i < vecChannels.size(); ++i)
                    //{
                    //    _int iTargetBoneIdx = vecChannels[i]->Get_BoneIdx(); // 해당 채널이 가르키는 실제 뼈 인덱스
                    //    _uint iNumKeyFrame =  vecChannels[i]->Get_NumKeyFrames();
                    //    const _char* pChName = vecChannels[i]->Get_ChannelName();

                    //    vector<KEYFRAME>& vecKeyFrame = vecChannels[i]->Get_KeyFrame();
                    //}
                }
            }

            ImGui::TreePop();
        }
    }



    return true;
}

void CLevel_AnimTool::Show_InterpolationTableWindow()
{
    // 창 열림 상태를 관리하는 변수
    // ImGui::SetNextWindowSize(ImVec2(800, 500)); // 원하는 크기로 창 크기 설정

    // 새 창 생성
    if (m_bShowInterTable && ImGui::Begin("Animation Interpolation Table", &m_bShowInterTable,  ImGuiWindowFlags_NoCollapse))
    {
        if (ImGui::TreeNode("Open Interpolation Table"))
        {
            // 테이블 시작
            ImGui::BeginTable("Animation Transitions", 4, ImGuiTableFlags_Resizable);

            // 테이블 헤더
            ImGui::TableSetupColumn("Src Anim Name");
            ImGui::TableSetupColumn("Dst Anim Name");
            ImGui::TableSetupColumn("Inter Dis");
            ImGui::TableSetupColumn("Inter SP");

            ImGui::TableHeadersRow();

            // 애니메이션 전환 정보 입력
            for (size_t i = 0; i < m_InterpolationTableData.size(); ++i)
            {
                ImGui::TableNextRow();

                // Src Anim Name 열
                ImGui::TableNextColumn();
                ImGui::Text("%s", m_InterpolationTableData[i].strSrcAnimName.c_str());

                // Dst Anim Name 열
                ImGui::TableNextColumn();
                ImGui::Text("%s", m_InterpolationTableData[i].strDstAnimName.c_str());

                const static _float fInterDisMin = 0.1f, fInterDisMax = 3.f;
                const static _float fInterSpMin = 0.05f, fInterSpMax = 10.f;

                // Inter Dis 열 (값을 입력받을 수 있도록)
                ImGui::TableNextColumn();
                ImGui::InputFloat(("ip d" + to_string(i)).c_str(), &m_InterpolationTableData[i].fInterDis, 0.01f);

                // Inter SP 열 (값을 입력받을 수 있도록)
                ImGui::TableNextColumn();
                ImGui::InputFloat(("ip s" + to_string(i)).c_str(), &m_InterpolationTableData[i].fInterSp, 0.01f);
            }

            // 테이블 종료
            ImGui::EndTable();

            ImGui::TreePop();
        }

        if (ImGui::Button("Save This Interpolation Data To XML"))
        {
            WCHAR retFilePath[256];
            if (Open_Save_Modal("xml", retFilePath))
            {
                if (FAILED(XML_Save_InterpolationTableData(CONVERTTOSTRING(retFilePath))))
                {
                    MSG_BOX("Save_InterpolationTableData Fail!!!");
                }
            }
        }
    }

    ImGui::End(); // "Animation Transitions" 창 종료
}


void CLevel_AnimTool::XML_Add_Comment(xml::XMLDocument* pDoc)
{
    // 현재 날짜와 시간 가져오기
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S"); // 형식: YYYY-MM-DD HH:MM:SS
    std::string dateTimeStr = "Generated on: " + oss.str();

    // 주석 추가
    xml::XMLComment* pComment = pDoc->NewComment(dateTimeStr.c_str());
    pDoc->LinkEndChild(pComment);
}

void CLevel_AnimTool::XML_Add_Comment(xml::XMLDocument* pDoc, string strComment)
{
    // 주석 추가
    xml::XMLComment* pComment = pDoc->NewComment(strComment.c_str());
    pDoc->LinkEndChild(pComment);
}

HRESULT CLevel_AnimTool::XML_Save_InterpolationTableData(string strXMLFileName)
{
    auto& AnimNames = m_pAT_PartAnimObj_Model->Get_AnimNames();
    _int iAnimSize = AnimNames.size(); // Fiona 기준으로 애니매이션 25개

    // 진행 상태 추적을 위한 변수
    m_bSaveInterDataFromTable = true;
    m_iTotInterDataSize = m_InterpolationTableData.size();
    m_iCurInterDataSaveCnt = 0;

    // 루트 요소인 <AnimTransitions> 추가
    xml::XMLDocument doc;

    xml::XMLDeclaration* pDecl = doc.NewDeclaration();
    doc.LinkEndChild(pDecl);

    XML_Add_Comment(&doc); // 날짜 추가

    xml::XMLElement* pRoot = doc.NewElement("AnimationInterpolationData");     // + 현재 저장한 시간 붙이기
    pRoot->SetAttribute("AnimationCount", iAnimSize);
    pRoot->SetAttribute("DefaultInterPolationDis", 0.2f);
    pRoot->SetAttribute("DefaultInterpolationSP", 1.f);
    doc.LinkEndChild(pRoot);

    _int m_InterpolationTableDataIdx = 0;
    for (_int i = 0; i < AnimNames.size(); ++i)
    {
        xml::XMLElement* pNodeAnimName = doc.NewElement("Animation"); 
        pNodeAnimName->SetAttribute("AnimName", AnimNames[i].c_str()); // 속성 값으로 AnimNames[i] 이름 추가
        pNodeAnimName->SetAttribute("AnimIdx", i); // 속성 값으로 인덱스 추가
        pRoot->LinkEndChild(pNodeAnimName);

        _int adjustedIdx = 0; // i == j를 건너뛴 j 값을 조정
        for (_int j = 0; j < AnimNames.size(); ++j)
        {
            if (i == j) continue;

            // 애니매이션이 A B C 있다고 치자 m_InterpolationTableData에는 A->B, A->C, B->A, B->C, C->A, C->B 의 dis, sp가 담겨 있다.
            // 총 6개의 애니매이션이 있고 인덱스 증가가 0, 1, 2, 3, 4, 5가 되어야 한다.
            //  m_InterpolationTableDataIdx = i * (AnimNames.size() - 1) + j는 0, 1, 2, 3, 4, 5의 증가를 보여준다.
            // i * 2 + j => 0 1, 2 3, 4 5
            m_InterpolationTableDataIdx = i * (AnimNames.size() - 1) + adjustedIdx;
            adjustedIdx++;
            
            ANIM_INTER_INFO Info = m_InterpolationTableData[m_InterpolationTableDataIdx];

            xml::XMLElement* pNodeAnimInterInfo = doc.NewElement("InterpolationInfo");

            pNodeAnimInterInfo->SetAttribute("SrcAnim", Info.strSrcAnimName.c_str());
            pNodeAnimInterInfo->SetAttribute("SrcAnimIdx", i);
            pNodeAnimInterInfo->SetAttribute("DstAnim", Info.strDstAnimName.c_str());
            pNodeAnimInterInfo->SetAttribute("DstAnimIdx", j);
            pNodeAnimInterInfo->SetAttribute("InterpolationDistance", Info.fInterDis);
            pNodeAnimInterInfo->SetAttribute("InterpolationSpeed", Info.fInterSp);

            // InterpolationInfo 노드를 pNodeAnimName 노드의 자식으로 추가
            pNodeAnimName->LinkEndChild(pNodeAnimInterInfo);
        }
    }

    m_bSaveInterDataFromTable = false;
    m_iTotInterDataSize = 0;
    m_iCurInterDataSaveCnt = 0;


    fs::path fullPathObj = fs::absolute(strXMLFileName);        // 절대 경로로 변환
    fs::path dirPath = fullPathObj.parent_path();               // 디렉터리 경로만 추출

    // 경로가 존재 하지 않으면 생성한다
    if (!fs::exists(dirPath))
    {
        fs::create_directories(dirPath);
    }

    // XML 파일 저장
    // tinyxml2::XMLError result = doc.SaveFile(fullPathObj.string().c_str());
    xml::XMLError result = doc.SaveFile(strXMLFileName.c_str());

    // 저장 실패 시 E_FAIL 반환
    if (result != xml::XML_SUCCESS)
    {
        Add_Log(LOG_FAIL, XML_SAVE, "XML_Save_InterpolationTableData", "Fail");
        return E_FAIL;
    }

    fs::path fileName = fullPathObj.filename();
    string str = fileName.string() + "저장 성공";

    MSG_BOX("Anim Interpolation XML Save Success");
    Add_Log(LOG_OK, XML_SAVE, "XML_Save_InterpolationTableData", strXMLFileName);
    return S_OK;
}

//void CLevel_AnimTool::Show_AnimPlayTest_FileLoad()
//{
//    ImGui::SeparatorText("Show-Anim-Play-Test-With-FileLoad");
//    
//    // Default Play Test : 하나의 애니매이션을 루프로 재생하고 싶은 경우
//    // Multi Default Play Test : 2개 이상의 애니매이션을 보간없이 재생하고 싶은 경우
//    // Multi Interpolation Play Test : 2개 이상의 애니매이션을 보간을 하여 재생하고싶은 경우
//    static const _char* pAnimLoadDataType[] = { "None", "Load Anim Interpolation XML Data From Modal",  "Load Anim Interpolation XML Data From Modle" };
//
//    static _int iSelectAnimPlayTypeIdx = 0;
//    const _char* pComboSelectName = pAnimLoadDataType[iSelectAnimPlayTypeIdx];
//
//    if (ImGui::BeginCombo("Anim Load Type Combo", pComboSelectName))
//    {
//        for (int n = 0; n < IM_ARRAYSIZE(pAnimLoadDataType); n++)
//        {
//            const bool is_selected = (iSelectAnimPlayTypeIdx == n);
//            if (ImGui::Selectable(pAnimLoadDataType[n], is_selected))
//                iSelectAnimPlayTypeIdx = n;
//
//            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
//            if (is_selected)
//                ImGui::SetItemDefaultFocus();
//        }
//        ImGui::EndCombo();
//    }
//
//    ImGui::Spacing();
//
//    // 애니매이션 보간 XML 파일 모달로 로드하는 경우
//    if (iSelectAnimPlayTypeIdx == 1)
//    {
//
//        WCHAR strRetModelPath[256]; // 로드할 모델 최종 경로
//        if (Open_Modal("xml", strRetModelPath))
//        {
//            iSelectAnimPlayTypeIdx = 0;
//            Load_ParseAnimInterpolationData(strRetModelPath);
//        }
//        else
//        {
//            iSelectAnimPlayTypeIdx = 0;
//        }
//    }
//    //// 애니매이션 보간 XML 파일 Model함수를 호출해서 로드하는 경우
//    //else if (iSelectAnimPlayTypeIdx == 2)
//    //{
//    //    if (m_pAT_PartAnimObj_Model)
//    //    {
//    //        _wstring strRelativeFilePath = L"..\\Bin\\DataFiles\\AnimData\\Test\\player.xml";
//    //        fs::path absolutePath = fs::absolute(strRelativeFilePath);
//    //        if (FAILED(m_pAT_PartAnimObj_Model->Load_Anim_InterXML(absolutePath.wstring().c_str())))
//    //        {
//    //            MSG_BOX("Load xml file fail!");
//    //            iSelectAnimPlayTypeIdx = 0;
//    //        }
//    //        MSG_BOX("Load xml file success!");
//    //        iSelectAnimPlayTypeIdx = 0;
//    //    }
//    //}
//}

void CLevel_AnimTool::XML_Load_ParseAnimInterpolationData(const WCHAR* strXMLFilePath)
{
    if (m_pAT_PartAnimObj_Model)
    {
        if (FAILED(m_pAT_PartAnimObj_Model->Load_Anim_InterXML(strXMLFilePath)))
        {
            Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_ParseAnimInterpolationData", CONVERTTOSTRING(strXMLFilePath) + " Load Fail");
            MSG_BOX("Load xml fail");
            return;
        }

        MSG_BOX("OK : 현재 모델에 보간 테이블 정보가 로드 및 저장되었습니다.");
        Add_Log(LOG_OK, XML_LOAD, "XML_Load_ParseAnimInterpolationData", CONVERTTOSTRING(strXMLFilePath) + " Load xml OK");
        return;
    }
    else
    {
        MSG_BOX("아직 로드된 FBX파일이 없습니다. FBX파일 부터 로드하세요");
        return;
    }
    // 아래 코드 실행 안한다.

    return;

    // 루트 요소인 <AnimTransitions> 추가
    xml::XMLDocument doc;

    if (!doc.LoadFile(CONVERTTOSTRING(strXMLFilePath).c_str()) == xml::XML_SUCCESS)
    {
        MSG_BOX("Load Fail");
        Add_Log(LOG_OK, XML_LOAD, "XML_Load_ParseAnimInterpolationData", "Load OK");
        return;
    }

    _int iAnimCnt = 0;
    _float fDefaultInterDis, fDefaultInterSP;

    xml::XMLElement* pReadAnimaInterpolationDataNode = doc.FirstChildElement("AnimationInterpolationData"); // 루트 노드
    pReadAnimaInterpolationDataNode->FindAttribute("AnimationCount")->QueryIntValue(&iAnimCnt);

    if (!iAnimCnt)
    {
        MSG_BOX("iAnimCnt == 0 Error Load Fail");
        Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_ParseAnimInterpolationData", "iAnimCnt == 0 Error Load Fail");
        return;
    }

    pReadAnimaInterpolationDataNode->FindAttribute("DefaultInterPolationDis")->QueryFloatValue(&fDefaultInterDis);
    pReadAnimaInterpolationDataNode->FindAttribute("DefaultInterpolationSP")->QueryFloatValue(&fDefaultInterSP);
    
    if (fDefaultInterDis <= 0.f || fDefaultInterSP <= 0.f)
    {
        MSG_BOX("fDefaultInterDis fDefaultInterSP Error Load Fail");
        Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_ParseAnimInterpolationData", "fDefaultInterDis fDefaultInterSP Error Load Fail");
        return;
    }

    vector<map<_int, pair<_float, _float>>> InterpolationInfos;
    InterpolationInfos.resize(iAnimCnt);

    // 애니메이션 노드 읽기
    xml::XMLElement* pAnimNode = pReadAnimaInterpolationDataNode->FirstChildElement("Animation");
    
    for (_int i = 0; i < iAnimCnt; ++i)
    {
        if (!pAnimNode)
        {
            // cout << "Eror : Anim node not found!" << endl;
            return;
        }

        const _char* pAnimName = pAnimNode->Attribute("Name");
        if (pAnimName) 
        {
            // cout << "Animation Name: " << pAnimName << endl;
        }

        // InterpolationInfo 노드 읽기
        xml::XMLElement* pInterpolationInfoNode = pAnimNode->FirstChildElement("InterpolationInfo");
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
            {
                InterpolationInfos[i].insert({ iDstIdx, {interDis, interSP} });
            }

            cout << "  SrcAnim: " << (srcAnim ? srcAnim : "None")
                << ", SrcAnimIdx : " << iSrcIdx
                << ", DstAnim : " << (dstAnim ? dstAnim : "None")
                << ", SrcAnim : " << iDstIdx
                << ", InterpolationDistance: " << interDis
                << ", InterpolationSpeed: " << interSP << endl;

            pInterpolationInfoNode = pInterpolationInfoNode->NextSiblingElement("InterpolationInfo");
        }

        // 다음 애니메이션으로 이동
        pAnimNode = pAnimNode->NextSiblingElement("Animation");
    }
}

HRESULT CLevel_AnimTool::XML_Save_Bone_Hierachy(string strXMLFileName, string strBoneOwnerName)
{
    if (!m_pAT_Obj)
        return E_FAIL;

    m_BonesIndices.clear();
    
    string strXMLSavePath = "..\\Bin\\DataFiles\\AnimData\\";
    string strSvaeFullPath = strXMLSavePath + strXMLFileName + ".xml";

    const auto pai = m_pAT_PartAnimObj_Model->Get_AIScene();
    if (!pai)
        return E_FAIL;

    xml::XMLDocument* pDoc = new xml::XMLDocument;


    xml::XMLDeclaration* pDecl = pDoc->NewDeclaration();
    pDoc->LinkEndChild(pDecl);

    XML_Add_Comment(pDoc); // 날짜 추가

    
    xml::XMLElement* pRoot = pDoc->NewElement("BoneHierachy");
    pRoot->SetAttribute("Name", strBoneOwnerName.c_str());
    // pRoot->SetAttribute("NumBones", pai->mRootNode->); // 뼈 전체 개수 쓰기
    pDoc->LinkEndChild(pRoot);  // pDoc의 자식으로 pRoot를 추가한다.
    
    // 루트노드 뼈 쓰기
    aiNode* pRootNode = pai->mRootNode;
    xml::XMLElement* pRootBoneNodeElement = pDoc->NewElement("Bone");
    pRootBoneNodeElement->SetAttribute("Name", pRootNode->mName.C_Str());
    pRootBoneNodeElement->SetAttribute("Index", 0);
    pRootBoneNodeElement->SetAttribute("ParentIndex", -1);
    pRootBoneNodeElement->SetAttribute("NumChild", pRootNode->mNumChildren);

    // 인덱스를 구하기 위한 더미 데이터 추가
    m_BonesIndices.push_back(-1);

    _int iNumTotChildBone = static_cast<_int>(pRootNode->mNumChildren)  + 1; // 루트 뼈의 자식 + 본인 뼈개수
    
    for (_int i = 0; i < static_cast<_int>(pRootNode->mNumChildren); ++i)
        iNumTotChildBone += Read_Bone(pDoc, pRootNode->mChildren[i], pRootBoneNodeElement);
    
    pRootBoneNodeElement->SetAttribute("NumTotalChildBone", iNumTotChildBone);
    pRoot->LinkEndChild(pRootBoneNodeElement);  // pRoot의 자식으로 pRootBoneNode를 추가한다.
    pDoc->SaveFile(strXMLFileName.c_str());
    
    delete pDoc;
    pDoc = nullptr;
    return S_OK;
}

_int CLevel_AnimTool::Read_Bone(xml::XMLDocument* pDoc, aiNode* pCurNode, xml::XMLElement* pParentElement)
{
    xml::XMLElement* pBoneNodeElement = pDoc->NewElement("Bone");
    pBoneNodeElement->SetAttribute("Name", pCurNode->mName.C_Str());

    _int iCurIdx = (_int)m_BonesIndices.size();
    _int iParentIdx = (_int)(m_BonesIndices.size() - 1);

    // 인덱스를 구하기 위한 더미 데이터 추가
    m_BonesIndices.push_back(-1);

    pBoneNodeElement->SetAttribute("Index", iCurIdx);
    pBoneNodeElement->SetAttribute("ParentIndex", iParentIdx);
    pBoneNodeElement->SetAttribute("NumChild", pCurNode->mNumChildren);
        
    _int iNumTotChildBone = static_cast<_int>(pCurNode->mNumChildren);
    for (_int i = 0; i < static_cast<_int>(pCurNode->mNumChildren); ++i)
        iNumTotChildBone += Read_Bone(pDoc, pCurNode->mChildren[i], pBoneNodeElement);
    
    pBoneNodeElement->SetAttribute("NumTotalChildBone", iNumTotChildBone);
    pParentElement->LinkEndChild(pBoneNodeElement);

    return iNumTotChildBone;
}

HRESULT CLevel_AnimTool::XML_Load_BoneHierachy(const WCHAR* strXMLFilePath)
{
    xml::XMLDocument Doc;

    m_bBoneLoadSuccess = false;

    if (_waccess(strXMLFilePath, 0) == -1) 
    {
        std::wcout << L"File not found: " << strXMLFilePath << std::endl;
        return E_FAIL;
    }
    else 
    {
        std::wcout << L"File exists: " << strXMLFilePath << std::endl;
    }

    BONE_XML RootBone;
    string str = CONVERTTOSTRING(strXMLFilePath);

    if (Doc.LoadFile(str.c_str()) != xml::XML_SUCCESS)
    {
        std::cout << "Error: " << Doc.ErrorID() << " - " << Doc.ErrorStr() << std::endl;
        return E_FAIL;
    }

    xml::XMLElement* pStartNode = Doc.FirstChildElement("BoneHierachy"); // 루트 노드
    if (!pStartNode)
        return E_FAIL;

    xml::XMLElement* pRootEle = pStartNode->FirstChildElement("Bone");
    if (pRootEle)
    {
        XML_ParseBone_ReCursion(pRootEle, RootBone);
    }
    else
        return E_FAIL;
    
    m_RootBone = RootBone;
    m_bBoneLoadSuccess = true;

    Add_Log(LOG_OK, XML_LOAD, "XML_Load_BoneHierachy", CONVERTTOSTRING(strXMLFilePath));
    return S_OK;
}

HRESULT CLevel_AnimTool::XML_Load_RootMotionBoneNameData(const WCHAR* strXMLFilePath)
{
    xml::XMLDocument Doc;

    string str = CONVERTTOSTRING(strXMLFilePath);

    if (Doc.LoadFile(str.c_str()) != xml::XML_SUCCESS)
    {
        std::cout << "Error: " << Doc.ErrorID() << " - " << Doc.ErrorStr() << std::endl;
        MSG_BOX("Load Fail RootBone XML");
        return E_FAIL;
    }

    string strRootMotionBoneName{};

    xml::XMLElement* pEle = Doc.FirstChildElement("RootMotionBoneName");
    if (!pEle)
        return E_FAIL;

    strRootMotionBoneName = pEle->FindAttribute("Name")->Value();

    if (m_pAT_PartAnimObj_Model)
    {
        m_pAT_PartAnimObj_Model->Load_Anim_RootMotionBoneXML(strXMLFilePath);
        Add_Log(LOG_OK, XML_LOAD, "Show_Load_RootMotionBoneName_Btn", "load root motion bone name datas");
        return S_OK;
    }
    return E_FAIL;
}

HRESULT CLevel_AnimTool::XML_Save_RootMotionBoneNameData(const WCHAR* pXMLSaveFilePath, string strRootMotionboneName)
{
    xml::XMLDocument doc;
    xml::XMLDeclaration* pDecl = doc.NewDeclaration();
    doc.LinkEndChild(pDecl);
    XML_Add_Comment(&doc); // 날짜 추가

    xml::XMLElement* pEle = doc.NewElement("RootMotionBoneName");
    if (!pEle)
        return E_FAIL;

    pEle->SetAttribute("Name", strRootMotionboneName.c_str());
    doc.LinkEndChild(pEle);

    if (doc.SaveFile(CONVERTTOSTRING(pXMLSaveFilePath).c_str()) == xml::XML_SUCCESS)
    {
        MSG_BOX("RootMotionBoneName Save Succeess To AnimData Folder");
    }
    else
    {
        MSG_BOX("RootMotionBoneName Save Fail To AnimData Folder");
        return E_FAIL;
    }

    return S_OK;
}

void CLevel_AnimTool::XML_ParseBone_ReCursion(xml::XMLElement* pBoneEle, BONE_XML& Bone)
{
    // 재귀 탈줄 조건
    if (!pBoneEle)
        return;

    // 현재 노드 속성 읽기 (루트 부터 읽힌다)
    Bone.name = pBoneEle->Attribute("Name");
    pBoneEle->QueryIntAttribute("Index", &Bone.index);
    pBoneEle->QueryIntAttribute("ParentIndex", &Bone.parentIndex);
    pBoneEle->QueryIntAttribute("NumChild", &Bone.numChild);
    pBoneEle->QueryIntAttribute("NumTotalChildBone", &Bone.numTotalChildBone);

    m_BlendBones.push_back({ Bone.name, 0 });

    xml::XMLElement* pChildEle = pBoneEle->FirstChildElement("Bone");
    while (pChildEle)
    {
        BONE_XML ChildBone{};
        XML_ParseBone_ReCursion(pChildEle, ChildBone);
        Bone.children.push_back(ChildBone);
        pChildEle = pChildEle->NextSiblingElement("Bone");
    }
}

//// 뼈 정보를 저장하는 구조체
//typedef struct Bone
//{
//    string name;
//    int index;
//    int parentIndex;
//    int numChild;
//    int numTotalChildBone;
//    vector<Bone> children; // 자식 뼈를 저장하는 벡터
//
//    _bool bIsCheckedShow = false;
//    _bool bShowBonePos = false;
//
//} BONE_XML;

// Bone데이터 이전에 썻던 것으로 부터 재귀적으로 읽어서 저장한다.
void CLevel_AnimTool::XML_Save_BlendBones_ReCursion(xml::XMLDocument* pDoc, xml::XMLElement* pParentEle, BONE_XML& Bone, string strDstBoneName, _bool bFind, _int& iMinBoneIdx, _int& iMaxBoneIdx)
{

    xml::XMLElement* pChildNode = nullptr;

    if (bFind)
    {
        pChildNode = pDoc->NewElement("Bone");

        pChildNode->SetAttribute("Name", Bone.name.c_str());
        pChildNode->SetAttribute("Index", Bone.index);
        pChildNode->SetAttribute("ParentIndex", Bone.parentIndex);
        pChildNode->SetAttribute("NumChild", Bone.numChild);
        pChildNode->SetAttribute("NumTotalChildBone", Bone.numTotalChildBone);

        pParentEle->LinkEndChild(pChildNode); // 부모에 연결

        if (Bone.index < iMinBoneIdx)
            iMinBoneIdx = Bone.index;

        if (Bone.index > iMaxBoneIdx)
            iMaxBoneIdx = Bone.index;

        // recursion 탈줄 초건
        if (Bone.numChild == 0)
            return;
    }

    for (auto Element : Bone.children)
    {
        if (bFind)
            XML_Save_BlendBones_ReCursion(pDoc, pChildNode, Element, strDstBoneName, true, iMinBoneIdx, iMaxBoneIdx);
        else
        {
            if (Element.name == strDstBoneName)
                XML_Save_BlendBones_ReCursion(pDoc, pParentEle, Element, strDstBoneName, true, iMinBoneIdx, iMaxBoneIdx);
            else
                XML_Save_BlendBones_ReCursion(pDoc, pParentEle, Element, strDstBoneName, false, iMinBoneIdx, iMaxBoneIdx);
        }
    }
}

string CLevel_AnimTool::Show_StringInput(const _char* pLabel, const string& strDefaultVal)
{
    static _char Buffer[256]; // 입력받는 버퍼
    strncpy(Buffer, strDefaultVal.c_str(), sizeof(Buffer) - 1); // 초기값 설정
    Buffer[sizeof(Buffer) - 1] = '\0'; // 안전을 위한 null문자 추가

    // 문자열 입력 필드 표시
    if (ImGui::InputText(pLabel, Buffer, sizeof(Buffer)))
    {
        
    }

    return string(Buffer);
}

void CLevel_AnimTool::Show_MeshNames_ListBox()
{
    if (ImGui::TreeNode("Show-Mesh-Infos"))
    {
        ImGui::Text("Cur Model's Mesh Count : %d", m_pAT_PartAnimObj_Model->Get_NumMeshes());  // 현재 모델의 메쉬 개수 출력

        auto& Meshes = m_pAT_PartAnimObj_Model->Get_Meshes();

        // 메쉬 전체 개수를 위에서 출력하고 리스트 박스 형태로 메쉬 개수만큼 이름을 출력하고 선택할 수 있게한다.
        if (ImGui::BeginListBox("Meshes", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (_int i = 0; i < Meshes.size(); ++i)
                m_MeshNames.push_back(Meshes[i]->Get_MeshName());

            for (_int i = 0; i < Meshes.size(); ++i)
            {
                _bool IsSelected = (m_iCurMeshIdx == i);

                string strOutString = to_string(i) + " Mesh Name : " + m_MeshNames[i].c_str();
                if (ImGui::Selectable(strOutString.c_str(), IsSelected))
                    m_iCurMeshIdx = i; // 선택한 인덱스로 변경

                // 선택된 항목에 기본 포커스를 설정
                if (IsSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndListBox();
        }


        ImGui::TreePop();
    }
}

void CLevel_AnimTool::Show_LoadPartAnimModel_FBX_Radio()
{
    if (ImGui::TreeNode("Show-Load-PartObject"))
    {
        static _int iLoadFBXType = 0;
        ImGui::RadioButton("anim model", &iLoadFBXType, 0); ImGui::SameLine();
        ImGui::RadioButton("non anim model", &iLoadFBXType, 1);

        // 애니매이션이 들어간 모델 로드하는 경우 모달창 출력
        if (iLoadFBXType == 0)
        {
            if (ImGui::Button("Load Anim Model For Part Object"))
            {
                WCHAR strRetModelPath[256]; // 로드할 모델 최종 경로
                _wstring strModelPrototypeName = m_strModelPrototypeName + to_wstring(m_iModelNum++); // 모델 프로토 타입 이름

                if (Open_Modal_Window(m_strModelAbPath, m_strModelEx, strRetModelPath))
                    Load_AnimModel_FBX_File(strRetModelPath, ANIM_MODEL_FBX_LOAD_TYPE::ANIM_PART, strModelPrototypeName, TEXT("Prototype_GameObject_AnimToolObject"));
            }
        }
        else if (iLoadFBXType == 1)
        {
            if (ImGui::Button("Load NonAnim Model For Part Object"))
            {
            }
        }
        ImGui::TreePop();
    }
}

// 보간이 없는 2~3개의 애니매이션을 연속 재생하는 경우
void CLevel_AnimTool::Show_Loop_Default_AnimPlayTest()
{
    // Multi Default : 여러개의 애니매이션을 보간없이 연속 루프 재생하는 경우

    ImGui::SeparatorText("Loop Default Test");

    static _int iInterpolationTest = 0;
    ImGui::RadioButton("double anim default play test", &iInterpolationTest, 0); ImGui::SameLine();
    ImGui::RadioButton("third anim default play test", &iInterpolationTest, 1);

    if (iInterpolationTest == 0)
        Show_PlayAnimTest_MultiDefault_Second();    // 2개의 애니매이션을 보간없이 연속 재생
    else if (iInterpolationTest == 1)
        Show_PlayAnimTest_MultiDefault_Third();     // 3개의 애니매이션을 보간없이 연속 재생
}

// 애니매이션 플레이 테스트하는 함수
// - 보간 테스트나 여러 애니매이션 재생하는 경우 테스트 하는 함수이다.
void CLevel_AnimTool::Show_Loop_Interpolation_AnimPlayTest()
{
    // 파일 로드 없이 단순 애니매이션 테스트
    // src -> dst 애니매이션 보간 테스트
    // Default Play Test : 하나의 애니매이션을 루프로 재생하고 싶은 경우
    // Multi Default Play Test : 2개 이상의 애니매이션을 보간없이 재생하고 싶은 경우
    // Multi Interpolation Play Test : 2개 이상의 애니매이션을 보간을 하여 재생하고싶은 경우
    ImGui::SeparatorText("Loop Interpolation Test");

    static _int iInterpolationTest = 0;
    ImGui::RadioButton("double anim interpolation play test", &iInterpolationTest, 0); ImGui::SameLine();
    ImGui::RadioButton("third anim interpolation play test", &iInterpolationTest, 1);

    if (iInterpolationTest == 0)
        Show_PlayAnimTest_MultiInterpolation_Second();    // 2개의 애니매이션을 보간 연속 재생
    else if (iInterpolationTest == 1)
        Show_PlayAnimTest_MultiInterpolation_Third();     // 3개의 애니매이션을 보간 연속 재생

}

void CLevel_AnimTool::Show_Loop_Blending_AnimPlayTest()
{
    // 블렌딩 뼈 로드 한적이 없다면
    if (!m_bLoadSuccessBlendBones)
    {
        if (ImGui::Button("Load Blend Bones"))
        {
            if (FAILED(XML_Load_BlendBones()))
            {
                MSG_BOX("Load BlendBones Failed");
            }
        }
    }
    else
    {
        Show_BlendBone_ComboBox();
    }
}

void CLevel_AnimTool::Show_Blending_List(ExampleDualListBox& pArr, ANIM_BLEND_GROUP_TYPE eBlendType)
{
    _int iIdx = (_int)eBlendType;
    const static string strBlendType[(_int)ANIM_BLEND_GROUP_TYPE::BLEND_END]
        = { "None", "LowerBody", "UpperBody", "ArmLeft", "ArmRight", "Head", "Total" };

    ImGui::SeparatorText(strBlendType[iIdx].c_str());

    if (pArr.Items[0].Size == 0 && pArr.Items[1].Size == 0)
        for (int item_id = 0; item_id < m_AnimNamesCStr.size(); item_id++)
            pArr.Items[0].push_back((ImGuiID)item_id);

    // Show
    pArr.Show();
}

void CLevel_AnimTool::Show_BlendBone_ComboBox()
{
    const static string strBlendTypes[BLEND_TYPE_END] = { "One Blend", "Two Blend", "Three Blend", "Four Blend", "Five Blend" };
    static _int iCurBlendTypeIdx = 0;
    static _int iPrevBlendTypeIdx = 0;
    const _char* pCurBoneComboVal = strBlendTypes[iCurBlendTypeIdx].c_str();

    if (ImGui::BeginCombo("Blend Type", pCurBoneComboVal, 0))
    {
        for (_int i = 0; i < IM_ARRAYSIZE(strBlendTypes); i++)
        {
            const _bool is_selected = (iCurBlendTypeIdx == i);
            if (ImGui::Selectable(strBlendTypes[i].c_str(), is_selected))
            {
                iCurBlendTypeIdx = i;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    static ExampleDualListBox dlbs[(_int)ANIM_BLEND_GROUP_TYPE::BLEND_END];
    const static _int iStart = (_int)ANIM_BLEND_GROUP_TYPE::LOWER_BODY;
    const static _int iEnd = (_int)ANIM_BLEND_GROUP_TYPE::BLEND_END;

    for (_int i = iStart; i < iEnd; ++i)
        dlbs[i].m_AnimNamesCStr = m_AnimNamesCStr;

    if (iCurBlendTypeIdx != iPrevBlendTypeIdx)
        Move_All_Left(dlbs);

    const static _int iLowerBodyIdx = _int(ANIM_BLEND_GROUP_TYPE::LOWER_BODY);
    const static _int iUpperBodyIdx = _int(ANIM_BLEND_GROUP_TYPE::UPPER_BODY);
    const static _int iArmLeftIdx = _int(ANIM_BLEND_GROUP_TYPE::ARM_LEFT);
    const static _int iArmRightIdx = _int(ANIM_BLEND_GROUP_TYPE::ARM_RIGHT);
    const static _int iHeadIdx = _int(ANIM_BLEND_GROUP_TYPE::HEAD);
    const static _int iTotalIdx = _int(ANIM_BLEND_GROUP_TYPE::TOTAL);

    Show_Loop_Blend_Interpolation();

    // TOTAL만 보여준다.
    if (iCurBlendTypeIdx == BLEND_ONE)
    {
        const static _int idx = (_int)ANIM_BLEND_GROUP_TYPE::TOTAL;
        Show_Blending_List(dlbs[idx], (ANIM_BLEND_GROUP_TYPE)(idx));

        if (dlbs[idx].GetBlendItemName() != "None")
        {
            static string strCurAnimName = dlbs[idx].GetBlendItemName();
            strCurAnimName = dlbs[idx].GetBlendItemName();
            static string strPrevAnimName = "";
            
            if (strCurAnimName != strPrevAnimName)
            {
                for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                    m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                m_pAT_PartAnimObj_Model->Set_BlendGroup(make_pair(ANIM_BLEND_GROUP_TYPE(idx), m_pAT_PartAnimObj_Model->Get_AnimIdx(dlbs[idx].GetBlendItemName())));
            }

            strPrevAnimName = strCurAnimName;

        }
    }
    // 상체 하체 보여준다.
    else if (iCurBlendTypeIdx == BLEND_TWO)
    {       
        const static _int sidx = _int(ANIM_BLEND_GROUP_TYPE::LOWER_BODY);
        const static _int eidx = _int(ANIM_BLEND_GROUP_TYPE::UPPER_BODY);

        for (_int i = sidx; i <= eidx; ++i)
            Show_Blending_List(dlbs[i], (ANIM_BLEND_GROUP_TYPE)(i));

        vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecCurBlendAnims;
        static vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecPrevBlendAnims;

        for (_int i = sidx; i <= eidx; ++i)
        {
            if (dlbs[i].GetBlendItemName() == "None")
                continue;

            vecCurBlendAnims.push_back({ ANIM_BLEND_GROUP_TYPE(i), m_pAT_PartAnimObj_Model->Get_AnimIdx(dlbs[i].GetBlendItemName()) });
        }

        if (vecCurBlendAnims.size() == 2)
        {
            if (vecPrevBlendAnims.empty())
            {
                for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                    m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1]);
            }
            else
            {
                if (vecPrevBlendAnims[0] != vecCurBlendAnims[0] || vecPrevBlendAnims[1] != vecCurBlendAnims[1])
                {
                    for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                        m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                    m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1]);
                }
            }

            vecPrevBlendAnims = vecCurBlendAnims;
            vecCurBlendAnims.clear();

        }
    }
    // 하체, 상체, {오른손, 왼손, 모가지} 택 1
    else if (iCurBlendTypeIdx == BLEND_THREE)
    {
        enum THREE_BLEND_TYPE { SELECT, TYPE_LUR, TYPE_LUL, TYPE_LUH };
        const static string strThreeBlendType[4] = 
        { 
            "Select",
            "LowerBody + UpperBody + ArmRight",
            "LowerBody + UpperBody + LeftRight",
            "LowerBody + UpperBody + Head" 
        };

        static _int iThreeBlendTypeIdx = 0;
        const _char* pCurThreeBlendType = strThreeBlendType[iThreeBlendTypeIdx].c_str();
        if (ImGui::BeginCombo("Three Blend Type", pCurThreeBlendType, 0))
        {
            for (_int i = 0; i < IM_ARRAYSIZE(strThreeBlendType); i++)
            {
                const _bool is_selected = (iThreeBlendTypeIdx == i);
                if (ImGui::Selectable(strThreeBlendType[i].c_str(), is_selected))
                {
                    iThreeBlendTypeIdx = i;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        
        if (iThreeBlendTypeIdx != SELECT)
        {
            // LOWER, UPPER, ARM_RIGHT
            if (iThreeBlendTypeIdx == TYPE_LUR)
            {
                Show_Blending_List(dlbs[iLowerBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iLowerBodyIdx));
                Show_Blending_List(dlbs[iUpperBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iUpperBodyIdx));
                Show_Blending_List(dlbs[iArmRightIdx], (ANIM_BLEND_GROUP_TYPE)(iArmRightIdx));

                vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecCurBlendAnims;
                static vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecPrevBlendAnims;

                for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                {
                    if ((i == iLowerBodyIdx) || (i == iUpperBodyIdx) || (i == iArmRightIdx))
                    {
                        if (dlbs[i].GetBlendItemName() == "None")
                            continue;

                        vecCurBlendAnims.push_back({ ANIM_BLEND_GROUP_TYPE(i), m_pAT_PartAnimObj_Model->Get_AnimIdx(dlbs[i].GetBlendItemName()) });
                    }
                }

                if (vecCurBlendAnims.size() == 3)
                {
                    if (vecPrevBlendAnims.empty())
                    {
                        for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                            m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                        m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2]);
                    }
                    else
                    {
                        if (vecPrevBlendAnims[0] != vecCurBlendAnims[0] || vecPrevBlendAnims[1] != vecCurBlendAnims[1] || vecPrevBlendAnims[2] != vecCurBlendAnims[2])
                        {
                            for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                            m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2]);
                        }
                    }

                    vecPrevBlendAnims = vecCurBlendAnims;
                    vecCurBlendAnims.clear();
                }
            }
            // lowerbody, upperbody, ArmLeft
            else if (iThreeBlendTypeIdx == TYPE_LUL)
            {
                Show_Blending_List(dlbs[iLowerBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iLowerBodyIdx));
                Show_Blending_List(dlbs[iUpperBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iUpperBodyIdx));
                Show_Blending_List(dlbs[iArmLeftIdx], (ANIM_BLEND_GROUP_TYPE)(iArmLeftIdx));

                vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecCurBlendAnims;
                static vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecPrevBlendAnims;

                for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                {
                    if ((i == iLowerBodyIdx) || (i == iUpperBodyIdx) || (i == iArmLeftIdx))
                    {
                        if (dlbs[i].GetBlendItemName() == "None")
                            continue;
                        vecCurBlendAnims.push_back({ ANIM_BLEND_GROUP_TYPE(i), m_pAT_PartAnimObj_Model->Get_AnimIdx(dlbs[i].GetBlendItemName()) });
                    }
                }

                if (vecCurBlendAnims.size() == 3)
                {
                    if (vecPrevBlendAnims.empty())
                    {
                        for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                            m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                        m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2]);
                    }
                    else
                    {
                        if (vecPrevBlendAnims[0] != vecCurBlendAnims[0] || vecPrevBlendAnims[1] != vecCurBlendAnims[1] || vecPrevBlendAnims[2] != vecCurBlendAnims[2])
                        {
                            for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                            m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2]);
                        }
                    }

                    vecPrevBlendAnims = vecCurBlendAnims;
                    vecCurBlendAnims.clear();
                }
            }
            // lowerbody, upperbody, head
            else if (iThreeBlendTypeIdx == TYPE_LUH)
            {
                Show_Blending_List(dlbs[iLowerBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iLowerBodyIdx));
                Show_Blending_List(dlbs[iUpperBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iUpperBodyIdx));
                Show_Blending_List(dlbs[iHeadIdx], (ANIM_BLEND_GROUP_TYPE)(iHeadIdx));

                vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecCurBlendAnims;
                static vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecPrevBlendAnims;

                for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                {
                    if ((i == iLowerBodyIdx) || (i == iUpperBodyIdx) || (i == iHeadIdx))
                    {
                        if (dlbs[i].GetBlendItemName() == "None")
                            continue;
                        vecCurBlendAnims.push_back({ ANIM_BLEND_GROUP_TYPE(i), m_pAT_PartAnimObj_Model->Get_AnimIdx(dlbs[i].GetBlendItemName()) });
                    }
                }

                if (vecCurBlendAnims.size() == 3)
                {
                    if (vecPrevBlendAnims.empty())
                    {
                        for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                            m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                        m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2]);
                    }
                    else
                    {
                        if (vecPrevBlendAnims[0] != vecCurBlendAnims[0] || vecPrevBlendAnims[1] != vecCurBlendAnims[1] || vecPrevBlendAnims[2] != vecCurBlendAnims[2])
                        {
                            for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                            m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2]);
                        }
                    }

                    vecPrevBlendAnims = vecCurBlendAnims;
                    vecCurBlendAnims.clear();
                }
            }
        }
    }
    // 하체, 상체, { {오른손, 왼손}, {오른손, 모가지}, {왼손, 모가지} }
    else if (iCurBlendTypeIdx == BLEND_FOUR)
    {
        enum FOUR_BLEND_TYPE { SELECT, TYPE_LULR, TYPE_LULH, TYPE_LURH };
        const static string strFourBlendType[4] = 
        { 
            "Select",
            "LowerBody + UpperBody + ArmLeft + ArmRight", 
            "LowerBody + UpperBody + ArmLeft + Head",
            "LowerBody + UpperBody + ArmRight + Head" 
        };

        static _int iFourBlendTypeIdx = 0;
        const _char* pCurFourBlendType = strFourBlendType[iFourBlendTypeIdx].c_str();
        if (ImGui::BeginCombo("Three Blend Type", pCurFourBlendType, 0))
        {
            for (_int i = 0; i < IM_ARRAYSIZE(strFourBlendType); i++)
            {
                const _bool is_selected = (iFourBlendTypeIdx == i);
                if (ImGui::Selectable(strFourBlendType[i].c_str(), is_selected))
                {
                    iFourBlendTypeIdx = i;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (iFourBlendTypeIdx != SELECT)
        {
            // LowerBody UpperBody ArmLeft, ArmRight
            if (iFourBlendTypeIdx == TYPE_LULR)
            {
                Show_Blending_List(dlbs[iLowerBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iLowerBodyIdx));
                Show_Blending_List(dlbs[iUpperBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iUpperBodyIdx));
                Show_Blending_List(dlbs[iArmLeftIdx], (ANIM_BLEND_GROUP_TYPE)(iArmLeftIdx));
                Show_Blending_List(dlbs[iArmRightIdx], (ANIM_BLEND_GROUP_TYPE)(iArmRightIdx));

                vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecCurBlendAnims;
                static vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecPrevBlendAnims;

                for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                {
                    if ((i == iLowerBodyIdx) || (i == iUpperBodyIdx) || (i == iArmLeftIdx) || (i == iArmRightIdx))
                    {
                        if (dlbs[i].GetBlendItemName() == "None")
                            continue;
                        vecCurBlendAnims.push_back({ ANIM_BLEND_GROUP_TYPE(i), m_pAT_PartAnimObj_Model->Get_AnimIdx(dlbs[i].GetBlendItemName()) });
                    }
                }

                if (vecCurBlendAnims.size() == 4)
                {
                    if (vecPrevBlendAnims.empty())
                    {
                        for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                            m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                        m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2], vecCurBlendAnims[3]);
                    }
                    else
                    {
                        if (vecPrevBlendAnims[0] != vecCurBlendAnims[0] || vecPrevBlendAnims[1] != vecCurBlendAnims[1] || vecPrevBlendAnims[2] != vecCurBlendAnims[2] || vecPrevBlendAnims[3] != vecCurBlendAnims[3])
                        {
                            for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                            m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2], vecCurBlendAnims[3]);
                        }
                    }

                    vecPrevBlendAnims = vecCurBlendAnims;
                    vecCurBlendAnims.clear();
                }
            }
            // Lower Upper ArmLeft Head
            else if (iFourBlendTypeIdx == TYPE_LULH)
            {
                Show_Blending_List(dlbs[iLowerBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iLowerBodyIdx));
                Show_Blending_List(dlbs[iUpperBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iUpperBodyIdx));
                Show_Blending_List(dlbs[iArmLeftIdx], (ANIM_BLEND_GROUP_TYPE)(iArmLeftIdx));
                Show_Blending_List(dlbs[iHeadIdx], (ANIM_BLEND_GROUP_TYPE)(iHeadIdx));

                vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecCurBlendAnims;
                static vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecPrevBlendAnims;

                for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                {
                    if ((i == iLowerBodyIdx) || (i == iUpperBodyIdx) || (i == iArmLeftIdx) || (i == iHeadIdx))
                    {
                        if (dlbs[i].GetBlendItemName() == "None")
                            continue;
                        vecCurBlendAnims.push_back({ ANIM_BLEND_GROUP_TYPE(i), m_pAT_PartAnimObj_Model->Get_AnimIdx(dlbs[i].GetBlendItemName()) });
                    }
                }

                if (vecCurBlendAnims.size() == 4)
                {
                    if (vecPrevBlendAnims.empty())
                    {
                        for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                            m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                        m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2], vecCurBlendAnims[3]);
                    }
                    else
                    {
                        if (vecPrevBlendAnims[0] != vecCurBlendAnims[0] || vecPrevBlendAnims[1] != vecCurBlendAnims[1] || vecPrevBlendAnims[2] != vecCurBlendAnims[2] || vecPrevBlendAnims[3] != vecCurBlendAnims[3])
                        {
                            for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                            m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2], vecCurBlendAnims[3]);
                        }
                    }

                    vecPrevBlendAnims = vecCurBlendAnims;
                    vecCurBlendAnims.clear();
                }

            }
            // Lower Upper ArmRight Head
            else if (iFourBlendTypeIdx == TYPE_LURH)
            {
                Show_Blending_List(dlbs[iLowerBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iLowerBodyIdx));
                Show_Blending_List(dlbs[iUpperBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iUpperBodyIdx));
                Show_Blending_List(dlbs[iArmRightIdx], (ANIM_BLEND_GROUP_TYPE)(iArmRightIdx));
                Show_Blending_List(dlbs[iHeadIdx], (ANIM_BLEND_GROUP_TYPE)(iHeadIdx));

                vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecCurBlendAnims;
                static vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecPrevBlendAnims;

                for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                {
                    if ((i == iLowerBodyIdx) || (i == iUpperBodyIdx) || (i == iArmRightIdx) || (i == iHeadIdx))
                    {
                        if (dlbs[i].GetBlendItemName() == "None")
                            continue;
                        vecCurBlendAnims.push_back({ ANIM_BLEND_GROUP_TYPE(i), m_pAT_PartAnimObj_Model->Get_AnimIdx(dlbs[i].GetBlendItemName()) });
                    }
                }

                if (vecCurBlendAnims.size() == 4)
                {
                    if (vecPrevBlendAnims.empty())
                    {
                        for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                            m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                        m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2], vecCurBlendAnims[3]);
                    }
                    else
                    {
                        if (vecPrevBlendAnims[0] != vecCurBlendAnims[0] || vecPrevBlendAnims[1] != vecCurBlendAnims[1] || vecPrevBlendAnims[2] != vecCurBlendAnims[2] || vecPrevBlendAnims[3] != vecCurBlendAnims[3])
                        {
                            for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                                m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                            m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2], vecCurBlendAnims[3]);
                        }
                    }

                    vecPrevBlendAnims = vecCurBlendAnims;
                    vecCurBlendAnims.clear();
                }
            }
        }
        
    }
    // 전부다 TOTAL 뺴고
    else if (iCurBlendTypeIdx == BLEND_FIVE)
    {
        Show_Blending_List(dlbs[iLowerBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iLowerBodyIdx));
        Show_Blending_List(dlbs[iUpperBodyIdx], (ANIM_BLEND_GROUP_TYPE)(iUpperBodyIdx));
        Show_Blending_List(dlbs[iArmLeftIdx], (ANIM_BLEND_GROUP_TYPE)(iArmLeftIdx));
        Show_Blending_List(dlbs[iArmRightIdx], (ANIM_BLEND_GROUP_TYPE)(iArmRightIdx));
        Show_Blending_List(dlbs[iHeadIdx], (ANIM_BLEND_GROUP_TYPE)(iHeadIdx));

        vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecCurBlendAnims;
        static vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecPrevBlendAnims;

        for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
        {
            if ((i == iLowerBodyIdx) || (i == iUpperBodyIdx) || (i == iArmLeftIdx) || (i == iArmRightIdx) || (i == iHeadIdx))
            {
                if (dlbs[i].GetBlendItemName() == "None")
                    continue;
                vecCurBlendAnims.push_back({ ANIM_BLEND_GROUP_TYPE(i), m_pAT_PartAnimObj_Model->Get_AnimIdx(dlbs[i].GetBlendItemName()) });
            }
        }

        if (vecCurBlendAnims.size() == 5)
        {
            if (vecPrevBlendAnims.empty())
            {
                for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                    m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2], vecCurBlendAnims[3], vecCurBlendAnims[4]);
            }
            else
            {
                if (vecPrevBlendAnims[0] != vecCurBlendAnims[0] || vecPrevBlendAnims[1] != vecCurBlendAnims[1] || vecPrevBlendAnims[2] != vecCurBlendAnims[2] || vecPrevBlendAnims[3] != vecCurBlendAnims[3] || vecPrevBlendAnims[4] != vecCurBlendAnims[4])
                {
                    for (_int i = iLowerBodyIdx; i <= iTotalIdx; ++i)
                        m_pAT_PartAnimObj_Model->Reset_BlendGroupBone((ANIM_BLEND_GROUP_TYPE)i);

                    m_pAT_PartAnimObj_Model->Set_BlendGroup(vecCurBlendAnims[0], vecCurBlendAnims[1], vecCurBlendAnims[2], vecCurBlendAnims[3], vecCurBlendAnims[4]);
                }
            }

            vecPrevBlendAnims = vecCurBlendAnims;
            vecCurBlendAnims.clear();
        }
    }


    iPrevBlendTypeIdx = iCurBlendTypeIdx;
}

void CLevel_AnimTool::Move_All_Left(ExampleDualListBox Arr[])
{
    for (_int i = 1; i <= (_int)ANIM_BLEND_GROUP_TYPE::TOTAL; ++i)
        Arr[i].MoveAll(1, 0);
}

void CLevel_AnimTool::Show_Loop_Blend_Interpolation()
{
    // UI 렌더링 호출
    ImGui::Begin("Animation Selector");

    // 창 크기를 초과하는 콘텐츠를 스크롤 가능하도록 설정
    ImGui::BeginChild("ScrollableRegion", ImVec2(0, 0), true, ImGuiWindowFlags_None);


    _int iSize = m_AnimNamesCStr.size();
    static vector<tuple<string, ANIM_BLEND_GROUP_TYPE, _int>> vecSelectList(iSize, make_tuple("None", ANIM_BLEND_GROUP_TYPE::NONE, 0));
 
    const _int iSizeBlendType = 9;
    const static string strBlendType[iSizeBlendType] = { "None", "LowerBody", "UpperBody", "ArmLeft", "ArmRight", "Head", "Total", "Lower Upper", "Upper Lower" };

    // 창의 가용 영역 크기 가져오기
    float totalWidth = ImGui::GetContentRegionAvail().x;
    float bulletTextWidth = totalWidth * 0.5f;   // BulletText는 50% 크기
    float comboBoxWidth = totalWidth * 0.45f;   // ComboBox는 45% 크기
    float spacing = totalWidth * 0.05f;         // 두 컴포넌트 사이 간격

    ImGui::BeginGroup(); // 그룹 시작 (가로 배치를 위한 그룹)

    if (ImGui::Button("Start Interpolation Each Part"))
    {
        vector<pair<ANIM_BLEND_GROUP_TYPE, _int>> vecBlendGroups;
        for (_int i = 0; i < vecSelectList.size(); ++i)
        {
            if (get<0>(vecSelectList[i]) != "None")
            {
                vecBlendGroups.push_back(make_pair(get<1>(vecSelectList[i]), get<2>(vecSelectList[i])));
            }
        }

        if (!vecBlendGroups.empty())
        {
            if (vecBlendGroups.size() == 1)
            {
                ANIM_BLEND_GROUP_TYPE eType = vecBlendGroups[0].first;
                switch (eType)
                {
                case Engine::ANIM_BLEND_GROUP_TYPE::TOTAL:
                {
                    m_pAT_PartAnimObj_Model->Change_Anim(make_pair(vecBlendGroups[0].first, vecBlendGroups[0].second));
                }
                    break;
                case Engine::ANIM_BLEND_GROUP_TYPE::LOWERUPPER:
                {
                    m_pAT_PartAnimObj_Model->Change_Anim(make_pair(ANIM_BLEND_GROUP_TYPE::LOWER_BODY, vecBlendGroups[0].second), make_pair(ANIM_BLEND_GROUP_TYPE::UPPER_BODY, vecBlendGroups[0].second));
                }
                break;
                case Engine::ANIM_BLEND_GROUP_TYPE::UPPERLOWER:
                {
                    m_pAT_PartAnimObj_Model->Change_Anim(make_pair(ANIM_BLEND_GROUP_TYPE::UPPER_BODY, vecBlendGroups[0].second), make_pair(ANIM_BLEND_GROUP_TYPE::LOWER_BODY, vecBlendGroups[0].second));
                }
                    break;
                default:
                    break;
                }

            }
            else if (vecBlendGroups.size() == 2)
                m_pAT_PartAnimObj_Model->Change_Anim(make_pair(vecBlendGroups[0].first, vecBlendGroups[0].second), make_pair(vecBlendGroups[1].first, vecBlendGroups[1].second));
            else if (vecBlendGroups.size() == 3)
                m_pAT_PartAnimObj_Model->Change_Anim(make_pair(vecBlendGroups[0].first, vecBlendGroups[0].second), make_pair(vecBlendGroups[1].first, vecBlendGroups[1].second), make_pair(vecBlendGroups[2].first, vecBlendGroups[2].second));
            else if (vecBlendGroups.size() == 4)
                m_pAT_PartAnimObj_Model->Change_Anim(make_pair(vecBlendGroups[0].first, vecBlendGroups[0].second), make_pair(vecBlendGroups[1].first, vecBlendGroups[1].second), make_pair(vecBlendGroups[2].first, vecBlendGroups[2].second), make_pair(vecBlendGroups[3].first, vecBlendGroups[3].second));
            else if (vecBlendGroups.size() == 5)
                m_pAT_PartAnimObj_Model->Change_Anim(make_pair(vecBlendGroups[0].first, vecBlendGroups[0].second), make_pair(vecBlendGroups[1].first, vecBlendGroups[1].second), make_pair(vecBlendGroups[2].first, vecBlendGroups[2].second), make_pair(vecBlendGroups[3].first, vecBlendGroups[3].second), make_pair(vecBlendGroups[4].first, vecBlendGroups[4].second));
            
        }
    }

    // 보간값 설정
    {

        // 보간에 사용할 데이터 타입 : XML과 직접 설정한 데이터 중 고른다.
        static const _char* pAnimPlayTestTypes[] = { "Select", "Use Custum Data" };

        static _int iSelectedAnimPlayTestDataTypeIdx = 0;
        const _char* pComboSelectedAnimPlayTestDataTypeName = pAnimPlayTestTypes[iSelectedAnimPlayTestDataTypeIdx];

        if (ImGui::BeginCombo("anim play test data type combo", pComboSelectedAnimPlayTestDataTypeName))
        {
            for (int n = 0; n < IM_ARRAYSIZE(pAnimPlayTestTypes); n++)
            {
                const bool is_selected = (iSelectedAnimPlayTestDataTypeIdx == n);
                if (ImGui::Selectable(pAnimPlayTestTypes[n], is_selected))
                {
                    iSelectedAnimPlayTestDataTypeIdx = n;

                    // XML 데이터 사용하는 경우
                    if (iSelectedAnimPlayTestDataTypeIdx == 0)
                    {
                        m_pAT_PartAnimObj_Model->Set_UseCustumInterData(false);

                    }
                    // 커스텀 데이터 사용하는 경우
                    else if (iSelectedAnimPlayTestDataTypeIdx == 1)
                    {
                        m_pAT_PartAnimObj_Model->Set_UseCustumInterData(true);

                    }
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (iSelectedAnimPlayTestDataTypeIdx == 1)
            Show_Interpolation_UseCustumData();

        ImGui::Spacing();
        ImGui::EndGroup();
    }

    ImGui::SeparatorText("");


    for (_int i = 0; i < iSize; ++i)
    {
        // BulletText와 ComboBox를 한 줄에 배치
        ImGui::PushID(i); // PushID로 고유 ID 생성

        // BulletText: 왼쪽
        ImGui::SetNextItemWidth(bulletTextWidth);
        string strName = "Idx : " + to_string(i) + " " + m_AnimNamesCStr[i];
        ImGui::BulletText("%s", strName.c_str());

        // ComboBox: 오른쪽
        ImGui::SameLine(bulletTextWidth + spacing); // BulletText 오른쪽에 배치
        ImGui::SetNextItemWidth(comboBoxWidth);

        // 현재 선택된 블렌드 타입
        _int currentSelectedIndex = static_cast<_int>(get<1>(vecSelectList[i]));
        const char* currentComboValue = strBlendType[currentSelectedIndex].c_str();

        // 콤보 박스 시작
        if (ImGui::BeginCombo(("##BlendCombo" + to_string(i)).c_str(), currentComboValue, 0))
        {
            for (_int n = 0; n < iSizeBlendType; ++n)
            {
                const bool isSelected = (currentSelectedIndex == n);

                // 선택 항목 렌더링
                if (ImGui::Selectable(strBlendType[n].c_str(), isSelected))
                {
                    // 선택된 항목 업데이트
                    currentSelectedIndex = n;

                    if (n == 0)
                    {
                        get<1>(vecSelectList[i]) = ANIM_BLEND_GROUP_TYPE(n);
                        get<0>(vecSelectList[i]) = "None";
                    }
                    else
                    {
                        get<1>(vecSelectList[i]) = ANIM_BLEND_GROUP_TYPE(n);
                        get<0>(vecSelectList[i]) = strBlendType[currentSelectedIndex];
                        get<2>(vecSelectList[i]) = i;
                    }
                }

                // 현재 선택된 항목 포커스
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::PopID(); // PushID와 쌍으로 사용
    }

    ImGui::EndChild(); // End the scrollable region
    ImGui::End();
}

//void CLevel_AnimTool::Show_Loop_Blend_Interpolation_Data()
//{
//    const static _float fCustumInterDisStand = 0.2f;
//    const static _float fCustumInterSPStand = 1.f;
//    static _float fCustumInterDis = 0.2f;
//    static _float fCustumInterSP = 1.f;
//
//    ImGui::InputFloat("Custum Inter Dis Input", &fCustumInterDis, 0.01f);
//    ImGui::DragFloat("Cumstum Inter Dis Drag", &fCustumInterDis, 0.01f, 0.01f, 5.f);
//
//    ImGui::InputFloat("Custum Inter SP Input", &fCustumInterSP, 0.01f);
//    ImGui::DragFloat("Cumstum Inter SP Drag", &fCustumInterSP, 0.01f, 0.1f, 5.f);
//
//    if (fCustumInterDis != fCustumInterDisStand)
//    {
//        m_pAT_PartAnimObj_Model->Set_CustumInterDis(fCustumInterDis);
//    }
//
//    if (fCustumInterSP != fCustumInterSPStand)
//    {
//        m_pAT_PartAnimObj_Model->Set_CustumInterSP(fCustumInterSP);
//    }
//}

void CLevel_AnimTool::MenuItem_Save_BoneHierachy()
{
    static _bool bShowSaveBonehierachyToXML = false;

    if (ImGui::MenuItem("Save Anim Bone Hierarchy XML", "Ctrl + S + B"))
    {
        if (!m_pAT_Obj)
        {
            MSG_BOX("현재 로드된 fbx모델이 없습니다. fbx부터 로드하세요 (Ctrl + O)");
            return;
        }

        bShowSaveBonehierachyToXML = true;
    }

    // Bone 계층구조 XML로 저장
    if (bShowSaveBonehierachyToXML)
    {
        WCHAR strRetFilePath[256];
        if (Open_Save_Modal("xml", strRetFilePath))
        {
            fs::path FilePath = strRetFilePath;
            fs::path FileName = FilePath.stem();
            string strFileName = FileName.filename().string();

            if (FAILED(XML_Save_Bone_Hierachy(CONVERTTOSTRING(strRetFilePath), strFileName)))
            {
                MSG_BOX("Save Bone Hierachy Faile");
            }
            else
            {
                MSG_BOX("Save Bone Hierachy Success");
                Add_Log(LOG_OK, XML_SAVE, "MenuItem_Save_BoneHierachy", CONVERTTOSTRING(strRetFilePath));
            }

            bShowSaveBonehierachyToXML = 0;
        }
    }
}

_bool CLevel_AnimTool::Show_LoadBoneData_Btn()
{
    ImGui::SeparatorText("Load XML Bone Data");

    if (ImGui::Button("Load XML BoneData"))
    {
        WCHAR strFilePath[256];
        if (Open_Modal("xml", strFilePath))
        {
            if (FAILED(XML_Load_BoneHierachy(strFilePath)))
            {
                Add_Log(LOG_FAIL, XML_LOAD, "Show_LoadBoneData_Btn", "load from : " + CONVERTTOSTRING(strFilePath));
                return false;
            }
            else
            {
                Add_Log(LOG_OK, XML_LOAD, "Show_LoadBoneData_Btn", "load from : " + CONVERTTOSTRING(strFilePath));
                return true;
            }
        }
    }

    return true;
}

_bool CLevel_AnimTool::Show_Load_RootMotionBoneName_Btn()
{
    if (ImGui::Button("Load XML RootMotionBoneName Data"))
    {
        WCHAR strFilePath[256];
        if (Open_Modal("xml", strFilePath))
            if (FAILED(XML_Load_RootMotionBoneNameData(strFilePath)))
                return false;

        Add_Log(LOG_OK, XML_LOAD, "Show_Load_RootMotionBoneName_Btn", "load root motion bone name datas");
    }

    return true;
}

_bool CLevel_AnimTool::Begin_Show_BoneHierarchy()
{
    static _bool bOpenBoneHierarchy = true;

    if (bOpenBoneHierarchy)
    {
        ImGui::Begin("BoneHierarchy", &bOpenBoneHierarchy, ImGuiWindowFlags_MenuBar | ImGuiTableFlags_SizingStretchProp);

        // 디버그용 보여주는 뼈(사각형 객체)삭제 및 선택한 뼈이름 삭제
        m_bBeginBoneHierarchy[AT_BONE_HIERARCHY_TYPE::BH_CLEAR_SELECTED_SHOWBONES] = Show_Clear_AllSelected_ShowBones_Btn(); 
        
        // 블렌딩할 뼈 정보 저장하는 버튼 함수 (TODO : 아직 기능 없음, 250125 80%완)
        m_bBeginBoneHierarchy[AT_BONE_HIERARCHY_TYPE::BH_BLEND_BONE] = Show_Save_BlendBone_Btn();

        // 뼈의 구조를 재귀적으로 호출해서 렌더링 한다.
        ImGui::SeparatorText("Bone Hierachy");

        string strBoneName = Show_BoneInfo_Recur_TreeNode(m_RootBone); // 뼈 구조 재귀 호출해서 imgui에 계층적으로 렌더링
        m_bBeginBoneHierarchy[AT_BONE_HIERARCHY_TYPE::BH_SAVE_ROOTBONE] = XML_Show_SaveRootMotionBone_Btn(strBoneName);
        if (!m_bBeginBoneHierarchy[AT_BONE_HIERARCHY_TYPE::BH_SAVE_ROOTBONE])
        {
            MSG_BOX("Failed XML_Show_SaveRootMotionBone_Btn");
        }

        ImGui::End();
    }
    else
    {
        // 초기화
        // - 자식이 있다면 싹다 밀어버린다.
        if (!m_RootBone.children.empty())
            Clear_LoadBoneInfo(m_RootBone);

        bOpenBoneHierarchy = true;
        m_bBoneLoadSuccess = false;
    }

    return true;
}

void CLevel_AnimTool::Clear_LoadBoneInfo(BONE_XML& Bone)
{
    if (Bone.children.empty())
        return;

    for (_int i = 0; i < Bone.children.size(); ++i)
    {
        Clear_LoadBoneInfo(Bone.children[i]);
    }

    Bone.children.clear();
}

_bool CLevel_AnimTool::Show_Clear_AllSelected_ShowBones_Btn()
{
    if (ImGui::Button("Clear all selected show bones"))
    {
        if (!m_SelectedShowBonesNames.empty())
        {
            for (auto iterFirst = m_SelectedShowBonesNames.begin(); iterFirst != m_SelectedShowBonesNames.end();)
            {
                auto iterSecond = m_SelectedShowBoneSquare.find(*iterFirst);
                if (iterSecond != m_SelectedShowBoneSquare.end())
                {
                    if (Remove_ShowboneSquare(*iterFirst) == S_OK)
                        iterFirst = m_SelectedShowBonesNames.erase(iterFirst);
                    else
                        MSG_BOX("Clear All Show Bones Failed");
                }
                else
                    MSG_BOX("메모리 누수!!!");
            }

            Add_Log(LOG_OK, CLOSE, "Show_Clear_AllSelected_ShowBones_Btn", "clear m_ShowBonesNames & m_ShowBoneSquare");
        }
        else
        {
            Add_Log(LOG_OK, CLOSE, "Show_Clear_AllSelected_ShowBones_Btn", "empty m_ShowBonesNames");
        }
    }

    ImGui::SameLine();
    HelpMarker("remove all selected show bonse what you selected");

    return true;
}

_bool CLevel_AnimTool::Show_Save_BlendBone_Btn()
{
    // return true;

    // 블렌딩 뼈 정보 저장하는 함수 (상하체에 사용 할 함수)
    if (FAILED(XML_Save_Selected_BlendBones_Btn()))
    {
        MSG_BOX("Failed Save Selected Show Bones");
    }

    return true;
}

string CLevel_AnimTool::Show_BoneInfo_Recur_TreeNode(BONE_XML& Bone)
{
    if (Bone.children.empty()) // 비어있다면 탈출한다.
        return "";

    static string strSelectedBoneName = "";

    string strTreeNodeName = "Infos : index : " + to_string(Bone.index) + " | " + "ParentIndex : " + to_string(Bone.parentIndex) + " | " + "NumChild : " + to_string(Bone.numChild) + " | " + "NumTotalChildBone : " + to_string(Bone.numTotalChildBone);

    if (ImGui::TreeNode(Bone.name.c_str()))
    {
        ImGui::Text(strTreeNodeName.c_str()); 

        // 어떤 뼈를 루트 본으로 설정할지 체크하는 박스
        {
            _bool bIsChecked = (strSelectedBoneName == Bone.name); // 현재 선택된 뼈와 비교
            if (ImGui::Checkbox("check this bone as root motion", &bIsChecked))
            {
                if (bIsChecked)
                    strSelectedBoneName = Bone.name;
                else
                    strSelectedBoneName = "";
            }
        }

        // 어떤 뼈를 보여줄지 체크하는 박스
        {
            if (Bone.bShowBonePos)
            {
                _bool bSame = false;
                for (_int i = 0; i < m_SelectedShowBonesNames.size(); ++i)
                {
                    if (Bone.name == m_SelectedShowBonesNames[i])
                        bSame = true;
                }

                if (!bSame) // 찾은게 없다면
                {
                    Bone.bShowBonePos = false;
                }
            }



 

            
            //// 현재 커서 위치를 가져옴
            //ImVec2 cursorPos = ImGui::GetCursorScreenPos();

            //// 아이콘을 그리기 위한 DrawList 가져오기
            //ImDrawList* drawList = ImGui::GetWindowDrawList();

            //// 네모 그리기
            //drawList->AddRectFilled(
            //    ImVec2(cursorPos.x, cursorPos.y),                       // 네모의 왼쪽 위 좌표
            //    ImVec2(cursorPos.x + iconSize, cursorPos.y + iconSize), // 네모의 오른쪽 아래 좌표
            //    ImColor(icons[i].color)                                // 색상
            //);

            //// 네모 오른쪽으로 텍스트 출력
            //ImGui::Dummy(ImVec2(iconSize, iconSize)); // 네모의 크기만큼 공간 확보
            //ImGui::SameLine();
            //ImGui::Text("%s", icons[i].text);
            


            // 플레이어 룩 벡터 방향으로 레이를 보여줄 것인지
            {
                auto iter = m_UsingRaysPlayerLook.find(Bone.name);
                if (iter == m_UsingRaysPlayerLook.end())
                    m_UsingRaysPlayerLook[Bone.name] = false;
                else 
                {
                    ImGui::Checkbox("Active Ray (Player Look)", &m_UsingRaysPlayerLook[Bone.name]);

                    ImGui::SameLine();

                    // 현재 커서 위치를 가져옴
                    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                    // 아이콘을 그리기 위한 DrawList 가져오기
                    ImDrawList* drawList = ImGui::GetWindowDrawList();

                    // 네모 그리기
                    drawList->AddRectFilled
                    (
                        ImVec2(cursorPos.x, cursorPos.y),                       // 네모의 왼쪽 위 좌표
                        ImVec2(cursorPos.x + iconSize, cursorPos.y + iconSize), // 네모의 오른쪽 아래 좌표
                        ImColor(icons[0].color)                                // 색상
                    );

                    // 네모 오른쪽으로 텍스트 출력
                    ImGui::Dummy(ImVec2(iconSize, iconSize)); // 네모의 크기만큼 공간 확보
                    ImGui::SameLine();
                    ImGui::Text("%s", icons[0].text);
                }
            }
            
            // 뼈 월드 룩 방향으로 레이를 보여줄 것인지
            {
                auto iter = m_UsingRaysBoneLook.find(Bone.name);
                if (iter == m_UsingRaysBoneLook.end())
                    m_UsingRaysBoneLook[Bone.name] = false;
                else
                {
                    ImGui::Checkbox("Active Ray (Bone World Look)", &m_UsingRaysBoneLook[Bone.name]);

                    ImGui::SameLine();

                    // 현재 커서 위치를 가져옴
                    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                    // 아이콘을 그리기 위한 DrawList 가져오기
                    ImDrawList* drawList = ImGui::GetWindowDrawList();

                    // 네모 그리기
                    drawList->AddRectFilled
                    (
                        ImVec2(cursorPos.x, cursorPos.y),                       // 네모의 왼쪽 위 좌표
                        ImVec2(cursorPos.x + iconSize, cursorPos.y + iconSize), // 네모의 오른쪽 아래 좌표
                        ImColor(icons[2].color)                                // 색상
                    );

                    // 네모 오른쪽으로 텍스트 출력
                    ImGui::Dummy(ImVec2(iconSize, iconSize)); // 네모의 크기만큼 공간 확보
                    ImGui::SameLine();
                    ImGui::Text("%s", icons[2].text);
                }
            }

            // 뼈 로컬 룩 방향으로 레이를 보여줄 것인지
            {
                auto iter = m_UsingRaysBoneLocalLook.find(Bone.name);
                if (iter == m_UsingRaysBoneLocalLook.end())
                    m_UsingRaysBoneLocalLook[Bone.name] = false;
                else
                {
                    ImGui::Checkbox("Active Ray (Bone Local Look)", &m_UsingRaysBoneLocalLook[Bone.name]);

                    ImGui::SameLine();

                    // 현재 커서 위치를 가져옴
                    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                    // 아이콘을 그리기 위한 DrawList 가져오기
                    ImDrawList* drawList = ImGui::GetWindowDrawList();

                    // 네모 그리기
                    drawList->AddRectFilled
                    (
                        ImVec2(cursorPos.x, cursorPos.y),                       // 네모의 왼쪽 위 좌표
                        ImVec2(cursorPos.x + iconSize, cursorPos.y + iconSize), // 네모의 오른쪽 아래 좌표
                        ImColor(icons[1].color)                                // 색상
                    );

                    // 네모 오른쪽으로 텍스트 출력
                    ImGui::Dummy(ImVec2(iconSize, iconSize)); // 네모의 크기만큼 공간 확보
                    ImGui::SameLine();
                    ImGui::Text("%s", icons[1].text);
                }
            }

            // 뼈 로컬 UpdateExcludeTransformationMatrix 룩 방향으로 레이를 보여줄 것인지
            {
                /*string strUPBName = m_pAT_PartAnimObj_Model->Get_UpdateExcludeBoneName();

                if (strUPBName == Bone.name)
                {
                    auto iter = m_UsingRaysBoneLocalUpdateExcludeLook.find(Bone.name);
                    if (iter == m_UsingRaysBoneLocalUpdateExcludeLook.end())
                        m_UsingRaysBoneLocalUpdateExcludeLook[Bone.name] = false;
                    else
                        ImGui::Checkbox("Active Ray (Bone Local Update Exclude Look)", &m_UsingRaysBoneLocalUpdateExcludeLook[Bone.name]);
                }*/
            }


            if (ImGui::Checkbox("Show Bone Pos", &Bone.bShowBonePos))
            {
                if (Bone.bShowBonePos)
                {
                    cout << Bone.name << endl;

                    // 뼈 생성
                    if (FAILED(Spawn_ShowBoneSquare(Bone.name)))
                    {
                        MSG_BOX("Square 오브젝트 생성 실패 (개수 초과)");
                        Bone.bShowBonePos = false;
                    }
                }
                else
                {
                    string strRemoveShowBoneName{};
                    auto iter = find_if(m_SelectedShowBonesNames.begin(), m_SelectedShowBonesNames.end(), [&](string strSelectedShowBoneName) 
                        {
                            if (strSelectedShowBoneName == Bone.name)
                            {
                                strRemoveShowBoneName = strSelectedShowBoneName;
                                return true;
                            }
                            return false;
                        }
                    );

                    if (iter != m_SelectedShowBonesNames.end())
                    {
                        if (FAILED(Remove_ShowboneSquare(strRemoveShowBoneName)))
                        {
                            MSG_BOX("Square 오브젝트 삭제 실패");
                        }
                        m_SelectedShowBonesNames.erase(iter);
                    }
                }
            }

            _int iCurBoneBlendBoneTypeIdx = m_BlendBones[Bone.index].second;

            // 0은 none이다. None인 경우 m_SelectedBlendBones에서 찾아서 없앤다.
            if (m_BlendBones[Bone.index].second == 0)
            {
                for (auto iter = m_SelectedBlendBones.begin(); iter != m_SelectedBlendBones.end();)
                {
                    if (iter->first == m_BlendBones[Bone.index].first)
                        iter = m_SelectedBlendBones.erase(iter);
                    else
                        ++iter;
                }
            }

            // Pass in the preview value visible before opening the combo (it could technically be different contents or not pulled from items[])
            const _char* pCurBoneComboVal = m_BlendBoneTypes[iCurBoneBlendBoneTypeIdx];

            static _bool bChecked[_int(ANIM_BLEND_GROUP_TYPE::BLEND_END)] = { false }; // 유니폼 초기화

            if (ImGui::BeginCombo("Blend Bone Combo", pCurBoneComboVal, 0))
            {
                for (_int n = (_int)ANIM_BLEND_GROUP_TYPE::LOWER_BODY; n < IM_ARRAYSIZE(m_BlendBoneTypes); n++)
                {
                    const _bool is_selected = (iCurBoneBlendBoneTypeIdx == n);
                    if (ImGui::Selectable(m_BlendBoneTypes[n], is_selected))
                    {
                        if (m_SelectedBlendBones.size() <= 5)
                        {
                            iCurBoneBlendBoneTypeIdx = n;
                            m_BlendBones[Bone.index].second = iCurBoneBlendBoneTypeIdx;

                            if (iCurBoneBlendBoneTypeIdx != 0)
                            {
                                // LOWER_BODY
                                if (iCurBoneBlendBoneTypeIdx == _int(ANIM_BLEND_GROUP_TYPE::LOWER_BODY))
                                {
                                    m_SelectedBlendBones.push_back({ Bone.name, {Bone.index, ANIM_BLEND_GROUP_TYPE::LOWER_BODY } });
                                }
                                // UPPER_BODY
                                else if (iCurBoneBlendBoneTypeIdx == _int(ANIM_BLEND_GROUP_TYPE::UPPER_BODY))
                                {
                                    m_SelectedBlendBones.push_back({ Bone.name, {Bone.index, ANIM_BLEND_GROUP_TYPE::UPPER_BODY } });
                                }
                                // ARM_LEFT
                                else if (iCurBoneBlendBoneTypeIdx == _int(ANIM_BLEND_GROUP_TYPE::ARM_LEFT))
                                {
                                    m_SelectedBlendBones.push_back({ Bone.name, {Bone.index, ANIM_BLEND_GROUP_TYPE::ARM_LEFT } });
                                }
                                // ARM_RIGHT
                                else if (iCurBoneBlendBoneTypeIdx == _int(ANIM_BLEND_GROUP_TYPE::ARM_RIGHT))
                                {
                                    m_SelectedBlendBones.push_back({ Bone.name, {Bone.index, ANIM_BLEND_GROUP_TYPE::ARM_RIGHT } });
                                }
                                // HEAD
                                else if (iCurBoneBlendBoneTypeIdx == _int(ANIM_BLEND_GROUP_TYPE::HEAD))
                                {
                                    m_SelectedBlendBones.push_back({ Bone.name, {Bone.index, ANIM_BLEND_GROUP_TYPE::HEAD } });
                                }
                                // TOTAL
                                else if (iCurBoneBlendBoneTypeIdx == _int(ANIM_BLEND_GROUP_TYPE::TOTAL))
                                {
                                    m_SelectedBlendBones.push_back({ Bone.name, {Bone.index, ANIM_BLEND_GROUP_TYPE::TOTAL } });
                                }
                            }
                        }
                    }

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }

        // Show Bone SRT
        // 이곳에서 Show Bone SRT를 펼친 뼈 이름에 해당하는 TrnasformationMatrix의 주소를 얻어온다
        if (ImGui::TreeNode("Show Bone SRT"))
        {
            string strBoneName = Bone.name;

            // 뼈의 Transformationmatrix
            _float4x4* LocalMat = m_pAT_PartAnimObj_Model->Get_Bone_TransformationMatrix_Ptr(strBoneName);
            if (LocalMat)
            {
                // 본의 로컬 matrix저장
                auto iter = m_BoneTransformationMatrices.find(strBoneName);
                if (iter == m_BoneTransformationMatrices.end())
                {
                    // 뼈 TransformMatrix 주소 저장
                    _float temp[3] = {0.f, 0.f, 0.f};
                    m_BoneTransformationMatrices[strBoneName] = LocalMat;

                    // 뼈 pos 입력 저장
                    m_BoneMatrixPosInput[strBoneName][0] = temp[0];
                    m_BoneMatrixPosInput[strBoneName][1] = temp[1];
                    m_BoneMatrixPosInput[strBoneName][2] = temp[2];

                    m_BoneMatrixRotInput[strBoneName][0] = temp[0];
                    m_BoneMatrixRotInput[strBoneName][1] = temp[1];
                    m_BoneMatrixRotInput[strBoneName][2] = temp[2];

                    // 뼈 SRT변경 여부 저장
                    m_BoneMatrixSRTChange[strBoneName] = false;
                }

                 Controll_Bone_Matrix(strBoneName);
            }

            ImGui::TreePop();
        }

        ImGui::SeparatorText("");

        for (_int i = 0; i < Bone.children.size(); ++i)
            Show_BoneInfo_Recur_TreeNode(Bone.children[i]);

        ImGui::TreePop();
    }

    return strSelectedBoneName;
}


_bool CLevel_AnimTool::Show_BoneWorldPos()
{
    if (!m_SelectedShowBonesNames.empty())
    {
        for (_int i = 0; i < m_SelectedShowBonesNames.size(); ++i)
        {
            _matrix matBoneCombinedTransformationMatrix = m_pAT_PartAnimObj_Model->Get_Bone_CombiendTransformationMatrix(m_SelectedShowBonesNames[i]);

            _matrix matWorld = m_pAT_Obj->Get_Transform()->Get_WorldMatrix();
            matWorld = matBoneCombinedTransformationMatrix * matWorld;
            _vector vPos = matWorld.r[3];

            auto iter = m_SelectedShowBoneSquare.find(m_SelectedShowBonesNames[i]);
            if (iter != m_SelectedShowBoneSquare.end())
            {
                CAT_Square* pSquare = static_cast<CAT_Square*>((iter->second).second);
                if (pSquare)
                {
                    pSquare->Get_Transform()->Set_State(CTransform::STATE_POSITION, vPos);
                    continue;
                }
            }
        }
    }

    return true;
}

HRESULT CLevel_AnimTool::Spawn_ShowBoneSquare(string strShowBoneName)
{
    const _int iMaxNumShowBone = 20;
    if (m_SelectedShowBoneSquare.size() >= iMaxNumShowBone)
        return E_FAIL;

    static _int iNum = 0;
    static string strObjName = "ShowBoneSphere";
    strObjName += to_string(iNum++);

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(LEVEL_ANIMTOOL, TEXT("Prototype_GameObject_AT_Square"), LEVEL_ANIMTOOL, L"Layer_ShowBone", nullptr, CONVERTTOWSTRING(strObjName))))
        return E_FAIL;

    auto pObj = m_pGameInstance->Find_Object_By_Name(LEVEL_ANIMTOOL, L"Layer_ShowBone", CONVERTTOWSTRING(strObjName));
    if (!pObj)
        return E_FAIL;

    m_SelectedShowBonesNames.push_back(strShowBoneName);
    m_SelectedShowBoneSquare.insert({ strShowBoneName, {strObjName, static_cast<CAT_Square*>(pObj)} });

    return S_OK;
}

HRESULT CLevel_AnimTool::Remove_ShowboneSquare(string strShowBoneName)
{
    auto iter = m_SelectedShowBoneSquare.find(strShowBoneName);
    if (iter != m_SelectedShowBoneSquare.end())
    {
        if (FAILED(m_pGameInstance->Remove_Object(LEVEL_ANIMTOOL, CONVERTTOWSTRING((iter->second).first))))
            return E_FAIL;
        else
        {
            m_SelectedShowBoneSquare.erase(iter);
            return S_OK;
        }
    }
    return E_FAIL;
}

_bool CLevel_AnimTool::XML_Show_SaveRootMotionBone_Btn(string strSelectedBoneName)
{
    ImGui::SeparatorText("");
    /*static string strInput = "";
    ImGui::Text("input file name"); ImGui::SameLine();
    strInput = Show_StringInput("Input", strInput);
    ImGui::Text("You Input : %s", strInput.c_str());*/

    if (ImGui::Button("Save Check Bone As Root Motion Bone"))
    {
        if (strSelectedBoneName == "")
        {
            Add_Log(LOG_FAIL, XML_SAVE, "XML_Show_SaveRootMotionBone_Btn", "not select rootbone");
            return false;
        }
        else
        {
            WCHAR strRetModelPath[256]; // 로드할 모델 최종 경로
            if (Open_Save_Modal("xml", strRetModelPath))
            {
                if (FAILED(XML_Save_RootMotionBoneNameData(strRetModelPath, strSelectedBoneName)))
                {
                    Add_Log(LOG_FAIL, XML_SAVE, "XML_Show_SaveRootMotionBone_Btn", "save");
                    return false;
                }
                Add_Log(LOG_OK, XML_SAVE, "XML_Show_SaveRootMotionBone_Btn", CONVERTTOSTRING(strRetModelPath));
            }
        }
    }

    return true;
}


HRESULT CLevel_AnimTool::XML_Save_Selected_BlendBones_Btn()
{
    if (ImGui::Button("Save Selected Blend Bones"))
    {
        if (m_SelectedBlendBones.empty())
        {
            Add_Log(LOG_FAIL, XML_SAVE, "XML_SaveSelectedBlendBones_Btn", "m_SelectedBlendBones is empty");
            return E_FAIL;
        }

        WCHAR strPath[256] = {};
        if (Open_Save_Modal("xml", strPath))
        {            
            xml::XMLDocument doc;
            xml::XMLDeclaration* pDecl = doc.NewDeclaration();
            doc.LinkEndChild(pDecl);

            XML_Add_Comment(&doc); // 날짜 추가

            xml::XMLElement* pEle = doc.NewElement("BlendBones");
            pEle->SetAttribute("Num", m_SelectedBlendBones.size());
            doc.LinkEndChild(pEle);

            for (_int i = 0; i < m_SelectedBlendBones.size(); ++i)
            {
                xml::XMLElement* pEleBlendBone = doc.NewElement("BlendBone");

                pEleBlendBone->SetAttribute("BlendBoneName", m_SelectedBlendBones[i].first.c_str());
                pEleBlendBone->SetAttribute("BlendBoneIndex", m_SelectedBlendBones[i].second.first);
                pEleBlendBone->SetAttribute("BlendBoneType", (_int)m_SelectedBlendBones[i].second.second);

                if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::LOWER_BODY))
                    pEleBlendBone->SetAttribute("BlendBoneTypeName", "LowerBody");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::UPPER_BODY))
                    pEleBlendBone->SetAttribute("BlendBoneTypeName", "UpperBody");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::ARM_LEFT))
                    pEleBlendBone->SetAttribute("BlendBoneTypeName", "ArmLeft");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::ARM_RIGHT))
                    pEleBlendBone->SetAttribute("BlendBoneTypeName", "ArmRight");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::HEAD))
                    pEleBlendBone->SetAttribute("BlendBoneTypeName", "Head");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::TOTAL))
                    pEleBlendBone->SetAttribute("BlendBoneTypeName", "Total");

                pEle->LinkEndChild(pEleBlendBone);
            }

            for (_int i = 0; i < m_SelectedBlendBones.size(); ++i)
            {
                xml::XMLElement* pEleBlendBoneParent = doc.NewElement("BlendBoneParentName");

                pEleBlendBoneParent->SetAttribute("Name", m_SelectedBlendBones[i].first.c_str());
                
                if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::LOWER_BODY))
                    pEleBlendBoneParent->SetAttribute("Type", "LowerBody");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::UPPER_BODY))
                    pEleBlendBoneParent->SetAttribute("Type", "UpperBody");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::ARM_LEFT))
                    pEleBlendBoneParent->SetAttribute("Type", "ArmLeft");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::ARM_RIGHT))
                    pEleBlendBoneParent->SetAttribute("Type", "ArmRight");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::HEAD))
                    pEleBlendBoneParent->SetAttribute("Type", "Head");
                else if ((_int)m_SelectedBlendBones[i].second.second == _int(ANIM_BLEND_GROUP_TYPE::TOTAL))
                    pEleBlendBoneParent->SetAttribute("Type", "Total");


                _int iMinBoneIdx = 1e6, iMaxBoneIdx = -1e6;
                XML_Save_Selected_BlendBones(&doc, pEleBlendBoneParent, m_RootBone, m_SelectedBlendBones[i].first, iMinBoneIdx, iMaxBoneIdx);

                // min, max bone idx를 저장한다.
                pEleBlendBoneParent->SetAttribute("MinBoneIdx", iMinBoneIdx);
                pEleBlendBoneParent->SetAttribute("MaxBoneIdx", iMaxBoneIdx);

                doc.LinkEndChild(pEleBlendBoneParent);
            }


            if (doc.SaveFile(CONVERTTOSTRING(strPath).c_str()) == xml::XML_SUCCESS)
            {
                Add_Log(LOG_OK, XML_SAVE, "XML_SaveSelectedBlendBones_Btn", CONVERTTOSTRING(strPath));
                return S_OK;
            }
            else
            {
                Add_Log(LOG_FAIL, XML_SAVE, "XML_SaveSelectedBlendBones_Btn", CONVERTTOSTRING(strPath));
                return E_FAIL;
            }
        }
    }
    
    ImGui::SameLine();
    HelpMarker("save blend 4 bones, what you selected");
    return S_OK;
}

void CLevel_AnimTool::XML_Save_Selected_BlendBones(xml::XMLDocument* pDoc, xml::XMLElement* pEleParent, BONE_XML& Bone, string strDstBoneName, _int& iMinBoneIdx, _int& iMaxBoneIdx)
{
    // 재귀 탈줄 조건
    // if (!pBoneEle)
    //     return;

    XML_Save_BlendBones_ReCursion(pDoc, pEleParent, Bone, strDstBoneName, false, iMinBoneIdx, iMaxBoneIdx);
}



HRESULT CLevel_AnimTool::XML_Load_BlendBones()
{
    WCHAR strFilePath[256] = {};
    if (Open_Modal("xml", strFilePath))
    {
        xml::XMLDocument Doc;

        if (_waccess(strFilePath, 0) == -1)
        {
            std::wcout << L"File not found: " << strFilePath << std::endl;
            return E_FAIL;
        }
        else
        {
            std::wcout << L"File exists: " << strFilePath << std::endl;
        }

        string stRetFilePath = CONVERTTOSTRING(strFilePath);

        if (Doc.LoadFile(stRetFilePath.c_str()) != xml::XML_SUCCESS)
        {
            std::cout << "Error: " << Doc.ErrorID() << " - " << Doc.ErrorStr() << std::endl;
            Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_BlendBones", "load fail xml");
            return E_FAIL;
        }

        xml::XMLElement* pStartNode = Doc.FirstChildElement("BlendBones"); // 루트 노드
        if (!pStartNode)
        {
            Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_BlendBones", "pStartNode not match xml");
            return E_FAIL;
        }

        _int iNumBlendBone = 0;
        pStartNode->QueryIntAttribute("Num", &iNumBlendBone);

        if (!iNumBlendBone)
        {
            Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_BlendBones", "iNumBlendBone == 0");
            return E_FAIL;
        }

        vector<string> vecBlendBoneNames; vecBlendBoneNames.resize(iNumBlendBone);
        vector<_int> vecBlendBoneindces; vecBlendBoneindces.resize(iNumBlendBone);
        vector<_int> vecBlendBoneTypes; vecBlendBoneTypes.resize(iNumBlendBone);

        xml::XMLElement* pBlendBoneNode = pStartNode->FirstChildElement("BlendBone");
        if (!pBlendBoneNode)
        {
            Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_BlendBones", "pBlendBoneNode is nullptr");
            return E_FAIL;
        }

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
        {
            Add_Log(LOG_FAIL, XML_LOAD, "XML_Load_BlendBones", "pNextNode is nullptr");
            return E_FAIL;
        }

        for (_int i = 0; i < iNumBlendBone; ++i)
        {

            string strBlendBoneParentName = pNextNode->FindAttribute("Name")->Value();
            string strBlendBoneType = pNextNode->FindAttribute("Type")->Value();
            _int iBlendBoneMinIdx = pNextNode->FindAttribute("MinBoneIdx")->IntValue();
            _int iBlendBoneMaxIdx = pNextNode->FindAttribute("MaxBoneIdx")->IntValue();

            xml::XMLElement* pChild = pNextNode->FirstChildElement("Bone");

            BONE_XML RootBone{};
            XML_ParseBone_Recursion(pChild, RootBone, true);
            vecBlendBoneGroup.push_back(RootBone);

            if (m_pAT_PartAnimObj_Model)
            {
                ANIM_BLEND_GROUP_TYPE eBlendGroupType = ANIM_BLEND_GROUP_TYPE::BLEND_END;
                if (strBlendBoneType == "LowerBody")
                    eBlendGroupType = ANIM_BLEND_GROUP_TYPE::LOWER_BODY;
                else if (strBlendBoneType == "UpperBody")
                    eBlendGroupType = ANIM_BLEND_GROUP_TYPE::UPPER_BODY;
                else if (strBlendBoneType == "ArmLeft")
                    eBlendGroupType = ANIM_BLEND_GROUP_TYPE::ARM_LEFT;
                else if (strBlendBoneType == "ArmRight")
                    eBlendGroupType = ANIM_BLEND_GROUP_TYPE::ARM_RIGHT;
                else if (strBlendBoneType == "Head")
                    eBlendGroupType = ANIM_BLEND_GROUP_TYPE::HEAD;
                else if (strBlendBoneType == "Total")
                    eBlendGroupType = ANIM_BLEND_GROUP_TYPE::TOTAL;
                else if (eBlendGroupType == ANIM_BLEND_GROUP_TYPE::BLEND_END)
                    return E_FAIL;

                // Blend Bone의 기본 초기값 설정해준다.
                m_pAT_PartAnimObj_Model->Init_BlendGroup_DefaultInfo(eBlendGroupType, CModel::DEFAULT_INFO{ strBlendBoneParentName, eBlendGroupType, iBlendBoneMinIdx, iBlendBoneMaxIdx });
            }

            pNextNode = pNextNode->NextSiblingElement();
        }

        m_bLoadSuccessBlendBones = true;
        Add_Log(LOG_OK, XML_LOAD, "XML_Load_BlendBones", "load from" + stRetFilePath);
    }
    return S_OK;
}

void CLevel_AnimTool::XML_ParseBone_Recursion(xml::XMLElement* pParentEle, BONE_XML& Bone, _bool bLoadBlendBone)
{
    // 재귀 탈줄 조건
    if (!pParentEle)
        return;

    // 현재 노드 속성 읽기 (루트 부터 읽힌다)
    Bone.name = pParentEle->Attribute("Name");
    pParentEle->QueryIntAttribute("Index", &Bone.index);
    pParentEle->QueryIntAttribute("ParentIndex", &Bone.parentIndex);
    pParentEle->QueryIntAttribute("NumChild", &Bone.numChild);
    pParentEle->QueryIntAttribute("NumTotalChildBone", &Bone.numTotalChildBone);

    m_BlendBones.push_back({ Bone.name, 0 });

    xml::XMLElement* pChildEle = pParentEle->FirstChildElement("Bone");
    while (pChildEle)
    {
        BONE_XML ChildBone{};
        XML_ParseBone_ReCursion(pChildEle, ChildBone);
        Bone.children.push_back(ChildBone);
        pChildEle = pChildEle->NextSiblingElement("Bone");
    }
}

HRESULT CLevel_AnimTool::XML_Save_Events(const WCHAR* strXMLFilePath)
{
    xml::XMLDocument* pDoc = new xml::XMLDocument;

    xml::XMLDeclaration* pDecl = pDoc->NewDeclaration();
    pDoc->LinkEndChild(pDecl);
    XML_Add_Comment(pDoc); // 날짜 추가


    xml::XMLElement* pRoot = pDoc->NewElement("AnimationEvent");
    pRoot->SetAttribute("NumAnim", m_Events.size());
    pDoc->LinkEndChild(pRoot);

    for (auto iter = m_Events.begin(); iter != m_Events.end(); ++iter)
    {
        string strAnimName = iter->first;
        map<string, vector<pair<string, _float>>> vecEventLayers = iter->second;

        xml::XMLElement* pAnimRootEle = pDoc->NewElement("Animation");
        pAnimRootEle->SetAttribute("Name", strAnimName.c_str());                // 애니매이션 이름 쓰기
        pAnimRootEle->SetAttribute("NumLayer", vecEventLayers.size());          // 애니매이션 레이어 개수 쓰기
        pRoot->LinkEndChild(pAnimRootEle);                                      // pDoc의 자식으로 pRoot를 추가한다.

        for (auto iterEL = vecEventLayers.begin(); iterEL != vecEventLayers.end(); ++iterEL)
        {
            string strEventLayerName = iterEL->first;
            vector<pair<string, _float>> vecEvents = iterEL->second;

            xml::XMLElement* pLayerEle = pDoc->NewElement("Layer");             
            pLayerEle->SetAttribute("Name", strEventLayerName.c_str());         // 레이어 이름 쓰기
            pLayerEle->SetAttribute("NumEvent", vecEvents.size());              // 레이어 가 가지고 있는 이벤트 개수 쓰기
            pAnimRootEle->LinkEndChild(pLayerEle);

            for (_int i = 0; i < vecEvents.size(); ++i)
            {
                string strEventName = vecEvents[i].first;
                _float strEventTrackPos = vecEvents[i].second;

                xml::XMLElement* pEventEle = pDoc->NewElement("Event");
                pEventEle->SetAttribute("Name", strEventName.c_str());          // 이벤트 이름 쓰기
                pEventEle->SetAttribute("TrackPos", strEventTrackPos);          // 이벤트 TrackPos 쓰기
                pLayerEle->LinkEndChild(pEventEle);
            }
        }
    }

    pDoc->SaveFile(CONVERTTOSTRING(strXMLFilePath).c_str());
    delete pDoc;
    pDoc = nullptr;
    return S_OK;
}

HRESULT CLevel_AnimTool::XML_Load_Events(const WCHAR* strXMLFilePath)
{
    xml::XMLDocument doc;
    if (doc.LoadFile(CONVERTTOSTRING(strXMLFilePath).c_str()) != xml::XML_SUCCESS)
        return E_FAIL;

    xml::XMLElement* pRoot = doc.FirstChildElement("AnimationEvent");
    if (!pRoot)
        return E_FAIL;

    _int iNumAnim{};
    pRoot->QueryAttribute("NumAnim", &iNumAnim);

    xml::XMLElement* pAnimEle = pRoot->FirstChildElement();
    if (!pAnimEle)
        return E_FAIL;

    for (_int i = 0; i < iNumAnim; ++i)
    {
        const _char* pAnimName = nullptr;
        pAnimEle->QueryAttribute("Name", &pAnimName);
        if (!pAnimName)
            return E_FAIL;

        string strAnimName = pAnimName;
        _int iNumLayer{};
        pAnimEle->QueryIntAttribute("NumLayer", &iNumLayer);
        
        xml::XMLElement* pLayerEle = pAnimEle->FirstChildElement();
        if (!pLayerEle)
            return E_FAIL;

        for (_int j = 0; j < iNumLayer; ++j)
        {
            const _char* pLayerName = nullptr;
            pLayerEle->QueryAttribute("Name", &pLayerName);
            if (!pLayerName)
                return E_FAIL;

            string strLayerName = pLayerName;
            _int iNumEvent;
            pLayerEle->QueryIntAttribute("NumEvent", &iNumEvent);
            
            xml::XMLElement* pEventEle = pLayerEle->FirstChildElement();
            if (!pEventEle)
                return E_FAIL;

            for (_int k = 0; k < iNumEvent; ++k)
            {
                const _char* pEventName = nullptr;
                pEventEle->QueryAttribute("Name", &pEventName);
                if (!pEventName)
                    return E_FAIL;
                
                string strEventName = pEventName;
                _float fTrackPos{};
                pEventEle->QueryFloatAttribute("TrackPos", &fTrackPos);

                m_Events[strAnimName][strLayerName].emplace_back(strEventName, fTrackPos);
                
                pEventEle = pEventEle->NextSiblingElement();
            }

            pLayerEle = pLayerEle->NextSiblingElement();

        }
        
        pAnimEle = pAnimEle->NextSiblingElement();
    }

    return S_OK;
}


void CLevel_AnimTool::MultiControll_ModelMeshNames_TreeNode()
{
    if (ImGui::TreeNode("Multi-Controll-Model And Mesh"))
    {
        Show_MeshNames_ListBox();
        Show_LoadPartAnimModel_FBX_Radio();
        ImGui::TreePop();
    }
}

ImGuizmo::OPERATION g_CurrentGizmoOperation(ImGuizmo::TRANSLATE);
ImGuizmo::MODE g_CurrentGizmoMode(ImGuizmo::WORLD);
bool g_useSnap(false);
float g_snap[3] = { 1.f, 1.f, 1.f };
void CLevel_AnimTool::TestGizmo()
{
    _float4x4 matProj{}, matView{};
    XMStoreFloat4x4(&matView, m_pGameInstance->Get_TransformMatrix(CPipeLine::D3DTS_VIEW));
    XMStoreFloat4x4(&matProj, m_pGameInstance->Get_TransformMatrix(CPipeLine::D3DTS_PROJ));

    _float4x4 identity{};
    XMStoreFloat4x4(&identity, XMMatrixIdentity());

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    //ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList(viewport));
    ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

    ImGui::Begin("Test");

    ImGuiIO& io = ImGui::GetIO();

    ImGui::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);

    if (ImGuizmo::IsUsing())
    {
        ImGui::Text("Using gizmo");
    }
    else
    {
        ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
        ImGui::SameLine();
        ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
        ImGui::SameLine();
        ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
        ImGui::SameLine();
        ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
    }
    ImGui::Separator();

    if (ImGui::RadioButton("Translate", g_CurrentGizmoOperation == ImGuizmo::TRANSLATE))
        g_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", g_CurrentGizmoOperation == ImGuizmo::ROTATE))
        g_CurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", g_CurrentGizmoOperation == ImGuizmo::SCALE))
        g_CurrentGizmoOperation = ImGuizmo::SCALE;

    float matrixTranslation[3], matrixRotation[3], matrixScale[3];

    // m_pAT_PartAnimObj_Model->Get_bone
    _float4x4* pWorldMatrix = m_pAT_Obj->Get_Transform()->Get_WorldFloat4x4_Ptr_NConst();

    ImGuizmo::DecomposeMatrixToComponents(&pWorldMatrix->m[0][0], matrixTranslation, matrixRotation, matrixScale);
    ImGui::DragFloat3("Tr", matrixTranslation, 0.01f);
    ImGui::DragFloat3("Rt", matrixRotation, 0.01f);
    ImGui::DragFloat3("Sc", matrixScale, 0.01f);
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, &pWorldMatrix->m[0][0]);

    if (g_CurrentGizmoOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", g_CurrentGizmoMode == ImGuizmo::LOCAL))
            g_CurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", g_CurrentGizmoMode == ImGuizmo::WORLD))
            g_CurrentGizmoMode = ImGuizmo::WORLD;
    }

    ImGui::Checkbox(" ", &g_useSnap);
    ImGui::SameLine();
    switch (g_CurrentGizmoOperation)
    {
    case ImGuizmo::TRANSLATE:
        ImGui::InputFloat3("Snap", &g_snap[0]);
        break;
    case ImGuizmo::ROTATE:
        ImGui::InputFloat("Angle Snap", &g_snap[0]);
        break;
    case ImGuizmo::SCALE:
        ImGui::InputFloat("Scale Snap", &g_snap[0]);
        break;
    }



    _float4x4 FinalWorldMatrix{};


    if (pWorldMatrix)
        ImGuizmo::Manipulate(&matView.m[0][0], &matProj.m[0][0], g_CurrentGizmoOperation, g_CurrentGizmoMode, &pWorldMatrix->m[0][0], NULL, g_useSnap ? &g_snap[0] : NULL);

    // _float4x4 matWorld{};
    // XMStoreFloat4x4(&matWorld, XMLoadFloat4x4(&FinalWorldMatrix));
    // XMStoreFloat4x4(const_cast<_float4x4*>(pWorldMatrix), XMLoadFloat4x4(&FinalWorldMatrix));

    ImGui::End();

}

void CLevel_AnimTool::MultiControll_IK_TreeNode()
{
    if (ImGui::TreeNode("Multi-Controll-IK"))
    {
        Show_IK_Options();

        ImGui::TreePop();
    }
}

struct AnimEvent {
    float time;
    std::string name;
};

// 애니메이션 이벤트를 저장할 벡터
std::vector<AnimEvent> animEvents;



void ShowAnimationSequencer()
{
    // 현재 선택된 프레임 (플레이 헤드 역할)
    static int currentFrame = 0;

    _int iTotalFrame = 100;

    // 애니메이션 키프레임들이 들어있는 벡터 (각 프레임 위치)
    static std::vector<int> keyframes = { 10, 30, 50 };

    // 현재 선택된 키프레임의 인덱스 (-1이면 선택 안됨)
    static int selectedKeyframe = -1;

    // 새로운 ImGui 윈도우 시작
    ImGui::Begin("Animation Sequencer");

    // 타임라인 제목 출력
    ImGui::Text("Timeline");

    // 구분선 추가 (시각적으로 영역을 나눔)
    ImGui::Separator();

    // 타임라인 전체 너비 (현재 창에서 사용 가능한 너비 가져오기)
    float timelineWidth = ImGui::GetContentRegionAvail().x;

    // 100프레임을 기준으로 타임라인을 나누기 위한 스케일 계산
    float frameStep = timelineWidth / (_float)iTotalFrame;

    // 현재 윈도우의 DrawList 가져오기 (ImGui에서 직접 그리기 위한 객체)
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // 현재 마우스 커서 위치 가져오기 (여기에 타임라인이 그려짐)
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    // 타임라인 그리기 (0~100 프레임을 일정 간격으로 구분)
    for (int i = 0; i <= iTotalFrame; i += 10)
    {
        float x = cursorPos.x + i * frameStep;  // 현재 프레임의 X 위치 계산
        drawList->AddLine(ImVec2(x, cursorPos.y),  // 시작점 (위쪽) 
            ImVec2(x, cursorPos.y + 20),  // 끝점 (아래쪽)
            IM_COL32(255, 255, 255, 255)); // 흰색 선으로 표시
    }

    // 키프레임 그리기
    for (size_t i = 0; i < keyframes.size(); i++)
    {
        int frame = keyframes[i];  // 현재 키프레임의 프레임 값 가져오기
        float x = cursorPos.x + frame * frameStep;  // 키프레임의 X 좌표 계산
        ImVec2 pos(x - 5, cursorPos.y + 10);  // 키프레임 사각형 시작 위치
        ImVec2 size(x + 5, cursorPos.y + 20); // 키프레임 사각형 끝 위치

        // 키프레임이 선택된 상태인지 확인
        bool isSelected = (selectedKeyframe == i);

        // 키프레임 그리기 (선택되었으면 빨간색, 아니면 초록색)
        drawList->AddRectFilled(pos, size,
            isSelected ? IM_COL32(255, 0, 0, 255)  // 선택된 경우 빨간색
            : IM_COL32(0, 255, 0, 255)); // 기본 초록색

        // 키프레임 선택 감지 (마우스로 클릭하면 선택됨)
        if (ImGui::IsMouseHoveringRect(pos, size) && ImGui::IsMouseClicked(0))
        {
            selectedKeyframe = i;  // 클릭된 키프레임을 선택된 키프레임으로 설정
        }
    }


    // 선택된 키프레임을 마우스로 드래그하여 이동
    if (selectedKeyframe >= 0 && ImGui::IsMouseDragging(0))
    {
        float mouseX = ImGui::GetMousePos().x - cursorPos.x; // 현재 마우스 X 좌표 가져오기
        int newFrame = std::clamp((int)(mouseX / frameStep), 0, 100); // 0~100 범위 내에서 프레임 계산
        keyframes[selectedKeyframe] = newFrame; // 선택된 키프레임 위치 업데이트
    }

    // ImGui 윈도우 닫기
    ImGui::End();
}


void CLevel_AnimTool::Begin_Animation_KeyFrame()
{
    _float fSX = ImGui_Get_ViewPort_StartPos().x;
    _float fSY = ImGui_Get_ViewPort_StartPos().y;

    _float fPaddingX = g_iMaxWidth / 2.f + 150.f;
    _float fSizeY = 350.f;

    // 창 위치 고정
    ImGui_Fix_Window_Pos({ fSX + fPaddingX, fSY + g_iWinSizeY - fSizeY });

    ImGui::Begin("Animation Time Line");

    string strCurAnimName = m_pAT_PartAnimObj_Model->Get_CurAnim()->Get_AnimName();
    _int iCurAnimIdx = m_pAT_PartAnimObj_Model->Get_AnimIdx(strCurAnimName);
    CAnimation* pCurAnim = m_pAT_PartAnimObj_Model->Get_Anim(iCurAnimIdx);
    
    m_fCurAnimDuration = pCurAnim->Get_AnimDuration(); // 현재 재생 애니매이션 duration 설정
    m_pCurAnimTrackPos = pCurAnim->Get_TrackPos_Ptr(); // 현재 재생 애니매이션 trackpos

    Draw_KeyFrame_TimeLine();

    ImGui::End();
}

// 키 프레임 타임 라인 그리는 함수
// save, load, clear 버튼이 있고, 키프레임들을 그린다. (직사각형을 그리고 그 안에 | 하얀 선들을 그리는 기능들의 함수들을 호출한다.)
void CLevel_AnimTool::Draw_KeyFrame_TimeLine()
{
    // XML 저장 함수
    if (ImGui::Button("Save Events To XML"))
    {
        if (!m_Events.empty())
        {
            WCHAR strRetPath[256];
            if (Open_Save_Modal("xml", strRetPath))
            {
                if (S_OK == XML_Save_Events(strRetPath))
                {
                    Add_Log(LOG_OK, AT_LOG_JOB::XML_SAVE, "Draw_KeyFrame_TimeLine", CONVERTTOSTRING(strRetPath));
                    MSG_BOX("Save OK Animation Events");
                }
                else
                {
                    Add_Log(LOG_FAIL, AT_LOG_JOB::XML_SAVE, "Draw_KeyFrame_TimeLine", CONVERTTOSTRING(strRetPath));
                    MSG_BOX("Save Failed Animation Events");
                }
            }
        }
            
    }

    ImGui::SameLine();

    // XML 로드 함수
    if (ImGui::Button("Load Events From XML"))
    {
        if (!m_Events.empty())
            m_Events.clear();

        WCHAR strFilePath[256];
        if (Open_Modal("xml", strFilePath))
        {
            if (XML_Load_Events(strFilePath) != S_OK)
            {
                Add_Log(LOG_FAIL, AT_LOG_JOB::XML_LOAD, "XML_Load_Events", CONVERTTOSTRING(strFilePath));
                MSG_BOX("Load Fail");
            }
            else
            {
                Add_Log(LOG_OK, AT_LOG_JOB::XML_LOAD, "XML_Load_Events", CONVERTTOSTRING(strFilePath));
                MSG_BOX("Load OK");
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear All Events"))
    {
        if (!m_Events.empty())
        {
            m_Events.clear();
            Add_Log("LOG_OK | Clear All Events");
        }
    }

    ImGui::Separator();

    // 타임라인 제목 출력
    ImGui::Text("Timeline");

    // 구분선 추가 (시각적으로 영역을 나눔)
    ImGui::Separator();

    // 현재 윈도우의 DrawList 가져오기 (ImGui에서 직접 그리기 위한 객체)
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    m_fTimeLineWidth = ImGui::GetContentRegionAvail().x; // 프레임 전체 길이 (현재 창에서 사용가능한 너비를 구한다)
    ImVec2 vCursorPos = ImGui::GetCursorScreenPos(); // 현재 마우스 시작점 (아마 현재 창의 시작점을 의미하는듯 하다)

    const _float fStartX = vCursorPos.x; // 타임 라인의 시작점 x
    const _float fEndX = vCursorPos.x + m_fTimeLineWidth; // 타임라인 끝점 x
    const _float fStartY = vCursorPos.y;

    m_fFrameLineStartX = fStartX;
    m_fFrameLineStartY = vCursorPos.y;

    // 실제 프레임 그리기 | | | 이런 선 그리기
    _float fFrameStep = Draw_TimeLine(pDrawList, m_fTimeLineWidth, vCursorPos, m_fCurAnimDuration);

    // 타임 라인 위치로 좌표계 변환
    // _float fCurTrackPosBarPosX = fStartX + *m_pCurAnimTrackPos * fFrameStep;
    _float fCurTrackPosBarPosX = Cal_TrackPosX(fStartX, *m_pCurAnimTrackPos, fFrameStep);
    _float fTrackPosBarSizeY = 35.f;

    if (KEY_DOWN(KEY::SPACE))
        m_pAT_PartAnimObj->StopAndPlay_CurAnim();

    // TrackPosBar 컨트롤 
    Controll_TrackPosBar(pDrawList, m_pCurAnimTrackPos, fStartX, fStartY, fEndX, fTrackPosBarSizeY, fFrameStep);

}

// pDrawList는 프레임을 그릴 때 필요한 객체이다. dx11로 치면 DeviceContext같은 느낌이다.
// m_fTimeLineWidth : 현재 창에서 사용가능한 너비
// vCursorPos : TimeLine시작 위치
// fCurAnimDuration : 현재 애니매이션 시간(거리)
_float CLevel_AnimTool::Draw_TimeLine(ImDrawList* pDrawList, _float fTimeLineWidth, ImVec2 vCursorPos, _float fCurAnimDuration)
{
    // 프레임을 기준으로 타임라인을 나누기 위한 스케일 계산
    // 현재 창 사용가능한 너비를 애니매이션 재생 시간으로 나눈다. => 프레임 간 간 격이 나온다.
    // 현재 애니매이션 duration을 framestep으로 나누어 | | | 이런 선들의 간격 padding을 구한다.
    _float frameStep = fTimeLineWidth / (_float)fCurAnimDuration; 
    _float fPadding = (_float)fCurAnimDuration / frameStep;

    //// 타임라인 그리기 (0~fCurAnimDuration 프레임을 일정 간격으로 구분)
    for (_int i = 0; i <= (_int)fCurAnimDuration; i++)
    {
        _float x = vCursorPos.x + i * frameStep;  // 현재 프레임의 X 위치 계산
        pDrawList->AddLine(ImVec2(x, vCursorPos.y),  // 시작점 (위쪽) 
            ImVec2(x, vCursorPos.y + m_fFrameLineLen),  // 끝점 (아래쪽)
            IM_COL32(255, 255, 255, 255)); // 흰색 선으로 표시

        _float fTest = (x - m_fFrameLineStartX) / frameStep;
        _float fTrackPos = (_float)i * fPadding;
        ImGui::SetCursorScreenPos(ImVec2(x, vCursorPos.y + m_fFrameLineLen));
        ImGui::Text("%.2f", fTest);

    }

    return frameStep;
}

// 실제 TrackPosBar를 그리고(DrawOn_TrackPosBar)
// TrackPosBar에 마우스를 올리는 경우의 처리(HoverOn_TrackPosBar)
// 추가한 이벤트를 그리는 기능 (Draw_Events)
void CLevel_AnimTool::Controll_TrackPosBar(ImDrawList* pDrawList, _float* pCurTrackPos, _float fFrameLineStartX, _float fFrameLineStartY, _float fFrameLineEndX, _float fTrackPosBarSizeY, _float fFrameStep)
{
    auto Pair = DrawOn_TrackPosBar(pDrawList, pCurTrackPos, fFrameLineStartX, fFrameLineStartY, fFrameLineEndX, fTrackPosBarSizeY, fFrameStep);

    // TrackPosBar에 Hover시 발생하는 것들 모음
    HoverOn_TrackPosBar(Pair.first, *pCurTrackPos);
    
    // TrackPosBar -> 우클릭 ->  Add Event로 추가한 이벤트를 그리는 기능의 함수
    Draw_Events(fFrameLineStartX, fFrameLineStartY, fFrameLineEndX);
}

pair<_bool, _bool> CLevel_AnimTool::DrawOn_TrackPosBar(ImDrawList* pDrawList, _float* pCurTrackPos, _float fFrameLineStartX, _float fFrameLineStartY, _float fFrameLineEndX, _float fTrackPosBarSizeY, _float fFrameStep)
{
    // 현재 선택된 키프레임의 인덱스 (-1이면 선택 안됨)
    static _bool isDragging = false; // 키프레임이 드래그 중인지 확인하는 플래그

    _float fNewTrackPosBarPosX = fFrameLineStartX + *pCurTrackPos * fFrameStep; // 타임 라인 위치로 좌표계 변환

    // 키프레임 사각형 영역 계산
    ImVec2 keyframeMin(fNewTrackPosBarPosX - 4.f, fFrameLineStartY);                     // 사각형의 왼쪽 위
    ImVec2 keyframeMax(fNewTrackPosBarPosX + 4.f, fFrameLineStartY + fTrackPosBarSizeY); // 사각형의 오른쪽 아래

    // 마우스 이벤트 처리
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos; // 현재 마우스 위치

    // 마우스가 키프레임 영역 안에 있는지 확인
    _bool isHovered = (mousePos.x >= keyframeMin.x && mousePos.x <= keyframeMax.x && mousePos.y >= keyframeMin.y && mousePos.y <= keyframeMax.y);

    // 마우스 클릭 시 드래그 시작
    if (isHovered && ImGui::IsMouseClicked(0)) // 왼쪽 클릭
    {
        // TrackPosBar 건드리면 멈춘다.
        if (m_pAT_PartAnimObj_Model->Get_CurAnim_PlayState())
            m_pAT_PartAnimObj_Model->Stop_CurAnim();

        isDragging = true;
    }


    // 드래그 중일 때 마우스 이동에 따라 키프레임 위치 업데이트
    if (isDragging && ImGui::IsMouseDown(0)) // 클릭을 유지하면 드래그 상태
    {
        fNewTrackPosBarPosX = mousePos.x;   // 마우스 X 좌표를 키프레임의 새로운 위치로 설정

        // 타임라인 범위를 벗어나지 않도록 제한
        if (fNewTrackPosBarPosX < fFrameLineStartX) fNewTrackPosBarPosX = fFrameLineStartX;
        if (fNewTrackPosBarPosX > fFrameLineEndX) fNewTrackPosBarPosX = fFrameLineEndX;

        // 키프레임의 새로운 위치를 애니메이션 프레임 값으로 변환
        *pCurTrackPos = ((fNewTrackPosBarPosX - fFrameLineStartX) / fFrameStep);

        keyframeMin = { fNewTrackPosBarPosX - 4.f, fFrameLineStartY };                     // 사각형의 왼쪽 위
        keyframeMax = { fNewTrackPosBarPosX + 4.f, fFrameLineStartY + fTrackPosBarSizeY }; // 사각형의 오른쪽 아래

        m_pAT_PartAnimObj_Model->Set_CurAnim_TrackPos(*pCurTrackPos);
    }

    // 마우스 버튼을 떼면 드래그 종료
    if (!ImGui::IsMouseDown(0))
    {
        // 애니매이션이 재생 중이라면 멈춘다.
        if (isDragging)
        {
            if (m_pAT_PartAnimObj_Model->Get_CurAnim_PlayState())
                m_pAT_PartAnimObj_Model->Stop_CurAnim();

            isDragging = false;

            fNewTrackPosBarPosX = mousePos.x;   // 마우스 X 좌표를 키프레임의 새로운 위치로 설정

            // 타임라인 범위를 벗어나지 않도록 제한
            if (fNewTrackPosBarPosX < fFrameLineStartX) fNewTrackPosBarPosX = fFrameLineStartX;
            if (fNewTrackPosBarPosX > fFrameLineEndX) fNewTrackPosBarPosX = fFrameLineEndX;

            // 키프레임의 새로운 위치를 애니메이션 프레임 값으로 변환
            *pCurTrackPos = ((fNewTrackPosBarPosX - fFrameLineStartX) / fFrameStep);

            keyframeMin = { fNewTrackPosBarPosX - 4.f, fFrameLineStartY };                     // 사각형의 왼쪽 위
            keyframeMax = { fNewTrackPosBarPosX + 4.f, fFrameLineStartY + fTrackPosBarSizeY }; // 사각형의 오른쪽 아래

            m_pAT_PartAnimObj_Model->Set_CurAnim_TrackPos(*pCurTrackPos);
        }
    }

    // 키프레임 색상 선택 (드래그 중이면 초록색, 기본은 빨간색)
    ImU32 keyframeColor = isDragging ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);

    // 키프레임 사각형 그리기
    pDrawList->AddRectFilled(keyframeMin, keyframeMax, keyframeColor);
    
    return make_pair(isHovered, isDragging);
}

void CLevel_AnimTool::HoverOn_TrackPosBar(_bool bHover, _float fCurTrackPos)
{
    // Hover 중이라면 TrackPos와 Add Event 메뉴를 띄운다.
    if (bHover)
    {
        // Hover 했을 때 도움 말? 같은 기능이 뜨게 해주는 함수이다.
        ImGui::SetTooltip("TrackPos: %.2f", fCurTrackPos);

        // imgui에서 지원하는 right 클릭
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            // OpenPopup을 한 경우 아래에서 BeginPopup으로 확인하고 렌더링을 해야한다.
            // EventMenu는 팝업창 키값으로 쓰인다.
            ImGui::OpenPopup("EventMenu");
        }
    }

    static _bool bOpen = false;
    if (ImGui::BeginPopup("EventMenu"))
    {
        if (ImGui::MenuItem("Add Event"))
        {
            bOpen = true;
        }
        ImGui::EndPopup();
    }

    if (bOpen)
    {
        ImGui::OpenPopup("AddEventPopup");
        if (ImGui::BeginPopupModal("AddEventPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static int item = 0;
            ImGui::Combo("Select", &item, "Select\0Layer And Event\0\0");

            if (item == 1)
            {
                static _char LayerBuffer[256]; // 입력받는 버퍼
                static _char EventBuffer[256]; // 입력받는 버퍼
                string strLayerName = "", strEventName = "";

                // 문자열 입력 필드 표시
                if (ImGui::InputText("Add Layer Name", LayerBuffer, sizeof(LayerBuffer)))
                {
                    
                }

                ImGui::SameLine();
                HelpMarker("If Layer does not exist will make new Layer");

                if (ImGui::InputText("Add Event Name", EventBuffer, sizeof(EventBuffer)))
                {

                }

                strLayerName = LayerBuffer;
                strEventName = EventBuffer;

                if (ImGui::Button("Add Event on this layer(existing or create)"))
                {
                    if (strLayerName != "" && strEventName != "")
                    {
                        auto pCurAnim = m_pAT_PartAnimObj_Model->Get_CurAnim();
                        string strCurAnimName = pCurAnim->Get_AnimName();


                        // 같은 이름의 이벤트가 있는지 찾는다.
                        vector<pair<string, _float>>& vecEvents = m_Events[strCurAnimName][strLayerName];
                        for (auto iter = vecEvents.begin(); iter != vecEvents.end();)
                        {
                            if (iter->first == strEventName)
                                iter = vecEvents.erase(iter);
                            else
                                ++iter;
                        }


                        //m_Events[strCurAnimName]가 존재하지 않으면 자동으로** 빈 map<string, vector<pair<string, _float>>>** 을 생성.
                        //    m_Events[strCurAnimName][strLayerName]가 존재하지 않으면 자동으로** 빈 vector<pair<string, _float>>** 을 생성.
                        //operator[]는 키가 존재하면 해당 값을 반환하고, 존재하지 않으면 새로 생성하여 반환.
                        //    이미 존재하는 키에 대해 emplace_back()을 호출하면 기존 데이터에 추가만 됨.
                        //    새로운 키를 사용할 경우, 자동으로 새로운 map과 vector가 생성되므로 직접 존재 여부를 확인할 필요 없음.
                        m_Events[strCurAnimName][strLayerName].emplace_back(make_pair(strEventName, fCurTrackPos));

                        strLayerName = "";
                        strEventName = "";
                        memset(LayerBuffer, '\0', sizeof LayerBuffer);
                        memset(EventBuffer, '\0', sizeof EventBuffer);
                        bOpen = false;
                        ImGui::CloseCurrentPopup();

                        item = 0;
                    }
                    else
                    {
                        MSG_BOX("레이어 이름과 이벤트 이름을 설정해야합니다");
                        
                        if (ImGui::Button("Close"))
                        {
                            bOpen = false;
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndPopup();
                    }
                }
            }
            /*else if (item == 2)
            {

            }*/

            if (ImGui::Button("Close"))
            {
                bOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}

void CLevel_AnimTool::Draw_Events(_float fFrameLineStartStartX, _float fFrameLineStartY, _float fFrameLineEndX)
{
    if (m_Events.empty())
        return;

    auto pCurAnim = m_pAT_PartAnimObj_Model->Get_CurAnim();
    string strCurAnimName = pCurAnim->Get_AnimName();

    for (auto iter = m_Events.begin(); iter != m_Events.end(); ++iter)
    {
        string strAnimName = iter->first;

        // m_Events의 애니매이션 이름과 다르다면 건너뛴다.
        if (strAnimName != strCurAnimName) continue;

        // 이름이 같은 애니매이션의 레이어 개수 만큼 Frame을 그린다.
        map<string, vector<pair<string, _float>>> LayerEvents = iter->second;
        for (_int i = 0; i < LayerEvents.size(); ++i)
        {
            auto iterLayer = LayerEvents.begin();
            advance(iterLayer, i);
            string strLayerName = iterLayer->first;
            vector<pair<string, _float>> vecEvents = iterLayer->second;

            // 이벤트가 하나도 없다면 해당 레이어 삭제
            if (vecEvents.empty())
            {
                LayerEvents.erase(iterLayer);
                return;
            }

            ImDrawList* pDrawList = ImGui::GetWindowDrawList();
            ImVec2 vCursorPos = ImGui::GetCursorScreenPos();

            _float fCurStartPosY = m_fFrameLineStartY;
            fCurStartPosY += (i + 1) * m_fFramePadding;
            _float fCurStartY = fCurStartPosY;

            ImGui::SetCursorScreenPos(ImVec2(vCursorPos.x, fCurStartY));
            ImGui::Text(strLayerName.c_str());

            // 구분선을 아래에 배치
            ImGui::SetCursorScreenPos(ImVec2(vCursorPos.x, fCurStartY + 10.f));
            ImGui::SeparatorText("");

            vCursorPos.y = fCurStartPosY;
            vCursorPos.y += m_fTimeLinePadding;
            _float fFrameStep = Draw_TimeLine(pDrawList, m_fTimeLineWidth, vCursorPos, m_fCurAnimDuration);

            Draw_Event(pDrawList, strAnimName, strLayerName, vCursorPos.y, fFrameStep);
        }
    }
}

void CLevel_AnimTool::Draw_Event(ImDrawList* pDrawList, string strCurAnimName, string strEventLayerName, _float fStartPosY, _float fFrameStep)
{
    const static _float fEventRadius = 7.f;
    static _bool bDragging = false;
    static _bool bRightClick = false;
    static string strFocusedEventName = "";
    static string strFocusedLayerName = "";

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos; // 현재 마우스 위치

    vector<pair<string, _float>>& vecLayerEvents = m_Events[strCurAnimName][strEventLayerName];

    // 비어 있다면 리턴
    if (vecLayerEvents.empty())
        return;

    for (_int i = 0; i < vecLayerEvents.size(); ++i)
    {
        string strEvName = vecLayerEvents[i].first; // i번 이벤트 이름
        _float fTrackPos = vecLayerEvents[i].second; // i번 이벤트의 track pos

        _float fEventPosX = Cal_TrackPosX(m_fFrameLineStartX, fTrackPos, fFrameStep); // i번 이벤트가 위치해야할 스크린 상 좌표 X
        _float2 CirClePos = { fEventPosX,  fStartPosY + m_fFrameLineLen / 2.f };        //  i번 이벤트 원 위치
        _float fMinX = CirClePos.x - fEventRadius / 2.f;
        _float fMaxX = CirClePos.x + fEventRadius / 2.f;
        _float fMinY = CirClePos.y - fEventRadius / 2.f;
        _float fMaxY = CirClePos.y + fEventRadius / 2.f;

        _bool bHover = (mousePos.x >= fMinX && mousePos.x <= fMaxX && mousePos.y >= fMinY && mousePos.y <= fMaxY);
        
        _float fNewPosX = fEventPosX;

        if (bHover)
        {
            ImGui::SetTooltip("EventName : %s\nTrackPos : %.2f", strEvName.c_str(), fTrackPos);

            // i번 이벤트 우클릭
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    bRightClick = true;
                    strFocusedEventName = strEvName;
                    strFocusedLayerName = strEventLayerName;
                }
            }
        }

        // 우클릭 이벤트 처리
        {
            if (bRightClick)
            {
                if (strFocusedEventName == strEvName && strFocusedLayerName == strEventLayerName)
                {
                    ImGui::OpenPopup("Edit Event");
                    if (ImGui::BeginPopup("Edit Event"))
                    {
                        if (ImGui::MenuItem("Edit Event Name"))
                        {
                            strFocusedEventName = "";
                            strFocusedLayerName = "";
                            bRightClick = false;
                        }

                        if (ImGui::MenuItem("Delete Event"))
                        {
                            auto iterErase = vecLayerEvents.begin() + i;
                            vecLayerEvents.erase(iterErase);

                            strFocusedEventName = "";
                            strFocusedLayerName = "";

                            bRightClick = false;
                        }

                        if (ImGui::MenuItem("Close"))
                        {
                            strFocusedEventName = "";
                            strFocusedLayerName = "";

                            bRightClick = false;
                        }

                        ImGui::EndPopup();
                    }
                }
            }
        }

        if (bHover && ImGui::IsMouseDown(0)) // 클릭을 유지하면 드래그 상태
        {
            if (!bDragging)
            {
                strFocusedEventName = strEvName; // 포커싱
                strFocusedLayerName = strEventLayerName;
                bDragging = true;
            }

            if (strFocusedEventName == strEvName && strFocusedLayerName == strEventLayerName)
            {
                _float fNewTrackPos = (mousePos.x - m_fFrameLineStartX) / fFrameStep;
                vecLayerEvents[i].second = fNewTrackPos;
                fNewPosX = Cal_TrackPosX(m_fFrameLineStartX, fNewTrackPos, fFrameStep);
            }
        }

        if (bDragging && ImGui::IsMouseDown(0)) // 클릭을 유지하면 드래그 상태
        {
            if (strFocusedEventName == strEvName && strFocusedLayerName == strEventLayerName)
            {
                _float fNewTrackPos = (mousePos.x - m_fFrameLineStartX) / fFrameStep;
                vecLayerEvents[i].second = fNewTrackPos;
                fNewPosX = Cal_TrackPosX(m_fFrameLineStartX, fNewTrackPos, fFrameStep);

                ImGui::SetTooltip("EventName : %s\nTrackPos : %.2f", strEvName.c_str(), fNewTrackPos);
            }
        }

        if (bDragging && !ImGui::IsMouseDown(0))
        {
            if (strFocusedEventName == strEvName && strFocusedLayerName == strEventLayerName)
            {
                _float fNewTrackPos = (mousePos.x - m_fFrameLineStartX) / fFrameStep;
                vecLayerEvents[i].second = fNewTrackPos;
                fNewPosX = Cal_TrackPosX(m_fFrameLineStartX, fNewTrackPos, fFrameStep);
            }

            bDragging = false;
            strFocusedEventName = "";
            strFocusedLayerName = "";
        }

        ImU32 color{};

        if (strFocusedEventName == strEvName)
            (bHover || bDragging) ? color = IM_COL32(0, 255, 0, 255) : color = IM_COL32(255, 0, 0, 255); // Hover중이거나 드래그 중일 때 초록색
        else
            bHover ? color = IM_COL32(0, 255, 0, 255) : color = IM_COL32(255, 0, 0, 255);

        pDrawList->AddCircleFilled(ImVec2(fNewPosX, fStartPosY + m_fFrameLineLen / 2.f), fEventRadius, color);
    }
}

_bool CLevel_AnimTool::Controll_WorldPos(CTransform* pCom)
{
    // right, up, look, pos 출력
    {
        _vector vRight = pCom->Get_State(CTransform::STATE_RIGHT);
        _vector vUp = pCom->Get_State(CTransform::STATE_UP);
        _vector vLook = pCom->Get_State(CTransform::STATE_LOOK);
        _vector vPos = pCom->Get_State(CTransform::STATE_POSITION);

        ImGui::Text("right : %.2f,  %.2f,  %.2f", vRight.m128_f32[0], vRight.m128_f32[1], vRight.m128_f32[2]);
        ImGui::Text("up    : %.2f,  %.2f,  %.2f", vUp.m128_f32[0], vUp.m128_f32[1], vUp.m128_f32[2]);
        ImGui::Text("look  : %.2f,  %.2f,  %.2f", vLook.m128_f32[0], vLook.m128_f32[1], vLook.m128_f32[2]);
        ImGui::Text("pos   : %.2f,  %.2f,  %.2f", vPos.m128_f32[0], vPos.m128_f32[1], vPos.m128_f32[2]);
    }

    // 위치
    {
        ImGui::SeparatorText("3-wide-world-pos");
        _vector vWorldPos = pCom->Get_State(CTransform::STATE_POSITION);

        static _float vCurObjPos[3] = { vWorldPos.m128_f32[0], vWorldPos.m128_f32[1], vWorldPos.m128_f32[2] };
        const static _float fMin = -100.f;
        const static _float fMax = 100.f;

        static _bool bChange = false;
        bChange |= ImGui::InputFloat3("input float3", m_vCurObjPosInput);
        bChange |= ImGui::DragFloat3("drag float3", m_vCurObjPosInput, 0.01f, fMin, fMax);
        _vector vRetPos = _vector{ vCurObjPos[0] + m_vCurObjPosInput[0], vCurObjPos[1] + m_vCurObjPosInput[1], vCurObjPos[2] + m_vCurObjPosInput[2], 1.f };

        // 위치 설정
        if (bChange)
            pCom->Set_State(CTransform::STATE_POSITION, vRetPos);

        bChange = false;
    }
    
    return true;
}

_bool CLevel_AnimTool::Controll_Rotation(CTransform* pCom)
{
    // 회전
    {
        const static _float fMin = -3.14f * 2.f;
        const static _float fMax = 3.14f * 2.f;

        m_vRightUpLook[0] = XMVector3Normalize(pCom->Get_State(CTransform::STATE_RIGHT));
        m_vRightUpLook[1] = XMVector3Normalize(pCom->Get_State(CTransform::STATE_UP));
        m_vRightUpLook[2] = XMVector3Normalize(pCom->Get_State(CTransform::STATE_LOOK));

        ImGui::SeparatorText("3-wide-rot");

        // InputFloat3와 DragFloat3의 반환값으로 변경 여부 확인
        static _bool changed = false;

        changed |= ImGui::InputFloat3("input rot float3", m_fCurRotVal);
        changed |= ImGui::DragFloat3("drag rot float3", m_fCurRotVal, 0.01f, fMin, fMax);

        // 입력 값 중 변경된 값만 회전 호출
        for (_int i = 0; i < 3; ++i)
        {
            if (m_fCurRotVal[i] != m_fPrevRotVal[i])
            {
                // 이전 값과 다를 경우에만 호출
                if (i == 0)
                    pCom->Rotation(m_vRightUpLook[0], m_fCurRotVal[0]); // Right 축 회전
                else if (i == 1)
                    pCom->Rotation(m_vRightUpLook[1], m_fCurRotVal[1]); // Up 축 회전
                else if (i == 2)
                    pCom->Rotation(m_vRightUpLook[2], m_fCurRotVal[2]); // Look 축 회전

                m_fPrevRotVal[i] = m_fCurRotVal[i]; // 이전 값 업데이트
            }
        }
    }

    return true;
}

_bool CLevel_AnimTool::Controll_Scale(CTransform* pCom)
{
    // 스케일링
    ImGui::SeparatorText("3-wide-scale");
    ImGui::Text("scale x : %.2f, scale y : %.2f, scale z : %.2f", m_fCurScale, m_fCurScale[1], m_fCurScale[2]);
    ImGui::InputFloat3("input scale float3", m_fCurScale);
    ImGui::DragFloat3("drag scale float3", m_fCurScale, 0.01f, 0.001f, 1000.f);
    pCom->Scaling(m_fCurScale[0], m_fCurScale[1], m_fCurScale[2]);

    return true;
}

// 단순 right up look pos 로컬 정보만 출력한다.
void CLevel_AnimTool::Show_BoneControll_Default(string strBoneName)
{
    _float4x4* pMatLocal = m_BoneTransformationMatrices.find(strBoneName)->second;

    _matrix BoneLocalMatrix = XMLoadFloat4x4(&*pMatLocal);
    _vector vRight = BoneLocalMatrix.r[0];
    _vector vUp = BoneLocalMatrix.r[1];
    _vector vLook = BoneLocalMatrix.r[2];
    _vector vPos = BoneLocalMatrix.r[3];

    ImGui::SeparatorText("bone local right-up-look-pos");
    ImGui::Text("right : %.4f,  %.4f,  %.4f", vRight.m128_f32[0], vRight.m128_f32[1], vRight.m128_f32[2]);
    ImGui::Text("up    : %.4f,  %.4f,  %.4f", vUp.m128_f32[0], vUp.m128_f32[1], vUp.m128_f32[2]);
    ImGui::Text("look  : %.4f,  %.4f,  %.4f", vLook.m128_f32[0], vLook.m128_f32[1], vLook.m128_f32[2]);
    ImGui::Text("pos   : %.4f,  %.4f,  %.4f", vPos.m128_f32[0], vPos.m128_f32[1], vPos.m128_f32[2]);
}

// 업데이트에서 제외하기로한 뼈의 초기설정 및 컨트롤 할 수 있는 기능 만들기
void CLevel_AnimTool::Show_BoneControll_Exclude(string strBoneName)
{
    const static _float fMin = -3.f;
    const static _float fMax = 3.f;

    auto iter = m_BoneCheckUpdateExcludeBone.find(strBoneName);

    // m_BoneCheckUpdateExcludeBone에 없는 경우 처음 설정
    if (iter != m_BoneCheckUpdateExcludeBone.end())
        m_BoneCheckUpdateExcludeBone.emplace(strBoneName, false); // 처음에는 false로 넣는다.

    // Update Exclude 체크박스를 확인을 누르기 전 까지는 정보를 그대로 출력해준다
    _float4x4* pMatLocal = m_BoneTransformationMatrices.find(strBoneName)->second;

    _matrix BoneLocalMatrix = XMLoadFloat4x4(&*pMatLocal);
    _vector vRight = BoneLocalMatrix.r[0];
    _vector vUp = BoneLocalMatrix.r[1];
    _vector vLook = BoneLocalMatrix.r[2];
    _vector vPos = BoneLocalMatrix.r[3];

    ImGui::SeparatorText("bone local-right-up-look");
    ImGui::Text("right : %.4f,  %.4f,  %.4f", vRight.m128_f32[0], vRight.m128_f32[1], vRight.m128_f32[2]);
    ImGui::Text("up    : %.4f,  %.4f,  %.4f", vUp.m128_f32[0], vUp.m128_f32[1], vUp.m128_f32[2]);
    ImGui::Text("look  : %.4f,  %.4f,  %.4f", vLook.m128_f32[0], vLook.m128_f32[1], vLook.m128_f32[2]);
    ImGui::Text("pos   : %.4f,  %.4f,  %.4f", vPos.m128_f32[0], vPos.m128_f32[1], vPos.m128_f32[2]);

    // 해당 뼈를 애니매이션 업데이트에서 제외할지 말지 여부 체크 박스
    _bool bUpdateExcludeCheck = m_BoneCheckUpdateExcludeBone[strBoneName];
    ImGui::Checkbox("Update Exclude This Bone From Animation", &bUpdateExcludeCheck);
    {
        // 해당 뼈를 애니매이션 업데이트에서 제외 하기로 한 경우
        // 현재 뼈 업데이트를 막는다.
        _bool& bOriginVal = m_BoneCheckUpdateExcludeBone[strBoneName];
        if (bUpdateExcludeCheck && !bOriginVal)
        {
            // 뼈 체크 표시 처리
            {
                // 하나만 true로 가능하기 때문에 이전에 true로 체크해놓은게 있다면 전부다 false로 해준다.
                for (auto iter = m_BoneCheckUpdateExcludeBone.begin(); iter != m_BoneCheckUpdateExcludeBone.end(); ++iter)
                {
                    if (iter->second)
                        iter->second = false;
                }
                // 위의 for문에서 false가 되었기 때문에 다시 true로 만들어준다.
                bOriginVal = true;
            }
 
            // 원본 TransformationMatrix를 다 날려버린다.
            m_BoneOriginTransformationMatrices.clear();

            // 제외하기로한 뼈의 로컬 상태 행렬을 저장한다.
            m_BoneOriginTransformationMatrices[strBoneName] = *pMatLocal;

            // 업데이트에서 제외할 뼈 로컬 행렬 설정
            m_pAT_PartAnimObj_Model->Set_UpdateExcludeBone_TransformationMatrix(XMLoadFloat4x4(pMatLocal));

            // 업데이트에서 제외할 뼈 이름 설정
            m_pAT_PartAnimObj_Model->Set_UpdateExcludeBoneName(strBoneName);
        }
        // 애니매이션 업데이트에서 제외 하기로한 경우가 아닌 경우
        else if (!bUpdateExcludeCheck)
        {
            m_BoneCheckUpdateExcludeBone[strBoneName] = false;

            // 원본 뼈 로컬 TransformationMatrix를 없앤다
            auto iter = m_BoneOriginTransformationMatrices.find(strBoneName);
            if (iter != m_BoneOriginTransformationMatrices.end())
                m_BoneOriginTransformationMatrices.erase(iter);

            // 업데이트에서 제외할 뼈 이름을 항등으로 한다.
            m_pAT_PartAnimObj_Model->Set_UpdateExcludeBone_TransformationMatrix(XMMatrixIdentity());

            // 업데이트에서 제외할 뼈 이름을 None으로 설정한다.
            m_pAT_PartAnimObj_Model->Set_UpdateExcludeBoneName("None");
        }
    }

    ImGui::Checkbox("Show Ray", &m_bShowDirBoneToTarget);

    ImGui::SameLine();

    // 현재 커서 위치를 가져옴
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    // 아이콘을 그리기 위한 DrawList 가져오기
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // 네모 그리기
    drawList->AddRectFilled
    (
        ImVec2(cursorPos.x, cursorPos.y),                       // 네모의 왼쪽 위 좌표
        ImVec2(cursorPos.x + 20.f, cursorPos.y + 20.f), // 네모의 오른쪽 아래 좌표
        ImColor(icons[3].color)                                // 색상
    );

    // 네모 오른쪽으로 텍스트 출력
    ImGui::Dummy(ImVec2(iconSize, iconSize)); // 네모의 크기만큼 공간 확보
    ImGui::SameLine();
    ImGui::Text("%s", icons[3].text);


    // 월드상에 존재하는 타겟을 향해 현재 애니매이션 업데이트에서 제외된 뼈의 룩 방향 벡터를 보여주기로 한 경우
    if (m_bShowDirBoneToTarget)
    {
        string strCurBoneName = m_ShowDirRay.first;
        CAT_Ray* pRay = m_ShowDirRay.second;

        // 없다면 m_RayVec를 순회해서 사용하지 않는 레이를 찾아서 m_ShowDirRay에 넣는다.
        if (strCurBoneName == "None")
        {
            for (_int i = 0; i < m_RayVec.size(); ++i)
            {
                if (!m_RayVec[i]->Get_Use())
                {
                    m_RayVec[i]->Set_Use(true); // 사용중이라 변경한다.
                    m_RayVec[i]->Set_Color(_float3{ 1.f, 0.f, 1.f }); // 마젠타

                    m_ShowDirRay.first = strBoneName;
                    m_ShowDirRay.second = m_RayVec[i];
                    break;
                }
            }
        }
        // 있다면 보여준다.
        else
        {
            if (strCurBoneName != strBoneName)
                m_ShowDirRay.first = strBoneName;

            Show_IK_Ray(m_ShowDirRay.first, m_ShowDirRay.second, IK_WORLD_TO_TARGET);
        }
    }
    // 보여주는 경우가 아닌 경우
    else
    {
        string strCurBoneName = m_ShowDirRay.first;
        CAT_Ray* pRay = m_ShowDirRay.second;

        if (strCurBoneName != "None" && pRay)
        {
            pRay->Set_Use(false);
            m_ShowDirRay.first = "None";
            m_ShowDirRay.second = nullptr;
        }
    }
    

    //_bool& bChange = m_BoneMatrixSRTChange[strBoneName];
    //ImGui::Checkbox("Change Bone SRT", &bChange);

    //// SRT 변경하는 경우
    //if (bChange)
    //{

    //    // 위치 설정
    //    ImGui::SeparatorText("Set Pos");
    //    {
    //        _float4x4 OriginBoneTransformationMatrix = m_BoneOriginTransformationMatrices[strBoneName];
    //        _matrix MatTemp{};
    //        MatTemp = XMLoadFloat4x4(&OriginBoneTransformationMatrix);
    //        _vector vBoneOriginPos = MatTemp.r[3];

    //        if (ImGui::InputFloat3("input pos float3", MatPosInput))
    //        {
    //            _vector vRetPos = _vector{ XMVectorGetX(vBoneOriginPos) + MatPosInput[0], XMVectorGetY(vBoneOriginPos) + MatPosInput[1], XMVectorGetZ(vBoneOriginPos) + MatPosInput[2], 1.f };
    //            MatTemp.r[3] = vRetPos;
    //            m_pAT_PartAnimObj_Model->Set_UpdateExcludeBone_TransformationMatrix(MatTemp);
    //        }

    //        // true를 반환 == 값이 변경된 경우에만
    //        if (ImGui::DragFloat3("drag pos float3", MatPosInput, 0.01f, fMin, fMax))
    //        {
    //            _vector vRetPos = _vector{ XMVectorGetX(vBoneOriginPos) + MatPosInput[0], XMVectorGetY(vBoneOriginPos) + MatPosInput[1], XMVectorGetZ(vBoneOriginPos) + MatPosInput[2], 1.f };
    //            MatTemp.r[3] = vRetPos;
    //            m_pAT_PartAnimObj_Model->Set_UpdateExcludeBone_TransformationMatrix(MatTemp);
    //        }
    //    }

    //    // 회전 설정
    //    ImGui::SeparatorText("Set Rot");
    //    {
    //        _float4x4 OriginBoneTransformationMatrix = m_BoneOriginTransformationMatrices[strBoneName];
    //        _matrix MatTemp{};
    //        MatTemp = XMLoadFloat4x4(&OriginBoneTransformationMatrix);

    //        _vector vRightOri = MatTemp.r[0];
    //        _vector vUpOri = MatTemp.r[1];
    //        _vector vLookOri = MatTemp.r[2];

    //        auto& RotInput = m_BoneMatrixRotInput[strBoneName];

    //        if (ImGui::InputFloat3("input rot float3 XYZ Radian", RotInput))
    //        {
    //            // 각축에 대한 회전 행렬을 만든다.
    //            // 입력된 회전값을 바탕으로 회전행렬 생성
    //            _matrix RotXMatrix = XMMatrixRotationAxis(vRightOri, RotInput[0]);
    //            _matrix RotYMatrix = XMMatrixRotationAxis(vUpOri, RotInput[1]);
    //            _matrix RotZMatrix = XMMatrixRotationAxis(vLookOri, RotInput[2]);

    //            // 중요한 개념이 들어간다.
    //            // - 원본 행렬(MatTemp) * RotMatrix를 곱하는데 원본 행렬의 각 축을 RotMatrix랑 곱해 회전 시킨다.
    //            _matrix RotMatrix = RotXMatrix * RotYMatrix * RotZMatrix;
    //            _matrix RetMat = MatTemp * RotMatrix;

    //            // SRT이기 때문에 위치는 영향을 안 받는다. 이동값을 바로 넣는다.
    //            RetMat.r[3] = vPos;
    //            m_pAT_PartAnimObj_Model->Set_UpdateExcludeBone_TransformationMatrix(RetMat);
    //        }

    //        // true를 반환 == 값이 변경된 경우에만
    //        if (ImGui::DragFloat3("drag rot float3 XYZ Radian", RotInput, 0.01f, -XM_PI, XM_PI))
    //        {
    //            // 각축에 대한 회전 행렬을 만든다.
    //            _matrix RotXMatrix = XMMatrixRotationAxis(vRightOri, RotInput[0]);
    //            _matrix RotYMatrix = XMMatrixRotationAxis(vUpOri, RotInput[1]);
    //            _matrix RotZMatrix = XMMatrixRotationAxis(vLookOri, RotInput[2]);

    //            // 중요한 개념이 들어간다.
    //            // - 원본 행렬(MatTemp) * RotMatrix를 곱하는데 원본 행렬의 각 축을 RotMatrix랑 곱해 회전 시킨다.
    //            _matrix RotMatrix = RotXMatrix * RotYMatrix * RotZMatrix;
    //            _matrix RetMat = MatTemp * RotMatrix;

    //            // SRT이기 때문에 위치는 영향을 안 받는다. 이동값을 바로 넣는다.
    //            RetMat.r[3] = vPos;

    //            m_pAT_PartAnimObj_Model->Set_UpdateExcludeBone_TransformationMatrix(RetMat);
    //        }
    //    }
    //}
}

void CLevel_AnimTool::Controll_Bone_Matrix(string strBoneName)
{
    ImGui::SeparatorText("Bone Controll Type");
    static const _char* pBoneCombo[] = { "Default Controll", "Exclude Bone"};
    static _int iSelBoneConType = 0;
    const _char* pSelBoneConTypeName = pBoneCombo[iSelBoneConType];

    if (ImGui::BeginCombo("Select Bone Controll Type", pSelBoneConTypeName))
    {
        for (int n = 0; n < IM_ARRAYSIZE(pBoneCombo); n++)
        {
            const bool is_selected = (iSelBoneConType == n);
            if (ImGui::Selectable(pBoneCombo[n], is_selected))
            {
                m_eBoneControllType = BONE_CONTROLL_TYPE(n);
                iSelBoneConType = n;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    
    switch (m_eBoneControllType)
    {
    case Client::CLevel_AnimTool::BONE_DEFAULT:
        Show_BoneControll_Default(strBoneName);
        break;
    case Client::CLevel_AnimTool::BONE_EXCLUDE:
    {
        Show_BoneControll_Exclude(strBoneName);
    }
        break;
    default:
        break;
    }

}

// 엑티브 bIsUse가 true인 이름의 뼈에 해당하는 레이의 위치를 설정하고 렌더링 해야한다.
void CLevel_AnimTool::Active_Ray_AtBone()
{
    const static _float3 vRed = { 1.f, 0.f, 0.f };
    const static _float3 vGreen = { 0.f, 1.f, 0.f };
    const static _float3 vBlue = { 0.f, 0.f, 1.f };
    const _int iRayMaxSize = 30;

    _int iCnt = 0;
    for (_int i = 0; i < iRayMaxSize; ++i)
        if (m_RayVec[i]->Get_Use()) ++iCnt;

    if (iCnt >= 30)
        return;

    for (auto iter = m_UsingRaysPlayerLook.begin(); iter != m_UsingRaysPlayerLook.end(); ++iter)
    {
        string strBoneName = iter->first;
        _bool bIsUse = iter->second;

        if (bIsUse)
        {
            auto iterF = m_RaysPlayerLook.find(strBoneName);
            if (iterF != m_RaysPlayerLook.end())
            {
                _bool bUse = iterF->second->Get_Use();
                if (bUse)
                {
                    CAT_Ray* pRay = iterF->second;
                    if (pRay)
                        Show_IK_Ray(strBoneName, pRay, IK_PLAYER_LOOK);
                }
            }
            else
            {
                // m_RayVec을 순회해서 사용중이지 않은 Ray객체를 찾아 사용중으로 바꾸고 렌더링 한다.
                for (_int i = 0; i < m_RayVec.size(); ++i)
                {
                    if (!m_RayVec[i]->Get_Use())
                    {
                        auto iterFind = m_RaysPlayerLook.find(strBoneName);
                        if (iterFind == m_RaysPlayerLook.end()) // m_RaysPlayerLook 에 없는 경우 추가한다.
                        {
                            m_RayVec[i]->Set_Use(true);
                            m_RayVec[i]->Set_Color(vRed);

                            m_RaysPlayerLook.emplace(strBoneName, m_RayVec[i]);
                            break;
                        }
                        else
                        {
                            MSG_BOX("없다");
                        }
                    }
                }
            }
        }
        // 해당 뼈가 Ray를 사용중이 아니라고 설정한 경우
        else
        {
            auto iterFind = m_RaysPlayerLook.find(strBoneName);
            if (iterFind != m_RaysPlayerLook.end())
            {
                iterFind->second->Set_Use(false);
                m_RaysPlayerLook.erase(iterFind);
            }
        }
    }

    
    for (auto iter = m_UsingRaysBoneLook.begin(); iter != m_UsingRaysBoneLook.end(); ++iter)
    {
        string strBoneName = iter->first;
        _bool bIsUse = iter->second;

        if (bIsUse)
        {
            auto iterF = m_RaysBoneLook.find(strBoneName);
            if (iterF != m_RaysBoneLook.end())
            {
                _bool bUse = iterF->second->Get_Use();
                if (bUse)
                {
                    CAT_Ray* pRay = iterF->second;
                    if (pRay)
                        Show_IK_Ray(strBoneName, pRay, IK_BONE_WORLD_LOOK);
                }
            }
            else
            {
                // m_RayVec을 순회해서 사용중이지 않은 Ray객체를 찾아 사용중으로 바꾸고 렌더링 한다.
                for (_int i = 0; i < m_RayVec.size(); ++i)
                {
                    if (!m_RayVec[i]->Get_Use())
                    {
                        auto iterFind = m_RaysBoneLook.find(strBoneName);
                        if (iterFind == m_RaysBoneLook.end()) // m_RaysPlayerLook 에 없는 경우 추가한다.
                        {
                            m_RayVec[i]->Set_Use(true);
                            m_RayVec[i]->Set_Color(vBlue);

                            m_RaysBoneLook.emplace(strBoneName, m_RayVec[i]);
                            break;
                        }
                        else
                        {
                            MSG_BOX("없다");
                        }
                    }
                }
            }
        }
        // 해당 뼈가 Ray를 사용중이 아니라고 설정한 경우
        else
        {
            auto iterFind = m_RaysBoneLook.find(strBoneName);
            if (iterFind != m_RaysBoneLook.end())
            {
                iterFind->second->Set_Use(false);
                m_RaysBoneLook.erase(iterFind);
            }
        }
    }


    for (auto iter = m_UsingRaysBoneLocalLook.begin(); iter != m_UsingRaysBoneLocalLook.end(); ++iter)
    {
        string strBoneName = iter->first;
        _bool bIsUse = iter->second;

        if (bIsUse)
        {
            auto iterF = m_RaysBoneLocalLook.find(strBoneName);
            if (iterF != m_RaysBoneLocalLook.end())
            {
                _bool bUse = iterF->second->Get_Use();
                if (bUse)
                {
                    CAT_Ray* pRay = iterF->second;
                    if (pRay)
                        Show_IK_Ray(strBoneName, pRay, IK_BONE_LOCAL_LOOK);
                }
            }
            else
            {
                // m_RayVec을 순회해서 사용중이지 않은 Ray객체를 찾아 사용중으로 바꾸고 렌더링 한다.
                for (_int i = 0; i < m_RayVec.size(); ++i)
                {
                    if (!m_RayVec[i]->Get_Use())
                    {
                        auto iterFind = m_RaysBoneLocalLook.find(strBoneName);
                        if (iterFind == m_RaysBoneLocalLook.end()) // m_RaysPlayerLook 에 없는 경우 추가한다.
                        {
                            m_RayVec[i]->Set_Use(true);
                            m_RayVec[i]->Set_Color(vGreen);

                            m_RaysBoneLocalLook.emplace(strBoneName, m_RayVec[i]);
                            break;
                        }
                        else
                        {
                            MSG_BOX("없다");
                        }
                    }
                }
            }
        }
        // 해당 뼈가 Ray를 사용중이 아니라고 설정한 경우
        else
        {
            auto iterFind = m_RaysBoneLocalLook.find(strBoneName);
            if (iterFind != m_RaysBoneLocalLook.end())
            {
                iterFind->second->Set_Use(false);
                m_RaysBoneLocalLook.erase(iterFind);
            }
        }
    }
}


void CLevel_AnimTool::Show_Selectable_AnimList_ListBox()
{
    if (!m_AnimNamesCStr.empty())
    {
        ImGui::SeparatorText("Default Test");

        if (ImGui::BeginListBox("Selectable Anim List", ImVec2(-FLT_MIN, 8 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (_int i = 0; i < m_AnimNamesCStr.size(); ++i)
            {
                _bool IsSelected = (m_iCurAnimIdx == i);
                string strFormat = "idx : " + to_string(i) + " | ";
                string strRet = strFormat + m_AnimNamesCStr[i];

                if (ImGui::Selectable(strRet.c_str(), IsSelected))
                {
                    PlayAnimTest_StartBtn(i, PartAnimObj_Type::ANIM_PLAY_TYPE::DEFAULT);
                }

                // 선택된 항목에 기본 포커스를 설정
                if (IsSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndListBox();
        }
    }
}

void CLevel_AnimTool::Show_PlayAnim_Btn(const _char* leftLabel, const _char* midLabel, const _char* rightLabel, ImVec2 buttonSize)
{
    // 버튼 사이즈 계산
    _float windowWidth = ImGui::GetWindowWidth();
    _float padding = ImGui::GetStyle().ItemSpacing.x; // 버튼 간의 기본 간격
    _float buttonWidth = (windowWidth - padding * 4) / 3; // 버튼 3개와 간격 계산
    ImVec2 _buttonSize(buttonWidth, 30.0f); // 버튼 높이는 30.0f로 설정

    // Left button
    if (ImGui::Button(leftLabel, _buttonSize))
    {
        // 왼쪽 버튼이 눌렸을 때 실행될 코드
        if (0 > m_iCurAnimIdx - 1)
            return;

        m_pAT_PartAnimObj->Set_CurAnimIdx(--m_iCurAnimIdx, 1);
    }

    // Mid button (중앙에 배치)
    ImGui::SameLine();
    if (ImGui::Button(midLabel, _buttonSize) || KEY_DOWN(KEY::SPACE))
    {
        m_pAT_PartAnimObj->StopAndPlay_CurAnim(); // 애니매이션 재생 멈춘다.

        // 중앙 버튼이 눌렸을 때 실행될 코드
        m_pAT_PartAnimObj->StopAndPlay_CurAnim();
    }

    // Right button
    ImGui::SameLine();
    if (ImGui::Button(rightLabel, _buttonSize))
    {
        // 오른쪽 버튼이 눌렸을 때 실행될 코드
        if (m_AnimNamesCStr.size() <= m_iCurAnimIdx + 1)
            return;

        m_pAT_PartAnimObj->Set_CurAnimIdx(++m_iCurAnimIdx, 1);
    }
}

void CLevel_AnimTool::Play_Anim(_int iAnimIdx, _bool bLoop)
{
    m_pAT_PartAnimObj_Model->SetUp_Animation(iAnimIdx, bLoop);
}

CLevel_AnimTool* CLevel_AnimTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_AnimTool* pInstance = new CLevel_AnimTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_AnimTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_AnimTool::Free()
{
	__super::Free();

    delete m_pAnimToolUtilObj;
    m_pAnimToolUtilObj = nullptr;

    Safe_Release(m_pAT_Obj);

    for (_int i = 0; m_RayVec.size(); ++i)
        Safe_Release(m_RayVec[i]);
}

#include "..\Public\GameInstance.h"

#include "Frustum.h"
#include "Shadow.h"
#include "Height.h"
#include "Picking.h"
#include "Renderer.h"
#include "PipeLine.h"
#include "Input_Device.h"
#include "Font_Manager.h"
#include "Light_Manager.h"
#include "Timer_Manager.h"
#include "Level_Manager.h"
#include "Graphic_Device.h"
#include "Object_Manager.h"
#include "Target_Manager.h"
#include "Prototype_Manager.h"

#ifdef _DEBUG

#include "Imgui_Manager.h"
#include "Basic_Tool.h"

#endif




IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()
{
	
}

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.isWindowed, EngineDesc.iViewportWidth, EngineDesc.iViewportHeight, ppDevice, ppContext);
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pLight_Manager = CLight_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	m_pInput_Device = CInput_Device::Create(EngineDesc.hInstance, EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return E_FAIL;

	m_pPipeLine = CPipeLine::Create();
	if (nullptr == m_pPipeLine)
		return E_FAIL;
	
	m_pTarget_Manager = CTarget_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	m_pRenderer = CRenderer::Create(*ppDevice, *ppContext);
	if (nullptr == m_pRenderer)
		return E_FAIL;

	m_pPrototype_Manager = CPrototype_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPrototype_Manager)
		return E_FAIL;

	m_pObject_Manager = CObject_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	m_pFont_Manager = CFont_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	m_pPicking = CPicking::Create(*ppDevice, *ppContext, EngineDesc.hWnd);
	if (nullptr == m_pPicking)
		return E_FAIL;

	m_pHeight = CHeight::Create(*ppDevice, *ppContext);
	if (nullptr == m_pHeight)
		return E_FAIL;

	m_pShadow = CShadow::Create(*ppDevice, *ppContext);
	if (nullptr == m_pShadow)
		return E_FAIL;

	m_pFrustum = CFrustum::Create();
	if (nullptr == m_pFrustum)
		return E_FAIL;

	m_pKey_Manager = CKey_Manager::Create(EngineDesc.hInstance, EngineDesc.hWnd);
	if (nullptr == m_pKey_Manager)
		return E_FAIL;

#ifdef _DEBUG
	m_pImgui_Manager = CImgui_Manager::Create(*ppDevice, *ppContext, EngineDesc.hWnd);
	if (nullptr == m_pImgui_Manager)
		return E_FAIL;

	m_pBasic_Tool = CBasic_Tool::Create();
	if (nullptr == m_pBasic_Tool)
		return E_FAIL;
#endif 


	return S_OK;
}

void CGameInstance::Update_Engine(_float fTimeDelta)
{
	m_pKey_Manager->Update();

	m_pInput_Device->Update_InputDev();

	m_pPicking->Update();
	m_pHeight->Update();

#ifdef _DEBUG
	m_pImgui_Manager->Update();
#endif

	m_pObject_Manager->Enable_Update();

	m_pObject_Manager->Priority_Update(fTimeDelta);

	m_pPipeLine->Update();
	m_pFrustum->Update();

	m_pObject_Manager->Update(fTimeDelta);
	m_pObject_Manager->Late_Update(fTimeDelta);

	m_pBasic_Tool->Update(fTimeDelta);

	m_pLevel_Manager->Update(fTimeDelta);

	
}

HRESULT CGameInstance::Render_Begin(const _float4 & vClearColor)
{
	m_pGraphic_Device->Clear_BackBuffer_View(vClearColor);
	m_pGraphic_Device->Clear_DepthStencil_View();
	
	return S_OK;
}

HRESULT CGameInstance::Draw()
{
	m_pRenderer->Draw_RenderObject();

	m_pLevel_Manager->Render();

#ifdef _DEBUG
	m_pImgui_Manager->Render();
#endif

	m_pObject_Manager->Disable_Update();

	return S_OK;
}

HRESULT CGameInstance::Render_End()
{
	return m_pGraphic_Device->Present();
}

void CGameInstance::Clear(_int iLevelID)
{
	/* 특정 레벨용 객체들을 지운다. */
	m_pObject_Manager->Clear(iLevelID);

	m_pPrototype_Manager->Clear(iLevelID);

}

_float CGameInstance::Compute_Random_Normal()
{
	return (_float)rand() / RAND_MAX;	
}

_float CGameInstance::Compute_Random(_float fMin, _float fMax)
{	
	return fMin + (fMax - fMin) * Compute_Random_Normal();	
}

_byte CGameInstance::Get_DIKeyState(_ubyte byKeyID)
{
	return m_pInput_Device->Get_DIKeyState(byKeyID);	
}

_byte CGameInstance::Get_DIMouseState(MOUSEKEYSTATE eMouse)
{
	return m_pInput_Device->Get_DIMouseState(eMouse);
}

_long CGameInstance::Get_DIMouseMove(MOUSEMOVESTATE eMouseState)
{
	return m_pInput_Device->Get_DIMouseMove(eMouseState);
}


_float CGameInstance::Get_TimeDelta(const _wstring & strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return 0.f;

	return m_pTimer_Manager->Get_TimeDelta(strTimerTag);
}

HRESULT CGameInstance::Add_Timer(const _wstring & strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	return m_pTimer_Manager->Add_Timer(strTimerTag);
}

void CGameInstance::Update_TimeDelta(const _wstring & strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return;

	return m_pTimer_Manager->Update_TimeDelta(strTimerTag);
}

HRESULT CGameInstance::Add_RenderObject(CRenderer::RENDERGROUP eRenderGroup, CGameObject * pRenderObject)
{
	if (nullptr == m_pRenderer)
		return E_FAIL;

	return m_pRenderer->Add_RenderObject(eRenderGroup, pRenderObject);
}

HRESULT CGameInstance::Add_DebugComponent(CComponent * pDebugCom)
{
	if (nullptr == m_pRenderer)
		return E_FAIL;

	return m_pRenderer->Add_DebugComponent(pDebugCom);
}

HRESULT CGameInstance::Open_Level(_int iLevelIndex, CLevel * pNewLevel)
{
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	return m_pLevel_Manager->Open_Level(iLevelIndex, pNewLevel);
}

HRESULT CGameInstance::Add_Prototype(_uint iLevelIndex, const _wstring & strPrototypeTag, CBase * pPrototype)
{
	if (nullptr == m_pPrototype_Manager)
		return E_FAIL;

	return m_pPrototype_Manager->Add_Prototype(iLevelIndex, strPrototypeTag, pPrototype);
}

CBase * CGameInstance::Clone_Prototype(Engine::PROTOTYPE eType, _uint iLevelIndex, const _wstring & strPrototypeTag, void * pArg)
{
	if (nullptr == m_pPrototype_Manager)
		return nullptr;

	return m_pPrototype_Manager->Clone_Prototype(eType, iLevelIndex, strPrototypeTag, pArg);
}

HRESULT CGameInstance::Add_GameObject_ToLayer(_uint iPrototypeLevelIndex, const _wstring & strPrototypeTag, _uint iLevelIndex, const _wstring & strLayerTag, void * pArg, const _wstring& strObjTag)
{	
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Add_GameObject_ToLayer(iPrototypeLevelIndex, strPrototypeTag, iLevelIndex, strLayerTag, pArg, strObjTag);
}

//CComponent * CGameInstance::Get_Component(_uint iLevelIndex, const _wstring & strLayerTag, const _wstring & strComponentTag, _uint iObjectIndex)
//{
//	if (nullptr == m_pObject_Manager)
//		return nullptr;
//
//	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strComponentTag, iObjectIndex);
//}
//
//CComponent * CGameInstance::Get_Component(_uint iLevelIndex, const _wstring & strLayerTag, _uint iPartObjectIndex, const _wstring & strPartComponentTag, _uint iObjectIndex)
//{
//	if (nullptr == m_pObject_Manager)
//		return nullptr;
//
//	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, iPartObjectIndex, strPartComponentTag, iObjectIndex);
//}

CGameObject* CGameInstance::Find_Object_By_ID(_uint iLevelIndex, const _wstring& strLayerTag, _uint iID)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Find_Object_By_ID(iLevelIndex, strLayerTag, iID);
}

CGameObject* CGameInstance::Find_Object_By_Name(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strName)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Find_Object_By_Name(iLevelIndex, strLayerTag, strName);
}

void CGameInstance::Set_Transform(CPipeLine::D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix)
{
	if (nullptr == m_pPipeLine)
		return;

	return m_pPipeLine->Set_Transform(eState, TransformMatrix);
}

_matrix CGameInstance::Get_TransformMatrix(CPipeLine::D3DTRANSFORMSTATE eState)
{
	if (nullptr == m_pPipeLine)
		return XMMatrixIdentity();

	return m_pPipeLine->Get_TransformMatrix(eState);
}

_float4x4 CGameInstance::Get_TransformFloat4x4(CPipeLine::D3DTRANSFORMSTATE eState)
{
	if (nullptr == m_pPipeLine)
		return _float4x4();

	return m_pPipeLine->Get_TransformFloat4x4(eState);
}

_float4x4 CGameInstance::Get_TransformFloat4x4_Inverse(CPipeLine::D3DTRANSFORMSTATE eState)
{
	if (nullptr == m_pPipeLine)
		return _float4x4();

	return m_pPipeLine->Get_TransformFloat4x4_Inverse(eState);
}


_matrix CGameInstance::Get_TransformMatrix_Inverse(CPipeLine::D3DTRANSFORMSTATE eState)
{
	if (nullptr == m_pPipeLine)
		return XMMatrixIdentity();

	return m_pPipeLine->Get_TransformMatrix_Inverse(eState);
}

const _float4 * CGameInstance::Get_CamPosition() const
{
	if (nullptr == m_pPipeLine)
		return nullptr;

	return m_pPipeLine->Get_CamPosition();
}

string CGameInstance::ConvertToString(wstring strName)
{
	if (nullptr == m_pPipeLine)
		return "";

	return m_pPipeLine->ConvertToString(strName);
}

wstring CGameInstance::ConvertToWString(string strName)
{
	if (nullptr == m_pPipeLine)
		return L"";

	return m_pPipeLine->ConvertToWString(strName);
}

HRESULT CGameInstance::Remove_Object(_uint iCurLevelIdx, const _wstring& strObjTag)
{
	if (!m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Remove_Object(iCurLevelIdx, strObjTag);
}

const KEY_STATE& CGameInstance::GetKeyState(KEY _eKEY)
{
	if (nullptr == m_pKey_Manager)
		return KEY_STATE::NONE;

	return m_pKey_Manager->GetKeyState(_eKEY);
}

const KEY_STATE& CGameInstance::GetMouseKeyState(MOUSE_KEY _eMouse)
{
	if (nullptr == m_pKey_Manager)
		return KEY_STATE::NONE;

	return m_pKey_Manager->GetMouseKeyState(_eMouse);
}

_long CGameInstance::Get_DIMouseMove(MOUSE_MOVE eMouseMove)
{
	if (nullptr == m_pKey_Manager)
		return 0;

	return m_pKey_Manager->Get_DIMouseMove(eMouseMove);
}

_bool CGameInstance::Check_Cursor_Position(RECT _tRect)
{
	return m_pKey_Manager->Check_Cursor_Position(_tRect);
}

POINT CGameInstance::Get_MousePos()
{
	return m_pKey_Manager->Get_MousePos();
}
HRESULT CGameInstance::Add_Light(const LIGHT_DESC & LightDesc)
{
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	return m_pLight_Manager->Add_Light(LightDesc);
}

const LIGHT_DESC * CGameInstance::Get_LightDesc(_uint iIndex) const
{
	if (nullptr == m_pLight_Manager)
		return nullptr;

	return m_pLight_Manager->Get_LightDesc(iIndex);
}

HRESULT CGameInstance::Render_Lights(CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{

	return m_pLight_Manager->Render(pShader, pVIBuffer);
}

HRESULT CGameInstance::Add_Font(const _wstring & strFontTag, const _tchar * pFontFilePath)
{

	return m_pFont_Manager->Add_Font(strFontTag, pFontFilePath);
}

HRESULT CGameInstance::Render_Font(const _wstring & strFontTag, const _tchar * pText, const _float2 & vPosition, _fvector vColor, _float fRotation, const _float2 & vOrigin)
{
	return m_pFont_Manager->Render_Font(strFontTag, pText, vPosition, vColor, fRotation, vOrigin);
}

HRESULT CGameInstance::Add_RenderTarget(const _wstring & strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4 & vClearColor)
{
	return m_pTarget_Manager->Add_RenderTarget(strTargetTag, iWidth, iHeight, ePixelFormat, vClearColor);	
}

HRESULT CGameInstance::Add_MRT(const _wstring & strMRTTag, const _wstring & strTargetTag)
{
	return m_pTarget_Manager->Add_MRT(strMRTTag, strTargetTag);
}

HRESULT CGameInstance::Begin_MRT(const _wstring & strMRTTag, ID3D11DepthStencilView* pDSV, _bool isClear)
{
	return m_pTarget_Manager->Begin_MRT(strMRTTag, pDSV, isClear);
}

HRESULT CGameInstance::End_MRT()
{
	return m_pTarget_Manager->End_MRT();
}

HRESULT CGameInstance::Bind_RT_ShaderResource(CShader * pShader, const _char * pConstantName, const _wstring & strTargetTag)
{
	return m_pTarget_Manager->Bind_ShaderResource(pShader, pConstantName, strTargetTag);	
}

HRESULT CGameInstance::Copy_RT_Resource(const _wstring & strTargetTag, ID3D11Texture2D * pDest)
{
	return m_pTarget_Manager->Copy_RT_Resource(strTargetTag, pDest);
}

HRESULT CGameInstance::Ready_RT_Debug(const _wstring & strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	return m_pTarget_Manager->Ready_Debug(strTargetTag, fX, fY, fSizeX, fSizeY);
}

HRESULT CGameInstance::Render_RT_Debug(const _wstring & strMRTTag, CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	return m_pTarget_Manager->Render_Debug(strMRTTag, pShader, pVIBuffer);
}

_bool CGameInstance::isPicked(_float4 * pWorldPos)
{
	return m_pPicking->isPicked(pWorldPos);	
}

const _float4x4 * CGameInstance::Get_Height_Transform(CHeight::D3DTRANSFORMSTATE eState)
{
	return m_pHeight->Get_Height_Transform(eState);	
}

void CGameInstance::SetUp_Height_TransformMatrix(CHeight::D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix)
{
	m_pHeight->SetUp_TransformMatrix(eState, TransformMatrix);
}

_float CGameInstance::Compute_Height(_fvector vWorldPos)
{
	return m_pHeight->Compute_Height(vWorldPos);
}

const _float4x4 * CGameInstance::Get_Shadow_Transform(CShadow::D3DTRANSFORMSTATE eState)
{

	return m_pShadow->Get_Shadow_Transform(eState);
}

void CGameInstance::SetUp_Shadow_TransformMatrix(CShadow::D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix)
{
	m_pShadow->SetUp_TransformMatrix(eState, TransformMatrix);
}

void CGameInstance::Transform_Frustum_ToLocalSpace(_fmatrix WorldMatrix)
{
	m_pFrustum->Transform_ToLocalSpace(WorldMatrix);
}

_bool CGameInstance::isIn_Frustum_InWorldSpace(_fvector vWorldPos, _float fRange)
{
	return m_pFrustum->isIn_InWorldSpace(vWorldPos, fRange);
}

_bool CGameInstance::isIn_Frustum_InLocalSpace(_fvector vLocalPos, _float fRange)
{
	return m_pFrustum->isIn_InLocalSpace(vLocalPos, fRange);
}

#ifdef _DEBUG
void CGameInstance::Object_Hierarchy()
{
	if (m_pObject_Manager == nullptr)
		return;

	m_pObject_Manager->Hierarchy();
}

void CGameInstance::Select_GameObject(CGameObject* pObj)
{
	if (m_pBasic_Tool == nullptr)
		return;

	m_pBasic_Tool->Select_GameObject(pObj);
}
void CGameInstance::Reset_Select_GameObject()
{
	if (m_pBasic_Tool == nullptr)
		return;

	m_pBasic_Tool->Reset_Select_GameObject();
}
#endif




void CGameInstance::Release_Engine()
{
	CGameInstance::GetInstance()->Free();

	CGameInstance::GetInstance()->DestroyInstance();
}

void CGameInstance::Free()
{
	__super::Free();

#ifdef _DEBUG
	Safe_Release(m_pImgui_Manager);
	Safe_Release(m_pBasic_Tool);
#endif 

	
	Safe_Release(m_pFrustum);
	Safe_Release(m_pShadow);
	Safe_Release(m_pHeight);
	Safe_Release(m_pPicking);
	Safe_Release(m_pTarget_Manager);
	Safe_Release(m_pFont_Manager);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pInput_Device);
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pRenderer);
	Safe_Release(m_pObject_Manager);
	Safe_Release(m_pPrototype_Manager);
	Safe_Release(m_pLevel_Manager);
	Safe_Release(m_pTimer_Manager);
	Safe_Release(m_pGraphic_Device);
}

#pragma once

#include "Shadow.h"
#include "Height.h"
#include "Renderer.h"
#include "PipeLine.h"
#include "Prototype_Manager.h"
#include "Key_Manager.h"


/* 1. 클라이언트 개발자가 사용할 만한 엔진에서 제공해주는 기능들을 보여준다 .*/
/* 2. 엔진의 기능을 원활하게 사용할 수 있도록 엔진을 초기화한다.Initialize_Engine */
/* 3. 사용 종료시에 엔진 기능 구현을 위해서 할당했던 모든 데이터를 삭제한다. Release_Engine*/
/* 4. 엔진에서 갱신이 필요한 모든 객체를 한곳에 모아서 갱신해 준다. */

BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)
private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public: /* For.GameInstance */
	HRESULT Initialize_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	void Update_Engine(_float fTimeDelta);
	HRESULT Render_Begin(const _float4& vClearColor = _float4(0.f, 0.f, 1.f, 1.f));
	HRESULT Draw();
	HRESULT Render_End();
	void Clear(_int iLevelID);

	_float Compute_Random_Normal();
	_float Compute_Random(_float fMin, _float fMax);

public: /* for.Input_Device */
	_byte	Get_DIKeyState(_ubyte byKeyID);
	_byte	Get_DIMouseState(MOUSEKEYSTATE eMouse);
	_long	Get_DIMouseMove(MOUSEMOVESTATE eMouseState);

public: /* For.timer_Manager */
	_float Get_TimeDelta(const _wstring& strTimerTag);
	HRESULT	Add_Timer(const _wstring& strTimerTag);
	void Update_TimeDelta(const _wstring& strTimerTag);

public: /* For.Renderer */
	HRESULT Add_RenderObject(CRenderer::RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);

#ifdef _DEBUG
public:
	HRESULT Add_DebugComponent(class CComponent* pDebugCom);

#endif
public: /* for.Level_Manager */
	HRESULT Open_Level(_int iLevelIndex, class CLevel* pNewLevel);

public: /* For.Prototype_Manager */
	HRESULT Add_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag, class CBase* pPrototype);
	class CBase* Clone_Prototype(Engine::PROTOTYPE eType, _uint iLevelIndex, const _wstring& strPrototypeTag, void* pArg = nullptr);

public: /* For.Object_Manager */
	HRESULT Add_GameObject_ToLayer(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, void* pArg = nullptr, const _wstring& strObjTag = L"GameObject");
	//class CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponentTag, _uint iObjectIndex = 0);
	//class CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, _uint iPartObjectIndex, const _wstring& strPartComponentTag, _uint iObjectIndex = 0);
	class CGameObject* Find_Object_By_ID(_uint iLevelIndex, const _wstring& strLayerTag, _uint iID);
	class CGameObject* Find_Object_By_Name(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strName);
public: /* For.PipeLine */
	void Set_Transform(CPipeLine::D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix);
	_matrix Get_TransformMatrix(CPipeLine::D3DTRANSFORMSTATE eState);
	_float4x4 Get_TransformFloat4x4(CPipeLine::D3DTRANSFORMSTATE eState);
	_float4x4 Get_TransformFloat4x4_Inverse(CPipeLine::D3DTRANSFORMSTATE eState);
	_matrix Get_TransformMatrix_Inverse(CPipeLine::D3DTRANSFORMSTATE eState);
	const _float4* Get_CamPosition() const;

	string ConvertToString(wstring strName);
	wstring ConvertToWString(string strName);
	
	HRESULT Remove_Object(_uint iCurLevelIdx, const _wstring& strObjTag);
	

public: /* For.Key_Manager */
	const			KEY_STATE& GetKeyState(KEY _eKEY);
	const			KEY_STATE& GetMouseKeyState(MOUSE_KEY _eMouse);
	_long			Get_DIMouseMove(MOUSE_MOVE eMouseMove);
	_bool			Check_Cursor_Position(RECT _tRect);
	POINT			Get_MousePos();


public: /* Light_Manager */
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	const LIGHT_DESC* Get_LightDesc(_uint iIndex) const;
	HRESULT Render_Lights(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

public: /* Font_Manager */
	HRESULT Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath);
	HRESULT Render_Font(const _wstring& strFontTag, const _tchar * pText, const _float2 & vPosition, _fvector vColor, _float fRotation = 0.f, const _float2 & vOrigin = _float2(0.f, 0.f));


public: /* Target_Manager */
	HRESULT Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);
	HRESULT Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV = nullptr, _bool isClear = true);
	HRESULT End_MRT();
	HRESULT Bind_RT_ShaderResource(class CShader* pShader, const _char* pConstantName, const _wstring& strTargetTag);
	HRESULT Copy_RT_Resource(const _wstring& strTargetTag, ID3D11Texture2D* pDest);

#ifdef _DEBUG
public:
	HRESULT Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_RT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif

public: /* Picking */
	_bool isPicked(_Inout_ _float4* pWorldPos);

public: /* Height */
	const _float4x4* Get_Height_Transform(CHeight::D3DTRANSFORMSTATE eState);
	void SetUp_Height_TransformMatrix(CHeight::D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix);
	_float Compute_Height(_fvector vWorldPos);

public: /* Shadow */
	const _float4x4* Get_Shadow_Transform(CShadow::D3DTRANSFORMSTATE eState);
	void SetUp_Shadow_TransformMatrix(CShadow::D3DTRANSFORMSTATE eState, _fmatrix TransformMatrix);

public: /* Frustum */
	void Transform_Frustum_ToLocalSpace(_fmatrix WorldMatrix); /* 특정 오브젝트의 로컬로 변환하기위해 필요에 따라 호출되는 함수. */
	_bool isIn_Frustum_InWorldSpace(_fvector vWorldPos, _float fRange = 0.f);
	_bool isIn_Frustum_InLocalSpace(_fvector vLocalPos, _float fRange = 0.f);

private:
	class CGraphic_Device*				m_pGraphic_Device = { nullptr };
	class CInput_Device*				m_pInput_Device = { nullptr };
	class CTimer_Manager*				m_pTimer_Manager = { nullptr };	
	class CLevel_Manager*				m_pLevel_Manager = { nullptr };
	class CPrototype_Manager*			m_pPrototype_Manager = { nullptr };
	class CObject_Manager*				m_pObject_Manager = { nullptr };
	class CPipeLine*					m_pPipeLine = { nullptr };
	class CRenderer*					m_pRenderer = { nullptr };
	class CLight_Manager*				m_pLight_Manager = { nullptr };
	class CFont_Manager*				m_pFont_Manager = { nullptr };
	class CTarget_Manager*				m_pTarget_Manager = { nullptr };
	class CPicking*						m_pPicking = { nullptr };
	class CHeight*						m_pHeight = { nullptr };
	class CShadow*						m_pShadow = { nullptr };
	class CFrustum*						m_pFrustum = { nullptr };
	class CKey_Manager*					m_pKey_Manager = nullptr;

#ifdef _DEBUG
	class CImgui_Manager*				m_pImgui_Manager = { nullptr };
	class CBasic_Tool*					m_pBasic_Tool = { nullptr };

public:
	void	Object_Hierarchy();
	void	Select_GameObject(class CGameObject* pObj);
	void	Reset_Select_GameObject();
#endif

	

public:
	static void Release_Engine();

public:
	virtual void Free() override;
};

END
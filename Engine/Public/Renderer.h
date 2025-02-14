#pragma once

#include "Base.h"

BEGIN(Engine)

class CRenderer final : public CBase
{
public:
	enum RENDERGROUP { RG_PRIORITY, RG_SHADOW, RG_NONBLEND, RG_NONPICKED, RG_NONLIGHT, RG_BLEND, RG_UI, RG_END };

private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;

public:
	HRESULT Initialize();
	HRESULT Add_RenderObject(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	HRESULT Draw_RenderObject();

#ifdef _DEBUG
public:
	HRESULT Add_DebugComponent(class CComponent* pDebugCom) {
		m_DebugComponents.push_back(pDebugCom);
		Safe_AddRef(pDebugCom);
		return S_OK;
	}

#endif

private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };	
	list<class CGameObject*>			m_RenderObjects[RG_END];
	class CGameInstance*				m_pGameInstance = { nullptr };
	class CShader*						m_pShader = { nullptr };
	class CVIBuffer_Rect*				m_pVIBuffer = { nullptr };
	_float4x4							m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	_uint								m_iOriginalViewportWidth{}, m_iOriginalViewportHeight{};
	ID3D11DepthStencilView*				m_pMaxDepthStencilView = { nullptr };


#ifdef _DEBUG
private:
	list<class CComponent*>				m_DebugComponents;

#endif

private:
	HRESULT Render_Priority();
	//HRESULT Render_Height();
	HRESULT Render_Shadow();

	/* 실제 객체들의 디퓨즈재질과 노멀 벡터를 각 렌더타겟에 렌더링했다. 빛 연산을 위한 재료 준비. */	
	HRESULT Render_NonBlend();

	/* 빛연산을 수행한다. 결과를 Target_Shade에다가 렌더링 해놓는다. */
	HRESULT Render_Lights();
	HRESULT Render_Final();

	HRESULT Render_Blur_X();
	HRESULT Render_Blur_Y();
	
	

	HRESULT Render_NonLight();
	HRESULT Render_Blend();
	HRESULT Render_UI();

	HRESULT Ready_DepthStencilView(_uint iWidth, _uint iHeight);
	HRESULT SetUp_Viewport(_uint iWidth, _uint iHeight);

#ifdef _DEBUG
private:
	HRESULT Render_Debug();
#endif

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END
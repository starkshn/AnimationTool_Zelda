#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include <deque>
#include "Model.h"
using namespace std;

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class PartAnimObj_Type
{
public:
	// 기본 재생하는 경우, 보간중인 경우
	enum class ANIM_PLAY_TYPE { DEFAULT, MULTI_DEFAULT, MULTI_INTERPOLATION };
};

class CAT_PartAnimObj final :public CGameObject
{
public:
	typedef struct anim_tool_object_boyd_desc : public GAMEOBJECT_DESC
	{
		_wstring strModelProtoName;
	} AT_PARTANIMOBJ_DESC;

private:
	CAT_PartAnimObj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAT_PartAnimObj(const CAT_PartAnimObj& Prototype);
	virtual ~CAT_PartAnimObj() = default;

public:
	const _float4x4* Get_BoneMatrix(const _char* pBoneName) const;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;

private:
	void Update_AnimLoop_Default(_float fTimeDelta);

	// 애니매이션 플레이 리스트에 있는 애니매이션 목록들을 보간 없이 순차적으로 재생하는 함수.
	void Update_AnimLoop_NotInterpolation(_float fTimeDelta);

	void Update_AnimLoop_Interpolation(_float fTimeDelta);
	void Update_AnimLoop_Interpolation_End(_float fTimeDelta);

public:
	_int Get_CurAnimIdx();
	void Set_CurAnimIdx(_int iCurAnimIdx, _bool bLoop = false);
	PartAnimObj_Type::ANIM_PLAY_TYPE Get_AnimPlayType() { return m_eAnimPlayType; }
	void Set_AnimPlayType(PartAnimObj_Type::ANIM_PLAY_TYPE eType) { m_eAnimPlayType = eType; }
	_float Get_InterDis() { return m_fInterDis;}
	void Set_InterDis(_float fInterDis) { m_fInterDis = fInterDis; }
	_float Get_InterSP() { return m_fInterSP; }
	void Set_InterSP(_float fInterSP) { m_fInterSP = fInterSP; }
	_bool Get_Interpolationing()
	{
		if (m_pModelCom)
			return m_pModelCom->Get_Interpolationing();
		return false;
	}

public:
	void Add_AnimIn_AnimPlayList(_int iAnimIdx, _bool bLoop = true) { m_AnimPlayLists.push_back({ iAnimIdx, bLoop }); }
	void Clear_AnimPlayLists() 
	{ 
		if (!m_AnimPlayLists.empty())
			m_AnimPlayLists.clear();
	}

	_fmatrix Get_RootBoneCombinedTransfomationMatrix() { return m_MatRootBoneCombindedTransformationMatrix; }
	_float Get_Anim_TickPerSecond();
	_float Get_Anim_Duration();
	_float Get_Anim_CurTrackPos();

public:
	void StopAndPlay_CurAnim();


private:
	void TestFunc();
	
	// 로그 관련
private:
	void Log_AnimEnd();
	void Log_AnimEnd(string strLog);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

private:
	_wstring m_strModelProtoName;
	_int m_iCurAnimIdx = 0;
	_bool m_bCurAnimLoop = false;

	// 재생할 애니매이션 목록을 관리하는 컨테이너
private:
	deque<pair<_int, _bool>> m_AnimPlayLists;
	PartAnimObj_Type::ANIM_PLAY_TYPE m_eAnimPlayType = PartAnimObj_Type::ANIM_PLAY_TYPE::DEFAULT;
	
	_float m_fInterDis = 0.2f;
	_float m_fInterSP = 1.f;

	_matrix m_MatRootBoneCombindedTransformationMatrix{};

public:
	_bool m_bFollowRootBone = false;

public:
	_bool m_bIsPlayAnimBlend = false;

public:
	_bool m_bStartInterpolation = false;



private:
	vector<string> m_Logs;

public:
	vector<string>& Get_Logs() { return m_Logs; }
	void Clear_Logs() { m_Logs.clear(); }

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CAT_PartAnimObj* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END
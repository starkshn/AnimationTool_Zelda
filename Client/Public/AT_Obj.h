#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include "AT_PartAnimObj.h"

BEGIN(Client)

class CAT_Obj final : public CGameObject
{
public:
	typedef struct anim_tool_object_desc : public GAMEOBJECT_DESC
	{
		_wstring strModelProtoName;
	} AT_OBJ_DESC;
private:
	CAT_Obj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAT_Obj(const CAT_Obj& Prototype);
	virtual ~CAT_Obj() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void	Add_PartAnimObject(CAT_PartAnimObj* pPartAnimObject);

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();

private:
	_wstring m_strModelProtoName;

public:
	_bool m_bFollowRootBone = false;
	_matrix m_MatPrevRootBoneCombinedTransformationMatrix;

public:
	static CAT_Obj* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
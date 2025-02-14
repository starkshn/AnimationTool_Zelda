#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CTexture final : public CComponent
{
private:
	CTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTexture(const CTexture& Prototype);
	virtual ~CTexture() = default;

public:
	virtual HRESULT Initialize_Prototype(const _tchar * pTextureFilePath, _uint iNumTexture);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, _uint iSRVIndex = 0);
	HRESULT Bind_ShaderResources(class CShader* pShader, const _char* pConstantName);

private:
	_uint										m_iNumSRVs = { 0 };
	/* 화면에 그려내기 위한 텍스쳐 타입. */
	vector<ID3D11ShaderResourceView*>			m_SRVs;

public:
	static CTexture* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pTextureFilePath, _uint iNumTextures = 1);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END
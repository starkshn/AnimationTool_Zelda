#pragma once
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Line final : public CVIBuffer
{
private:
	CVIBuffer_Line(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Line(const CVIBuffer_Line& Prototype);
	virtual ~CVIBuffer_Line() override = default;

public:
	virtual HRESULT						Initialize_Prototype() override;
	virtual HRESULT						Initialize(void* pArg) override;

public:
	static			CVIBuffer_Line*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual			CComponent*			Clone(void* pArg) override;
	virtual			void				Free() override;
};

END
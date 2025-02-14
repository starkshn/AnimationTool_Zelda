#pragma once
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Square final : public CVIBuffer
{
private:
	CVIBuffer_Square(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Square(const CVIBuffer_Square& Prototype);
	virtual ~CVIBuffer_Square() override = default;

public:
	virtual HRESULT						Initialize_Prototype() override;
	virtual HRESULT						Initialize(void* pArg) override;

public:
	static			CVIBuffer_Square* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual			CComponent* Clone(void* pArg) override;
	virtual			void				Free() override;
};

END
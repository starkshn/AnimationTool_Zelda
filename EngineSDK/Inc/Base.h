#pragma once

#include "Engine_Defines.h"

/* 1. 레퍼런스 카운트를 관리한다. */
/* 1-1. 레퍼런스 카운트를 증가한다. AddRef */
/* 1-2. 레퍼런스 카운트를 감소한다. or 삭제한다. Release */

BEGIN(Engine)

class ENGINE_DLL CBase abstract
{
protected:
	CBase();
	virtual ~CBase() = default;

public:
	_uint AddRef();
	_uint Release();

private:
	_uint			m_iRefCnt = { 0 };

public:
	virtual void Free();

};

END
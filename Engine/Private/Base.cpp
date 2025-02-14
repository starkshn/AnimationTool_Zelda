#include "..\Public\Base.h"

Engine::CBase::CBase()
{
	
}

/* _uint  : 증가한 결과물을 리턴한다. */
_uint CBase::AddRef()
{
	return ++m_iRefCnt;	
}

/* _uint  : 감소하기 전의 결과물을 리턴한다. */
_uint CBase::Release()
{
	/* 삭제한다. */	
	if (0 == m_iRefCnt)
	{
		Free();

		delete this;

		return 0;
	}
	/* 감소한다. */
	else
		return m_iRefCnt--;	
}

void CBase::Free()
{
}


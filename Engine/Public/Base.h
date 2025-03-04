#pragma once

#include "Engine_Defines.h"

BEGIN(Engine)

class ENGINE_DLL CBase abstract
{
protected:
	CBase();
	virtual ~CBase() = default;

public:
	/* ���۷��� ī��Ʈ�� ������Ų��. */
	/* ���� : �����ϰ� �� ������ RefCnt�� ���� */
	unsigned int AddRef();

	/* ���۷��� ī��Ʈ�� ���ҽ�Ų��. or �����Ѵ�. */
	/* ���� : ���ҽ�Ű�� ������ RefCnt�� ���� */
	unsigned int Release();
private:

	unsigned int		m_iRefCnt = { 0 };

public:
	virtual void Free();
};

END
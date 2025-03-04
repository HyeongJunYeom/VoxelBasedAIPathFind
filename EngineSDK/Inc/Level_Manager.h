#pragma once

#include "Base.h"

/* 1. ���� ���ӳ��� ��������� ������ �����ϰ� �ִ�. */
/* 2. �����ϰ� �ִ� ������ Tick, Render�� ������ �ݺ��Ͽ� ȣ���Ѵ�. */
/* 3. ���ο� ������ �Ҵ��� ��, ���������� �����ϴ� ����� �����Ѵ�. */
/* 4. ���� ������ �ı��� ��, ���� ���������� �ε��س��� �ڿ����� �����Ѵ�. */

BEGIN(Engine)

class CLevel_Manager final : public CBase
{
private:
	CLevel_Manager();
	virtual ~CLevel_Manager() = default;

public:
	HRESULT Initialize();
	HRESULT Open_Level(_uint iNewLevelID, class CLevel* pNewLevel);
	void Tick(_float fTimeDelta);
	HRESULT Render();


private:
	_uint					m_iCurrentLevelID = { 0 };
	class CLevel*			m_pCurrentLevel = { nullptr };

public:
	static CLevel_Manager* Create();
	virtual void Free() override;
};

END
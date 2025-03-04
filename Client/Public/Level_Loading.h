#pragma once

#include "Client_Defines.h"
#include "Level.h"

/* 1. ������ �ʿ��� ��ü���� �����Ѵ�. */
/* 2. ������ �ݺ������� �����Ͽ� ȭ�鿡 �����ش�. */

/* 3. �������������� �ڿ��� �ε��ϱ� ���� �δ���ü�� �������ش�.  */

BEGIN(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(LEVEL eNextLevelID);
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CLoader*				m_pLoader = { nullptr };
	LEVEL						m_eNextLevelID = { LEVEL_END };

private:
	/* �ε������� �ʿ��� ��ü�� ����� �Լ�. */
	HRESULT Ready_Layer_BackGround();
	HRESULT Ready_Layer_UI();


public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

END
#pragma once

#include "Base.h"

BEGIN(Engine)

class CPath_Optimizer final : public CBase
{
private:
	CPath_Optimizer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPath_Optimizer(const CPath_Optimizer& rhs) = delete;
	virtual ~CPath_Optimizer() = default;

public:
	HRESULT Initialize(void* pArg);
	HRESULT Render();

public:
	TEST_MODE Get_TestMode() { return m_eTestMode; }
	void Set_TestMode(TEST_MODE eTestMode) {
		if (TEST_MODE::_END <= eTestMode)
			return;
		m_eTestMode = eTestMode;
	}

public:
	void Compute_Optimized_Path(const list<_uint>& BestList);
	const list<_uint>& Get_OptimizedBestPath();

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

private:
	TEST_MODE					m_eTestMode = { TEST_MODE::_DEFAULT };

private:
	list<_uint>					m_BestPathIndices_Optimized;

public:
	static CPath_Optimizer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual void Free() override;
};

END
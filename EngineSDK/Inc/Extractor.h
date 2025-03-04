#pragma once

#include "Base.h"

/* ���� Ư�� Viewport���� ��ġ xy�� �����ָ� �ش���ġ�� �ִ� �ȼ��� ������ġ�� ������ش�. */
/* z���� ���������ؼ��� ���� ����Ÿ������ ���� ���´�. */


BEGIN(Engine)

class CExtractor final : public CBase
{
private:
	CExtractor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CExtractor() = default;

public:
	HRESULT Initialize();

public:
	_vector Compute_WorldPos(const _float2& vViewportPos, const wstring& strZRenderTargetTag, _uint iOffset);

private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };
	class CGameInstance*				m_pGameInstance = { nullptr };

	_uint								m_iViewportWidth = { 0 };
	_uint								m_iViewportHeight = { 0 };

	/* ����Ÿ�ٿ����� ������� �ؽ��ķκ��� ���� ���� ���� �� ���⶧���� .*/
	/*���� ������ ��������� ����Ÿ���� ������ ����޾Ƴ������� �ؽ����̴�. */
	ID3D11Texture2D*			m_pTextureHub = { nullptr };

public:
	static CExtractor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END
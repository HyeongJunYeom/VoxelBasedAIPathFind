#pragma once

#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Line final : public CVIBuffer
{
private:
	CVIBuffer_Line(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CVIBuffer_Line(const CVIBuffer_Line& rhs);
	virtual ~CVIBuffer_Line() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

public:
	_float4x4 Compute_Look_At(_fvector vLookPos, _fvector vAtPos);

private:
	_float4x4			m_WorldMatrix = {};

public:
	static CVIBuffer_Line* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END
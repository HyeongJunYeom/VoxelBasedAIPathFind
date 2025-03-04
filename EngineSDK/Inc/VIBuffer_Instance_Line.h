#pragma once
#include "VIBuffer_Instance.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance_Line final : public CVIBuffer
{

private:
	CVIBuffer_Instance_Line(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CVIBuffer_Instance_Line(const CVIBuffer_Instance_Line& rhs);
	virtual ~CVIBuffer_Instance_Line() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Bind_Buffers();
	virtual HRESULT Render();

public:
	void Set_Positions(const vector<pair<_float3, _float3>>& Poses);

	_uint Get_MaxInstance() { return m_iNumMaxInstance; };

protected:
	ID3D11Buffer*					m_pVBInstance = { nullptr };
	_uint							m_iInstanceStride = { 0 };
	_uint							m_iNumMaxInstance = { 0 };
	_uint							m_iNumCurInstance = { 0 };
	_uint							m_iIndexCountPerInstance = { 0 };
	VTXINSTANCE_LINE*				m_pInstanceVertices = { nullptr };

protected:
	D3D11_BUFFER_DESC				m_InstanceBufferDesc{};
	D3D11_SUBRESOURCE_DATA			m_InstanceSubResourceData{};

public:
	static CVIBuffer_Instance_Line* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

END

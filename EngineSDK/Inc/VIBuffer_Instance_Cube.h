#pragma once
#include "VIBuffer_Instance.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance_Cube final : public CVIBuffer
{

private:
	CVIBuffer_Instance_Cube(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CVIBuffer_Instance_Cube(const CVIBuffer_Instance_Cube& rhs);
	virtual ~CVIBuffer_Instance_Cube() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Bind_Buffers();
	virtual HRESULT Render();

public:
	void Set_Positions(const vector<_float3>&Poses, _float fScale);
	void Set_Positions_States_IDs(const vector<_float3>&Poses, const vector<_byte>& States, const vector<_byte>& IDs, const vector<_int>&NeighborFlags, _float fScale);

	_uint Get_MaxInstance() { return m_iNumMaxInstance; };

protected:
	ID3D11Buffer*					m_pVBInstance = { nullptr };
	_uint							m_iInstanceStride = { 0 };
	_uint							m_iNumMaxInstance = { 0 };
	_uint							m_iNumCurInstance = { 0 };
	_uint							m_iIndexCountPerInstance = { 0 };
	VTXINSTANCE_CUBE*				m_pInstanceVertices = { nullptr };

protected:
	D3D11_BUFFER_DESC				m_InstanceBufferDesc{};
	D3D11_SUBRESOURCE_DATA			m_InstanceSubResourceData{};

public:
	static CVIBuffer_Instance_Cube* Create(ID3D11Device * pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

END

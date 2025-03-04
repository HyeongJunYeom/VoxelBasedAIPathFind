#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CCollider final : public CComponent
{
public:
	enum MODEL_TYPE { TYPE_AABB, TYPE_OBB, TYPE_SPHERE, TYPE_END };
private:
	CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCollider(const CCollider& rhs);
	virtual ~CCollider() = default;

public:
	virtual HRESULT Initialize_Prototype(MODEL_TYPE eType);
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_fmatrix WorldMatrix);
	_bool Intersect(CCollider* pTargetCollider);

#ifdef _DEBUG
public:
	virtual HRESULT Render() override;

#endif

private:
	MODEL_TYPE						m_eType = { TYPE_END };
	class CBounding*			m_pBounding = { nullptr };

#ifdef _DEBUG
private:
	PrimitiveBatch<VertexPositionColor>*		m_pBatch = { nullptr };
	BasicEffect*								m_pEffect = { nullptr };
	ID3D11InputLayout*							m_pInputLayout = { nullptr };
#endif
	
	

public:
	static CCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL_TYPE eType);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
	
};

END
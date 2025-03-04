#pragma once

#include "GameObject.h"

BEGIN(Engine)

class CVoxelRenderCube final : public CGameObject
{
private:
	CVoxelRenderCube(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVoxelRenderCube(const CVoxelRenderCube& rhs);
	virtual ~CVoxelRenderCube() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Render() override;

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

private:
	class CShader*				m_pShaderCom = { nullptr };
	class CVIBuffer_Cube*		m_pParentVIBufferCube = { nullptr };

public:
	static CVoxelRenderCube* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END
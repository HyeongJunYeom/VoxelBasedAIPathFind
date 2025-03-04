#pragma once
#include "GameObject.h"

BEGIN(Engine)

class CDummy_Object final : public CGameObject
{
public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{

	}GAMEOBJECT_DESC;

protected:
	CDummy_Object(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDummy_Object(const CDummy_Object& rhs) = delete;
	virtual ~CDummy_Object() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Tick(_float fTimeDelta) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	_uint Get_NumAnims();
	void Set_Animation(const _uint iAnimIndex, const _bool isLoop);

public:
	HRESULT Add_Component(CComponent* pComponent, const wstring& strComponentTag);

private:
	HRESULT Bind_ShaderResources(CShader* pShader);
	HRESULT Bind_ShaderResources_Model();

private:
	_uint						m_iShaderPass = {};

public:
	static CDummy_Object* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual CDummy_Object* Clone(void* pArg);
	virtual void Free() override;
};

END
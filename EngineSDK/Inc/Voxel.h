#pragma once

#include "GameObject.h"

BEGIN(Engine)

class CVoxel : public CBase
{
	//private:	//	Pimple Idiom
	//	class CImpl_Voxel;
	//	typedef CImpl_Voxel Impl;

	//public:
	//	typedef struct : public CTransform::TRANSFORM_DESC
	//	{
	//	}GAMEOBJECT_DESC;

private:
	CVoxel();
	CVoxel(const CVoxel& rhs);
	virtual ~CVoxel() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void Late_Tick(_float fTimeDelta);
	virtual HRESULT Render();

public:
	HRESULT Set_ID(_int iID);
	HRESULT Set_Pos(const _float3& vPos);

public:
	_int Get_ID();
	_float3 Get_Pos();

private:
	_int					m_iID = { -1 };
	_float3					m_vPos = {};

public:
	static CVoxel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CVoxel* Clone(void* pArg);
	virtual void Free() override;
};

//	ÁöÇü , 

END
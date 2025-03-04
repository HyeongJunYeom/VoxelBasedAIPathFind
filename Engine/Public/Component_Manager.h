#pragma once

#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "Collider.h"
#include "Transform.h"
#include "Navigation.h"
#include "VIBuffer_Rect.h"
#include "VIBuffer_Cube.h"
#include "VIBuffer_Terrain.h"
#include "VIBuffer_Instance_Rect.h"
#include "VIBuffer_Instance_Point.h"
#include "VIBuffer_Instance_Cube.h"

#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_SPHERE.h"

#include "Obstacle_Cube.h"

#include "PathFinder_Voxel.h"

#include "Component.h"

/* 1. ������Ʈ���� ������ �������� �����Ѵ�. */
/* 2. ������ ������ �����Ͽ� �纻��ü�� �����ϰ� �����Ѵ� .*/

BEGIN(Engine)

class CComponent_Manager final : public CBase
{
private:
	CComponent_Manager();
	virtual ~CComponent_Manager() = default;

public:
	HRESULT Initialize(_uint iNumLevels);
	HRESULT Add_Prototype(_uint iLevelIndex, const wstring& strPrototypeTag, class CComponent* pPrototype);
	class CComponent* Clone_Component(_uint iLevelIndex, const wstring& strPrototypeTag, void* pArg);
	void Clear(_uint iLevelIndex);

private:
	map<const wstring, class CComponent*>*			m_pPrototypes = { nullptr };
	typedef map<const wstring, class CComponent*>	PROTOTYPES;
	_uint											m_iNumLevels = { 0 };

private:
	class CComponent* Find_Prototype(_uint iLevelIndex, const wstring& strPrototypeTag);

public:
	static CComponent_Manager* Create(_uint iNumLevels);
	virtual void Free() override;
};

END
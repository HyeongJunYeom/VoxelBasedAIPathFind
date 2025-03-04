#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Link : public CBase
{
public:
	typedef struct tagVoxelLinkDesc
	{
	}VOXEL_LINK_DESC;

private:
	CVoxel_Link(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVoxel_Link(const CVoxel_Link& rhs) = delete;
	virtual ~CVoxel_Link() = default;

public:
	HRESULT Initialize(void* pArg);
	HRESULT Render();

public:
	HRESULT Add_Link(const _uint iSrcIndex, const _uint iDstIndex);
	HRESULT Erase_Link(const _uint iSrcIndex, const _uint iDstIndex);

private:
	unordered_map<_uint, unordered_set<_uint>>			m_VoxelLinks;

	class CGameInstance*								m_pGameInstance = { nullptr };
	ID3D11Device*										m_pDevice = { nullptr };
	ID3D11DeviceContext*								m_pContext = { nullptr };

public:
	static CVoxel_Link* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END
#pragma once

#include "Base.h"
#include "ImGui_Includes.h"

BEGIN(Engine)

class CImGUI_Manager :
    public CBase
{
public:
	typedef struct tagImGuiManagerDesc
	{
		class CVoxel_Manager* pVoxel_Manager = { nullptr };
		class CPath_Manager* pPath_Manager = { nullptr };
	} IMGUI_MANAGER_DESC;

private:
	enum EDITOR_TYPE { EDITOR_UI, EDITOR_MAP, EDIOTR_COLLIDER, EDITOR_WAYPOINT, EDITOR_PROP, EDITOR_MONSTER, EDITOR_END };
	enum DEBUGER_TYPE { DEBUGER_OBJECT, DEBUGER_END };

private:
	CImGUI_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	~CImGUI_Manager() = default;

public:
	HRESULT Initialize(HWND hWnd, IMGUI_MANAGER_DESC* pDesc);
	void Tick(_float fTimeDelta);
	HRESULT Render();

	void OnOffImgui() { m_isEnable = !m_isEnable; }
	_bool IsEnable() { return m_isEnable; }

private:
	HRESULT Ready_Tools(const IMGUI_MANAGER_DESC& Desc);

public:
	HRESULT Insert_GuiObject(const wstring& strTag, class CGui_Object* pObject);
	HRESULT Erase_GuiObject(const wstring& strTag);
	HRESULT Clear_GuiObjects();

public:
	IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	_bool						m_isEnable = { false };

	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

	map<wstring, class CGui_Object*>		m_Objects;

	_bool						m_isHovered = { false };
	_bool						m_isWindowFocused = { false };


private:
	_int								m_iCurObjectIndex = { -1 };
	vector<class CDummy_Object*>		m_DummyObjects;
	vector<vector<class CComponent*>>	m_Components;

public:
	static CImGUI_Manager* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, HWND hWnd, IMGUI_MANAGER_DESC* pDesc);
	virtual void Free() override;
};

END
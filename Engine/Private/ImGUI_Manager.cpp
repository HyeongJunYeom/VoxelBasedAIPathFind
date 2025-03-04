#include "ImGUI_Manager.h"
#include "GameInstance.h"
#include "Gui_Object.h"
#include "Voxel_Tool.h"
#include "Navi_Tool.h"
#include "Object_Tool.h"
#include "Obstacle_Tool.h"

CImGUI_Manager::CImGUI_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CImGUI_Manager::Initialize(HWND hWnd, IMGUI_MANAGER_DESC* pDesc)
{
	if (nullptr == pDesc)
		return E_FAIL;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	/* ImGui √ ±‚»≠ */
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	m_Components.resize(static_cast<_uint>(COMPONENT_TYPE::_END));

	if (FAILED(Ready_Tools(*pDesc)))
		return E_FAIL;

	return S_OK;
}

void CImGUI_Manager::Tick(_float fTimeDelta)
{
	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing Dear ImGui context. Refer to examples app!");

	// Examples Apps (accessible from the "Examples" menu)
	static bool show_app_main_menu_bar = false;
	static bool show_app_console = false;
	static bool show_app_custom_rendering = false;
	static bool show_app_documents = false;
	static bool show_app_log = false;
	static bool show_app_layout = false;
	static bool show_app_property_editor = false;
	static bool show_app_simple_overlay = false;
	static bool show_app_auto_resize = false;
	static bool show_app_constrained_resize = false;
	static bool show_app_fullscreen = false;
	static bool show_app_long_text = false;
	static bool show_app_window_titles = false;

	// Dear ImGui Tools (accessible from the "Tools" menu)
	static bool show_tool_metrics = false;
	static bool show_tool_debug_log = false;
	static bool show_tool_id_stack_tool = false;
	static bool show_tool_style_editor = false;
	static bool show_tool_about = false;

	if (show_tool_metrics)
		ImGui::ShowMetricsWindow(&show_tool_metrics);
	if (show_tool_debug_log)
		ImGui::ShowDebugLogWindow(&show_tool_debug_log);
	if (show_tool_id_stack_tool)
		ImGui::ShowIDStackToolWindow(&show_tool_id_stack_tool);
	if (show_tool_style_editor)
	{
		ImGui::Begin("Dear ImGui Style Editor", &show_tool_style_editor);
		ImGui::ShowStyleEditor();
		ImGui::End();
	}
	if (show_tool_about)
		ImGui::ShowAboutWindow(&show_tool_about);

	// Demonstrate the various window flags. Typically you would just use the default!
	static bool no_titlebar = false;
	static bool no_scrollbar = false;
	static bool no_menu = false;
	static bool no_move = false;
	static bool no_resize = false;
	static bool no_collapse = false;
	static bool no_close = false;
	static bool no_nav = false;
	static bool no_background = false;
	static bool no_bring_to_front = false;
	static bool unsaved_document = false;

	ImGuiWindowFlags window_flags = 0;
	if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
	if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
	if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
	if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
	if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
	if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
	if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
	if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_isHovered = false;
	m_isWindowFocused = false;

	/* IMGUI STYLE */
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 12.f;
	style.GrabRounding = 12.f;
	style.FrameBorderSize = 1.f;
	style.Colors[ImGuiCol_TabHovered] = ImVec4(1.f, 0.f, 0.f, 0.5f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(1.f, 0.f, 0.f, 1.0f);
	/*========================================================================*/

#pragma region BASE_CODE
//	ImGui::Begin(u8"Editor_Select");
//
//	// Menu Bar
//	if (ImGui::BeginMenuBar())
//	{
//		if (ImGui::BeginMenu("Menu"))
//		{
//			ImGui::EndMenu();
//		}
//		if (ImGui::BeginMenu("Examples"))
//		{
//			ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
//
//			ImGui::SeparatorText("Mini apps");
//			ImGui::MenuItem("Console", NULL, &show_app_console);
//			ImGui::MenuItem("Custom rendering", NULL, &show_app_custom_rendering);
//			ImGui::MenuItem("Documents", NULL, &show_app_documents);
//			ImGui::MenuItem("Log", NULL, &show_app_log);
//			ImGui::MenuItem("Property editor", NULL, &show_app_property_editor);
//			ImGui::MenuItem("Simple layout", NULL, &show_app_layout);
//			ImGui::MenuItem("Simple overlay", NULL, &show_app_simple_overlay);
//
//			ImGui::SeparatorText("Concepts");
//			ImGui::MenuItem("Auto-resizing window", NULL, &show_app_auto_resize);
//			ImGui::MenuItem("Constrained-resizing window", NULL, &show_app_constrained_resize);
//			ImGui::MenuItem("Fullscreen window", NULL, &show_app_fullscreen);
//			ImGui::MenuItem("Long text display", NULL, &show_app_long_text);
//			ImGui::MenuItem("Manipulating window titles", NULL, &show_app_window_titles);
//
//			ImGui::EndMenu();
//		}
//		//if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
//		if (ImGui::BeginMenu("Tools"))
//		{
//#ifndef IMGUI_DISABLE_DEBUG_TOOLS
//			const bool has_debug_tools = true;
//#else
//			const bool has_debug_tools = false;
//#endif
//			ImGui::MenuItem("Metrics/Debugger", NULL, &show_tool_metrics, has_debug_tools);
//			ImGui::MenuItem("Debug Log", NULL, &show_tool_debug_log, has_debug_tools);
//			ImGui::MenuItem("ID Stack Tool", NULL, &show_tool_id_stack_tool, has_debug_tools);
//			ImGui::MenuItem("Style Editor", NULL, &show_tool_style_editor);
//			bool is_debugger_present = ImGui::GetIO().ConfigDebugIsDebuggerPresent;
//			if (ImGui::MenuItem("Item Picker", NULL, false, has_debug_tools && is_debugger_present))
//				ImGui::DebugStartItemPicker();
//			if (!is_debugger_present)
//				ImGui::SetItemTooltip("Requires io.ConfigDebugIsDebuggerPresent=true to be set.\n\nWe otherwise disable the menu option to avoid casual users crashing the application.\n\nYou can however always access the Item Picker in Metrics->Tools.");
//			ImGui::Separator();
//			ImGui::MenuItem("About Dear ImGui", NULL, &show_tool_about);
//			ImGui::EndMenu();
//		}
//		ImGui::EndMenuBar();
//	}
//
//	ImGui::Text("dear imgui says hello! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
//	ImGui::Spacing();
//	if (ImGui::CollapsingHeader("Help"))
//	{
//		ImGui::SeparatorText("ABOUT THIS DEMO:");
//		ImGui::BulletText("Sections below are demonstrating many aspects of the library.");
//		ImGui::BulletText("The \"Examples\" menu above leads to more demo contents.");
//		ImGui::BulletText("The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
//			"and Metrics/Debugger (general purpose Dear ImGui debugging tool).");
//
//		ImGui::SeparatorText("PROGRAMMER GUIDE:");
//		ImGui::BulletText("See the ShowDemoWindow() code in imgui_demo.cpp. <- you are here!");
//		ImGui::BulletText("See comments in imgui.cpp.");
//		ImGui::BulletText("See example applications in the examples/ folder.");
//		ImGui::BulletText("Read the FAQ at https://www.dearimgui.com/faq/");
//		ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
//		ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");
//
//		ImGui::SeparatorText("USER GUIDE:");
//		ImGui::ShowUserGuide();
//	}
//
//	if (ImGui::CollapsingHeader("Configuration"))
//	{
//		ImGuiIO& io = ImGui::GetIO();
//
//		if (ImGui::TreeNode("Configuration##2"))
//		{
//			ImGui::SeparatorText("General");
//			ImGui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard", &io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
//			ImGui::CheckboxFlags("io.ConfigFlags: NavEnableGamepad", &io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad);
//			ImGui::CheckboxFlags("io.ConfigFlags: NavEnableSetMousePos", &io.ConfigFlags, ImGuiConfigFlags_NavEnableSetMousePos);
//			ImGui::CheckboxFlags("io.ConfigFlags: NoMouse", &io.ConfigFlags, ImGuiConfigFlags_NoMouse);
//			if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)
//			{
//				// The "NoMouse" option can get us stuck with a disabled mouse! Let's provide an alternative way to fix it:
//				if (fmodf((float)ImGui::GetTime(), 0.40f) < 0.20f)
//				{
//					ImGui::SameLine();
//					ImGui::Text("<<PRESS SPACE TO DISABLE>>");
//				}
//				if (ImGui::IsKeyPressed(ImGuiKey_Space))
//					io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
//			}
//			ImGui::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange", &io.ConfigFlags, ImGuiConfigFlags_NoMouseCursorChange);
//			ImGui::Checkbox("io.ConfigInputTrickleEventQueue", &io.ConfigInputTrickleEventQueue);
//			ImGui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
//
//			ImGui::SeparatorText("Widgets");
//			ImGui::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
//			ImGui::Checkbox("io.ConfigInputTextEnterKeepActive", &io.ConfigInputTextEnterKeepActive);
//			ImGui::Checkbox("io.ConfigDragClickToInputText", &io.ConfigDragClickToInputText);
//			ImGui::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
//			ImGui::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
//			ImGui::Checkbox("io.ConfigMacOSXBehaviors", &io.ConfigMacOSXBehaviors);
//			ImGui::Text("Also see Style->Rendering for rendering options.");
//
//			ImGui::SeparatorText("Debug");
//			ImGui::Checkbox("io.ConfigDebugIsDebuggerPresent", &io.ConfigDebugIsDebuggerPresent);
//			ImGui::BeginDisabled();
//			ImGui::Checkbox("io.ConfigDebugBeginReturnValueOnce", &io.ConfigDebugBeginReturnValueOnce); // .
//			ImGui::EndDisabled();
//			ImGui::Checkbox("io.ConfigDebugBeginReturnValueLoop", &io.ConfigDebugBeginReturnValueLoop);
//			ImGui::Checkbox("io.ConfigDebugIgnoreFocusLoss", &io.ConfigDebugIgnoreFocusLoss);
//			ImGui::Checkbox("io.ConfigDebugIniSettings", &io.ConfigDebugIniSettings);
//
//			ImGui::TreePop();
//			ImGui::Spacing();
//		}
//
//		if (ImGui::TreeNode("Backend Flags"))
//		{
//			// FIXME: Maybe we need a BeginReadonly() equivalent to keep label bright?
//			ImGui::BeginDisabled();
//			ImGui::CheckboxFlags("io.BackendFlags: HasGamepad", &io.BackendFlags, ImGuiBackendFlags_HasGamepad);
//			ImGui::CheckboxFlags("io.BackendFlags: HasMouseCursors", &io.BackendFlags, ImGuiBackendFlags_HasMouseCursors);
//			ImGui::CheckboxFlags("io.BackendFlags: HasSetMousePos", &io.BackendFlags, ImGuiBackendFlags_HasSetMousePos);
//			ImGui::CheckboxFlags("io.BackendFlags: RendererHasVtxOffset", &io.BackendFlags, ImGuiBackendFlags_RendererHasVtxOffset);
//			ImGui::EndDisabled();
//			ImGui::TreePop();
//			ImGui::Spacing();
//		}
//
//		if (ImGui::TreeNode("Style"))
//		{
//			ImGui::ShowStyleEditor();
//			ImGui::TreePop();
//			ImGui::Spacing();
//		}
//
//		if (ImGui::TreeNode("Capture/Logging"))
//		{
//			ImGui::LogButtons();
//
//			if (ImGui::Button("Copy \"Hello, world!\" to clipboard"))
//			{
//				ImGui::LogToClipboard();
//				ImGui::LogText("Hello, world!");
//				ImGui::LogFinish();
//			}
//			ImGui::TreePop();
//		}
//	}
//
//	if (ImGui::CollapsingHeader("Window options"))
//	{
//		if (ImGui::BeginTable("split", 3))
//		{
//			ImGui::TableNextColumn(); ImGui::Checkbox("No titlebar", &no_titlebar);
//			ImGui::TableNextColumn(); ImGui::Checkbox("No scrollbar", &no_scrollbar);
//			ImGui::TableNextColumn(); ImGui::Checkbox("No menu", &no_menu);
//			ImGui::TableNextColumn(); ImGui::Checkbox("No move", &no_move);
//			ImGui::TableNextColumn(); ImGui::Checkbox("No resize", &no_resize);
//			ImGui::TableNextColumn(); ImGui::Checkbox("No collapse", &no_collapse);
//			ImGui::TableNextColumn(); ImGui::Checkbox("No close", &no_close);
//			ImGui::TableNextColumn(); ImGui::Checkbox("No nav", &no_nav);
//			ImGui::TableNextColumn(); ImGui::Checkbox("No background", &no_background);
//			ImGui::TableNextColumn(); ImGui::Checkbox("No bring to front", &no_bring_to_front);
//			ImGui::TableNextColumn(); ImGui::Checkbox("Unsaved document", &unsaved_document);
//			ImGui::EndTable();
//		}
//	}
//
//	ImGui::End();

#pragma endregion

	for (auto& Pair : m_Objects)
	{
		Pair.second->Tick(fTimeDelta);
	}
}

HRESULT CImGUI_Manager::Render()
{
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return S_OK;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HRESULT CImGUI_Manager::Ready_Tools(const IMGUI_MANAGER_DESC& Desc)
{
	CGui_Object::GUI_OBJECT_DESC			ObjectDesc;
	ObjectDesc.pFocus = &m_isWindowFocused;
	ObjectDesc.pHovered = &m_isHovered;

	CNavi_Tool::NAVI_TOOL_DESC				PathToolDesc{ ObjectDesc };
	PathToolDesc.pCurObjectIndex = &m_iCurObjectIndex;
	PathToolDesc.pNavigations = &m_Components[static_cast<_uint>(COMPONENT_TYPE::_NAVIGATION)];
	PathToolDesc.pTransforms = &m_Components[static_cast<_uint>(COMPONENT_TYPE::_TRANSFORM)];

	CObstacle_Tool::OBSTACLE_TOOL_DESC		ObstacleToolDesc{ ObjectDesc };
	ObstacleToolDesc.pCurObjectIndex = &m_iCurObjectIndex;
	ObstacleToolDesc.pObstacles = &m_Components[static_cast<_uint>(COMPONENT_TYPE::_OBSTACLE)];

	CObject_Tool::OBJECT_TOOL_DESC			ObjectToolDesc{ ObjectDesc };
	ObjectToolDesc.pCurObjectIndex = &m_iCurObjectIndex;
	ObjectToolDesc.pComponents = &m_Components;
	ObjectToolDesc.pDummyObjects = &m_DummyObjects;
	

	if (FAILED(Insert_GuiObject(TEXT("Tool_Voxel"), CVoxel_Tool::Create(m_pDevice, m_pContext, Desc.pVoxel_Manager, &ObjectDesc))))
		return E_FAIL;
	if (FAILED(Insert_GuiObject(TEXT("Tool_Navi"), CNavi_Tool::Create(m_pDevice, m_pContext, Desc.pPath_Manager, &PathToolDesc))))
		return E_FAIL;
	if (FAILED(Insert_GuiObject(TEXT("Tool_Obstacle"), CObstacle_Tool::Create(m_pDevice, m_pContext, &ObstacleToolDesc))))
		return E_FAIL;
	if (FAILED(Insert_GuiObject(TEXT("Tool_Object"), CObject_Tool::Create(m_pDevice, m_pContext, &ObjectToolDesc))))
		return E_FAIL;

	return S_OK;
}

HRESULT CImGUI_Manager::Insert_GuiObject(const wstring& strTag, CGui_Object* pObject)
{
	auto iter{ m_Objects.find(strTag) };
	if (iter != m_Objects.end())
		return E_FAIL;

	m_Objects.emplace(strTag, pObject);

	return S_OK;
}

HRESULT CImGUI_Manager::Erase_GuiObject(const wstring& strTag)
{
	auto iter{ m_Objects.find(strTag) };
	if (iter == m_Objects.end())
		return E_FAIL;

	Safe_Release(iter->second);
	m_Objects.erase(iter);

	return S_OK;
}

HRESULT CImGUI_Manager::Clear_GuiObjects()
{
	for (auto& Pair : m_Objects)
	{
		Safe_Release(Pair.second);
	}
	m_Objects.clear();

	return S_OK;
}

IMGUI_IMPL_API LRESULT CImGUI_Manager::ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ::ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

CImGUI_Manager* CImGUI_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd, IMGUI_MANAGER_DESC* pDesc)
{
	CImGUI_Manager* pInstance = { new CImGUI_Manager(pDevice, pContext) };
	if (FAILED(pInstance->Initialize(hWnd, pDesc)))
	{
		MSG_BOX(TEXT("Failed To Create : CImGUI_Manager"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CImGUI_Manager::Free()
{
	__super::Free();

	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX11_Shutdown();

	ImGui::DestroyContext();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Clear_GuiObjects();
}

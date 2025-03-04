#include "..\Public\GameInstance.h"

#include "Graphic_Device.h"
#include "Input_Device.h"
#include "Object_Manager.h"
#include "Target_Manager.h"
#include "Level_Manager.h"
#include "Timer_Manager.h"
#include "Light_Manager.h"
#include "Font_Manager.h"
#include "Voxel_Manager.h"
#include "Path_Manager.h"
#include "Boid_Manager.h"
#include "Formation_Manager.h"
#include "ImGUI_Manager.h"

#include "Extractor.h"
#include "Renderer.h"
#include "Frustum.h"
#include "Picking.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()
{

}

HRESULT CGameInstance::Initialize_Engine(HINSTANCE hInstance, _uint iNumLevels, const ENGINE_DESC& EngineDesc, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext)
{
	/* 그래픽 디바이스를 초기화한다 .*/
	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc, ppDevice, ppContext);
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pInput_Device = CInput_Device::Create(EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return E_FAIL;

	m_pPipeLine = CPipeLine::Create();
	if (nullptr == m_pPipeLine)
		return E_FAIL;

	m_pLight_Manager = CLight_Manager::Create();
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	m_pPicking = CPicking::Create(*ppDevice, *ppContext, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY);
	if (nullptr == m_pPicking)
		return E_FAIL;

	/*m_pFont_Manager = CFont_Manager::Create(*ppGraphic_Device);
	if (nullptr == m_pFont_Manager)
		return E_FAIL;*/
	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	m_pTarget_Manager = CTarget_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	m_pRenderer = CRenderer::Create(*ppDevice, *ppContext);
	if (nullptr == m_pRenderer)
		return E_FAIL;

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	m_pVoxel_Manager = CVoxel_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	m_pPath_Manager = CPath_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	m_pBoid_Manager = CBoid_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pBoid_Manager)
		return E_FAIL;

	m_pFormation_Manager = CFormation_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pFormation_Manager)
		return E_FAIL;

	CImGUI_Manager::IMGUI_MANAGER_DESC			ImGuiDesc;
	ImGuiDesc.pVoxel_Manager = m_pVoxel_Manager;
	ImGuiDesc.pPath_Manager = m_pPath_Manager;

	m_pImGUI_Manager = CImGUI_Manager::Create(*ppDevice, *ppContext, EngineDesc.hWnd, &ImGuiDesc);
	if (nullptr == m_pImGUI_Manager)
		return E_FAIL;

	/* 인풋 디바이스를 초기화한다 .*/

	/* 사운드 디바이스를 초기화한다 .*/

	/* 오브젝트 매니져의 공간 예약을 한다. */
	m_pObject_Manager = CObject_Manager::Create(iNumLevels);
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	/* 컴포넌트 매니져의 공간 예약을 한다. */
	m_pComponent_Manager = CComponent_Manager::Create(iNumLevels);
	if (nullptr == m_pComponent_Manager)
		return E_FAIL;

	m_pFont_Manager = CFont_Manager::Create();
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	if (FAILED(Ready_Fonts(*ppDevice, *ppContext)))
		return E_FAIL;

	m_pFrustum = CFrustum::Create();
	if (nullptr == m_pFrustum)
		return E_FAIL;

	m_pExtractor = CExtractor::Create(*ppDevice, *ppContext);
	if (nullptr == m_pExtractor)
		return E_FAIL;


	m_WinSize.iSizeX = EngineDesc.iWinSizeX;
	m_WinSize.iSizeY = EngineDesc.iWinSizeY;

	return S_OK;
}

void CGameInstance::Tick_Engine(_float fTimeDelta)
{
	if (nullptr == m_pLevel_Manager ||
		nullptr == m_pObject_Manager ||
		nullptr == m_pPipeLine)
		return;

	m_pInput_Device->Tick(fTimeDelta);

	/* 마우스의 이동을 구했으면 바로 피킹Tick을 통해서 마우스의 월드 좌표를 구한다. */
	m_pPicking->Tick();

	m_pObject_Manager->Priority_Tick(fTimeDelta);

	m_pVoxel_Manager->Priority_Tick(fTimeDelta);

	m_pPath_Manager->Priority_Tick(fTimeDelta);

	m_pFormation_Manager->Tick(fTimeDelta);

	m_pObject_Manager->Tick(fTimeDelta);

	m_pVoxel_Manager->Tick(fTimeDelta);

	m_pPath_Manager->Tick(fTimeDelta);

	m_pPipeLine->Tick();

	m_pFrustum->Tick();

	m_pObject_Manager->Late_Tick(fTimeDelta);

	/* 반복적인 갱신이 필요한 객체들의 Tick함수를 호출한다. */
	m_pLevel_Manager->Tick(fTimeDelta);

	if (nullptr != m_pImGUI_Manager)
		m_pImGUI_Manager->Tick(fTimeDelta);

}

HRESULT CGameInstance::Begin_Draw(const _float4& vClearColor)
{
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pGraphic_Device->Clear_BackBuffer_View(vClearColor);
	m_pGraphic_Device->Clear_DepthStencil_View();

	return S_OK;
}

HRESULT CGameInstance::End_Draw()
{
	return m_pGraphic_Device->Present();
}


HRESULT CGameInstance::Draw()
{
	if (nullptr == m_pGraphic_Device ||
		nullptr == m_pLevel_Manager)
		return E_FAIL;

	/* 화면에 그려져야할 객체들을 그리낟. == 오브젝트 매니져에 들어가있을꺼야 .*/
	/* 오브젝트 매니져에 렌더함수를 만들어서 호출하면 객체들을 다 그린다. */

	/* But. CRenderer객체의 렌더함수를 호출하여 객체를 그리낟. */

	if (FAILED(m_pPath_Manager->Render()))
		return E_FAIL;


	/*if (FAILED(m_pVoxel_Manager->Render()))
		return E_FAIL;*/

	if (FAILED(m_pRenderer->Render()))
		return E_FAIL;

	if (FAILED(m_pLevel_Manager->Render()))
		return E_FAIL;

	if (nullptr != m_pImGUI_Manager)
	{
		if (FAILED(m_pImGUI_Manager->Render()))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CGameInstance::Clear(_uint iClearLevelIndex)
{
	if (nullptr == m_pObject_Manager ||
		nullptr == m_pComponent_Manager)
		return E_FAIL;

	/* 지정된 레벨용 자원(텍스쳐, 사운드, 객체등등) 을 삭제한다. */

	/* 사본 게임오브젝트. */
	m_pObject_Manager->Clear(iClearLevelIndex);

	/* 컴포넌트 원형 */
	m_pComponent_Manager->Clear(iClearLevelIndex);


	return S_OK;
}

#pragma region Input_Device
_uint CGameInstance::Get_KeyState(_int iKey)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_KeyState(iKey);
}

_float CGameInstance::Get_Key_Press_Time(_int iKey)
{
	if (nullptr == m_pInput_Device)
		return 0.f;

	return m_pInput_Device->Get_Key_Press_Time(iKey);
}

void CGameInstance::Update_WHEEL(_int iDelta)
{
	if (nullptr == m_pInput_Device)
		return;

	return m_pInput_Device->Update_WHEEL(iDelta);
}

_bool CGameInstance::Check_Wheel_Down()
{
	if (nullptr == m_pInput_Device)
		return false;

	return m_pInput_Device->Check_Wheel_Down();
}

_bool CGameInstance::Check_Wheel_Up()
{
	if (nullptr == m_pInput_Device)
		return false;

	return m_pInput_Device->Check_Wheel_Up();
}

_int CGameInstance::Check_Wheel_Delta()
{
	if (nullptr == m_pInput_Device)
		return false;

	return m_pInput_Device->Check_Wheel_Delta();
}

void CGameInstance::Wheel_Clear()
{
	if (nullptr == m_pInput_Device)
		return;

	return m_pInput_Device->Wheel_Clear();
}

POINT CGameInstance::Get_MouseCurPos()
{
	if (nullptr == m_pInput_Device)
		return POINT{};

	return m_pInput_Device->Get_MouseCurPos();
}

POINT CGameInstance::Get_MouseDeltaPos()
{
	if (nullptr == m_pInput_Device)
		return POINT{};

	return m_pInput_Device->Get_MouseDeltaPos();
}

void CGameInstance::Set_MouseCurPos(POINT ptPos)
{
	if (nullptr == m_pInput_Device)
		return;

	m_pInput_Device->Set_MouseCurPos(ptPos);
}
#pragma endregion

HRESULT CGameInstance::Add_RenderGroup(CRenderer::RENDERGROUP eRenderGroup, CGameObject* pRenderObject)
{
	if (nullptr == m_pRenderer)
		return E_FAIL;

	return m_pRenderer->Add_RenderGroup(eRenderGroup, pRenderObject);
}

#ifdef _DEBUG
HRESULT CGameInstance::Add_DebugComponents(CComponent* pRenderComponent)
{

	if (nullptr == m_pRenderer)
		return E_FAIL;

	return m_pRenderer->Add_DebugComponents(pRenderComponent);
}
#endif

HRESULT CGameInstance::Open_Level(_uint iNewLevelID, CLevel* pNewLevel)
{
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	return m_pLevel_Manager->Open_Level(iNewLevelID, pNewLevel);
}

HRESULT CGameInstance::Add_Prototype(const wstring& strPrototypeTag, CGameObject* pPrototype)
{
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Add_Prototype(strPrototypeTag, pPrototype);
}

HRESULT CGameInstance::Add_Clone(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strPrototypeTag, void* pArg)
{
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Add_Clone(iLevelIndex, strLayerTag, strPrototypeTag, pArg);
}

CGameObject* CGameInstance::Clone_GameObject(const wstring& strPrototypeTag, void* pArg)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Clone_GameObject(strPrototypeTag, pArg);
}

const CComponent* CGameInstance::Get_Component(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strComTag, _uint iIndex)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strComTag, iIndex);
}

HRESULT CGameInstance::Add_Prototype(_uint iLevelIndex, const wstring& strPrototypeTag, CComponent* pPrototype)
{
	if (nullptr == m_pComponent_Manager)
		return E_FAIL;

	return m_pComponent_Manager->Add_Prototype(iLevelIndex, strPrototypeTag, pPrototype);
}

CComponent* CGameInstance::Clone_Component(_uint iLevelIndex, const wstring& strPrototypeTag, void* pArg)
{
	if (nullptr == m_pComponent_Manager)
		return nullptr;

	return m_pComponent_Manager->Clone_Component(iLevelIndex, strPrototypeTag, pArg);
}

HRESULT CGameInstance::Add_Timer(const wstring& strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	return m_pTimer_Manager->Add_Timer(strTimerTag);
}

_float CGameInstance::Compute_TimeDelta(const wstring& strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return 0.0f;

	return m_pTimer_Manager->Compute_TimeDelta(strTimerTag);
}

void CGameInstance::Set_Transform(CPipeLine::TRANSFORMSTATE eState, _fmatrix TransformMatrix)
{
	if (nullptr == m_pPipeLine)
		return;

	m_pPipeLine->Set_Transform(eState, TransformMatrix);
}

_matrix CGameInstance::Get_Transform_Matrix(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return XMMatrixIdentity();

	return m_pPipeLine->Get_Transform_Matrix(eState);
}

_float4x4 CGameInstance::Get_Transform_Float4x4(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return _float4x4();

	return m_pPipeLine->Get_Transform_Float4x4(eState);
}

_matrix CGameInstance::Get_Transform_Matrix_Inverse(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return XMMatrixIdentity();

	return m_pPipeLine->Get_Transform_Matrix_Inverse(eState);
}

_float4x4 CGameInstance::Get_Transform_Float4x4_Inverse(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return _float4x4();

	return m_pPipeLine->Get_Transform_Float4x4_Inverse(eState);
}

_vector CGameInstance::Get_CamPosition_Vector() const
{
	if (nullptr == m_pPipeLine)
		return XMVectorZero();

	return m_pPipeLine->Get_CamPosition_Vector();
}

_float4 CGameInstance::Get_CamPosition_Float4() const
{
	if (nullptr == m_pPipeLine)
		return _float4();

	return m_pPipeLine->Get_CamPosition_Float4();
}

const LIGHT_DESC* CGameInstance::Get_LightDesc(_uint iIndex)
{
	if (nullptr == m_pLight_Manager)
		return nullptr;

	return m_pLight_Manager->Get_LightDesc(iIndex);
}

HRESULT CGameInstance::Add_Light(const LIGHT_DESC& LightDesc)
{
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	return m_pLight_Manager->Add_Light(LightDesc);
}

HRESULT CGameInstance::Render_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	return m_pLight_Manager->Render(pShader, pVIBuffer);
}

HRESULT CGameInstance::Add_Font(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strFontTag, const wstring& strFontFilePath)
{
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	return m_pFont_Manager->Add_Font(pDevice, pContext, strFontTag, strFontFilePath);
}

HRESULT CGameInstance::Render_Font(const wstring& strFontTag, const wstring& strText, const _float2& vPosition, _fvector vColor, _float fRadian)
{
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	return m_pFont_Manager->Render(strFontTag, strText, vPosition, vColor, fRadian);
}

HRESULT CGameInstance::Render_Font_Scaled(const wstring& strFontTag, const wstring& strText, const _float2& vPosition, _fvector vColor, _float fScale)
{
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	return m_pFont_Manager->Render_Scaled(strFontTag, strText, vPosition, vColor, fScale);
}

HRESULT CGameInstance::Add_RenderTarget(const wstring& strRenderTargetTag, _uint iSizeX, _uint iSizeY, DXGI_FORMAT ePixelFormat, const _float4& vClearColor)
{
	return m_pTarget_Manager->Add_RenderTarget(strRenderTargetTag, iSizeX, iSizeY, ePixelFormat, vClearColor);
}

HRESULT CGameInstance::Add_MRT(const wstring& strMRTTag, const wstring& strRenderTargetTag)
{
	return m_pTarget_Manager->Add_MRT(strMRTTag, strRenderTargetTag);
}

HRESULT CGameInstance::Begin_MRT(const wstring& strMRTTag, ID3D11DepthStencilView* pDSV)
{
	return m_pTarget_Manager->Begin_MRT(strMRTTag, pDSV);
}

HRESULT CGameInstance::End_MRT()
{
	return m_pTarget_Manager->End_MRT();
}

HRESULT CGameInstance::Bind_RTShaderResource(CShader* pShader, const wstring& strRenderTargetTag, const _char* pConstantName)
{

	return m_pTarget_Manager->Bind_ShaderResource(pShader, strRenderTargetTag, pConstantName);
}

HRESULT CGameInstance::Copy_Resource(const wstring& strRenderTargetTag, ID3D11Texture2D** ppTextureHub)
{
	return m_pTarget_Manager->Copy_Resource(strRenderTargetTag, ppTextureHub);
}

_bool CGameInstance::isInFrustum_WorldSpace(_fvector vWorldPos, _float fRange)
{
	return m_pFrustum->isIn_WorldSpace(vWorldPos, fRange);
}

_bool CGameInstance::isInFrustum_LocalSpace(_fvector vLocalPos, _float fRange)
{
	return m_pFrustum->isIn_LocalSpace(vLocalPos, fRange);
}

void CGameInstance::TransformFrustum_LocalSpace(_fmatrix WorldMatrixInv)
{
	m_pFrustum->Transform_LocalSpace(WorldMatrixInv);
}

_vector CGameInstance::Compute_WorldPos(const _float2& vViewportPos, const wstring& strZRenderTargetTag, _uint iOffset)
{
	return m_pExtractor->Compute_WorldPos(vViewportPos, strZRenderTargetTag, iOffset);
}

HRESULT CGameInstance::Make_Path_Voxel(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Make_Path_Voxel(PathIndices, NodeInfos, iStartIndex, GoalIndices, isDetourStaticObstacle, isDetourDynamicObstacle, MyReserveIndices);
}

HRESULT CGameInstance::Make_Path_Inverse_Voxel(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, const _uint iGoalIndex, const _bool isDetourStaticObstacle)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Make_Path_Inverse_Voxel(NodeInfos, StartIndices, iGoalIndex, isDetourStaticObstacle);
}

list<_float3> CGameInstance::Get_BestPath_Positions_Float_Optimized()
{
	if (nullptr == m_pPath_Manager)
		return list<_float3>();

	return m_pPath_Manager->Get_BestPath_Positions_Float_Optimized();
}

const list<_uint>& CGameInstance::Get_BestPath_Indices_Optimized()
{
	static list<_uint>			Temp;
	if (nullptr == m_pPath_Manager)
		return Temp;

	return m_pPath_Manager->Get_BestPath_Indices_Optimized();
}

const vector<_float3>& CGameInstance::Get_BestPath_Positions_Smooth()
{
	static vector<_float3>			Temp;
	if (nullptr == m_pPath_Manager)
		return Temp;

	return m_pPath_Manager->Get_BestPath_Positions_Smooth();
}

HRESULT CGameInstance::Clear_PathInfos()
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Clear_PathInfos();
}

const unordered_set<_uint>& CGameInstance::Get_OpenList()
{
	static unordered_set<_uint>			Temp;
	if (nullptr == m_pPath_Manager)
		return Temp;

	return m_pPath_Manager->Get_OpenList();
}

const unordered_set<_uint>& CGameInstance::Get_CloseList()
{
	static unordered_set<_uint>			Temp;
	if (nullptr == m_pPath_Manager)
		return Temp;

	return m_pPath_Manager->Get_CloseList();
}

ALGORITHM CGameInstance::Get_Algorithm()
{
	if (nullptr == m_pPath_Manager)
		return ALGORITHM::_END;

	return m_pPath_Manager->Get_Algorithm();
}

MAIN_HEURISTIC CGameInstance::Get_MainHeuristic()
{
	if (nullptr == m_pPath_Manager)
		return MAIN_HEURISTIC::_END;

	return m_pPath_Manager->Get_MainHeuristic();
}

void CGameInstance::Set_Algorithm(ALGORITHM eAlgorithm)
{
	if (nullptr == m_pPath_Manager)
		return;

	return m_pPath_Manager->Set_Algorithm(eAlgorithm);
}

void CGameInstance::Set_MainHeuristic(MAIN_HEURISTIC eHeuristic)
{
	if (nullptr == m_pPath_Manager)
		return;

	return m_pPath_Manager->Set_MainHeuristic(eHeuristic);
}


void CGameInstance::Set_MaxCost(const _float fMaxCost)
{
	if (nullptr == m_pPath_Manager)
		return;

	return m_pPath_Manager->Set_MaxCost(fMaxCost);
}

TEST_MODE CGameInstance::Get_OptimizerTestMode()
{
	if (nullptr == m_pPath_Manager)
		return TEST_MODE::_END;

	return m_pPath_Manager->Get_OptimizerTestMode();
}

void CGameInstance::Set_OptimizerTestMode(TEST_MODE eTestMode)
{
	if (nullptr == m_pPath_Manager)
		return;

	return m_pPath_Manager->Set_OptimizerTestMode(eTestMode);
}

_float CGameInstance::Get_HeuristicWeights(HEURISTIC_TYPE eHeuriticType)
{
	if (nullptr == m_pPath_Manager)
		return 0.f;

	return m_pPath_Manager->Get_HeuristicWeights(eHeuriticType);
}

void CGameInstance::Set_HeuristicWeights(HEURISTIC_TYPE eHeuriticType, _float fWeight)
{
	if (nullptr == m_pPath_Manager)
		return;

	return m_pPath_Manager->Set_HeuristicWeights(eHeuriticType, fWeight);
}

_float CGameInstance::Compute_Total_Dist_BestPath(const list<_uint>& PathIndices)
{
	if (nullptr == m_pPath_Manager)
		return 0.f;

	return m_pPath_Manager->Compute_Total_Dist_BestPath(PathIndices);
}

_float CGameInstance::Compute_Total_Dist_OptimizedPath()
{
	if (nullptr == m_pPath_Manager)
		return 0.f;

	return m_pPath_Manager->Compute_Total_Dist_OptimizedPath();
}

_llong CGameInstance::Get_Make_Path_Time()
{
	if (nullptr == m_pPath_Manager)
		return _llong();

	return m_pPath_Manager->Get_Make_Path_Time();
}

_llong CGameInstance::Get_Optimize_Path_Time()
{
	if (nullptr == m_pPath_Manager)
		return _llong();

	return m_pPath_Manager->Get_Optimize_Path_Time();
}

_llong CGameInstance::Get_Smooth_Path_Time()
{
	if (nullptr == m_pPath_Manager)
		return _llong();

	return m_pPath_Manager->Get_Smooth_Path_Time();
}

HRESULT CGameInstance::Make_VectorFiled_Daijkstra(const vector<_uint>& GoalIndices, const _uint iNumStartAgent, _uint& iIndexTag)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Make_VectorFiled_Daijkstra(GoalIndices, iNumStartAgent, iIndexTag);
}

HRESULT CGameInstance::Make_VectorFiled_A_Star(const _uint iGoalIndex, const vector<_uint>& StartIndices, const _uint iNumStartAgent, _uint& iIndexTag)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Make_VectorFiled_A_Star(iGoalIndex, StartIndices, iNumStartAgent, iIndexTag);
}

HRESULT CGameInstance::Get_Direction_FlowField(const _float3& vCurPos, const _uint iIndexTag, _float3& vDirection)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Get_Direction_FlowField(vCurPos, iIndexTag, vDirection);
}

HRESULT CGameInstance::Get_ParentIndex_FlowField(const _float3& vCurPos, const _uint iIndexTag, _uint& iParentIndex)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Get_ParentIndex_FlowField(vCurPos, iIndexTag, iParentIndex);
}

_float CGameInstance::Get_Distance_From_Goal_FlowField(const _uint iVoxelIndex, const _uint iIndexTag)
{
	if (nullptr == m_pPath_Manager)
		return FLT_MAX;

	return m_pPath_Manager->Get_Distance_From_Goal_FlowField(iVoxelIndex, iIndexTag);
}

HRESULT CGameInstance::AddRef_FlowField(const _uint iIndexTag)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->AddRef_FlowField(iIndexTag);
}

HRESULT CGameInstance::Release_FlowField(const _uint iIndexTag)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Release_FlowField(iIndexTag);
}

vector<_uint> CGameInstance::Get_Active_FlowField_RefCnt()
{
	if (nullptr == m_pPath_Manager)
		return vector<_uint>();

	return m_pPath_Manager->Get_Active_FlowField_RefCnt();
}

vector<_uint> CGameInstance::Get_Active_FlowField_Indices()
{
	if (nullptr == m_pPath_Manager)
		return vector<_uint>();

	return m_pPath_Manager->Get_Active_FlowField_Indices();
}

HRESULT CGameInstance::Convert_BestList_From_FlowField(list<_uint>& BestList, const _uint iIndexTag, const _uint iStartIndex)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Convert_BestList_From_FlowField(BestList, iIndexTag, iStartIndex);
}

HRESULT CGameInstance::Resize_Voxel(_float fSize)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Resizing_Voxel(fSize);
}

void CGameInstance::Set_Render_VoxelLayer(_bool isRender, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return;

	m_pVoxel_Manager->Set_Render(eLayer, isRender);
}

void CGameInstance::Set_Render_VoxelLayer_All(_bool isRender)
{
	if (nullptr == m_pVoxel_Manager)
		return;

	m_pVoxel_Manager->Set_Render_All(isRender);
}

HRESULT CGameInstance::Render_Voxels()
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Render();
}

HRESULT CGameInstance::Cooking_Terrain_Static_Voxels(CVIBuffer_Terrain* pTerrain, CTransform* pTerrainTransform)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Cooking_Terrain_Static_Voxels(pTerrain, pTerrainTransform);
}

HRESULT CGameInstance::Cooking_Model_Static_Voxels(CModel* pModel, CTransform* pModelTransform)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Cooking_Model_Static_Voxels(pModel, pModelTransform);
}

vector<_uint> CGameInstance::Get_NeighborIndices_Voxel(const _float3& vPos, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return vector<_uint>();

	return m_pVoxel_Manager->Get_NeighborIndices(vPos, eLayer);
}

vector<_uint> CGameInstance::Get_NeighborIndices_Voxel(_uint iIndex, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return vector<_uint>();

	return m_pVoxel_Manager->Get_NeighborIndices(iIndex, eLayer);
}

vector<_uint> CGameInstance::Get_NeighborIndices_All_Voxel(const _float3& vPos, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return vector<_uint>();

	return m_pVoxel_Manager->Get_NeighborIndices_All(vPos, eLayer);
}

vector<_uint> CGameInstance::Get_NeighborIndices_All_Voxel(_uint iIndex, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return vector<_uint>();

	return m_pVoxel_Manager->Get_NeighborIndices_All(iIndex, eLayer);
}

HRESULT CGameInstance::Get_Index_Voxel(const _float3& vPos, _uint& iIndex)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_Index(vPos, iIndex);
}


HRESULT CGameInstance::Get_Index_Voxel(const _uint3& vIndexPos, _uint& iIndex)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_Index(vIndexPos, iIndex);
}

HRESULT CGameInstance::Get_WorldPosition_Voxel(_uint iIndex, _float3& vPos)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_WorldPosition(iIndex, vPos);
}

HRESULT CGameInstance::Get_WorldPosition_Voxel(const _uint3& vIndexPos, _float3& vPos)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_WorldPosition(vIndexPos, vPos);
}


HRESULT CGameInstance::Get_IndexPosition_Voxel(_uint iIndex, _uint3& vIndexPos)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_IndexPosition(iIndex, vIndexPos);
}

HRESULT CGameInstance::Get_IndexPosition_Voxel(const _float3& vPos, _uint3& vIndexPos)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_IndexPosition(vPos, vIndexPos);
}

HRESULT CGameInstance::Compute_Picking_Voxel(_uint& iPickIndex, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Compute_Picking_Voxel(iPickIndex, eLayer);
}

_bool CGameInstance::Is_Reach_TargetIndex(const _uint iStartIndex, const _uint iTargetIndex, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return false;

	return m_pVoxel_Manager->Is_Reach_TargetIndex(iStartIndex, iTargetIndex, eLayer);
}

_uint CGameInstance::Get_NumVoxel_Active_In_Layer(const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return 0;

	return m_pVoxel_Manager->Get_NumVoxel_Active_In_Layer(eLayer);
}

_uint CGameInstance::Get_NumVoxel_Active_All_Layer()
{
	if (nullptr == m_pVoxel_Manager)
		return 0;

	return m_pVoxel_Manager->Get_NumVoxel_Active_All_Layer();
}

_uint CGameInstance::Get_NumVoxel_In_Layer(const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return 0;

	return m_pVoxel_Manager->Get_NumVoxel_In_Layer(eLayer);
}

_uint CGameInstance::Get_NumVoxel_All_Layer()
{
	if (nullptr == m_pVoxel_Manager)
		return 0;

	return m_pVoxel_Manager->Get_NumVoxel_All_Layer();
}

_uint CGameInstance::Get_NumSector(const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return 0;

	return m_pVoxel_Manager->Get_NumSector(eLayer);
}

_uint CGameInstance::Get_NumSector_Active()
{
	if (nullptr == m_pVoxel_Manager)
		return 0;

	return m_pVoxel_Manager->Get_NumSector_Active();
}

_uint CGameInstance::Get_NumVoxelWorldDim()
{
	if (nullptr == m_pVoxel_Manager)
		return 0;

	return m_pVoxel_Manager->Get_NumVoxelWorldDim();
}

_float CGameInstance::Get_WorldSize_Voxel()
{
	if (nullptr == m_pVoxel_Manager)
		return 0.f;

	return m_pVoxel_Manager->Get_WorldSize_Voxel();
}

HRESULT CGameInstance::Get_VoxelID(const _float3& vWorldPos, VOXEL_ID& eID, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_VoxelID(vWorldPos, eID, eLayer);
}

HRESULT CGameInstance::Set_VoxelID(const _float3& vWorldPos, VOXEL_ID eID, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Set_VoxelID(vWorldPos, eID, eLayer);
}

HRESULT CGameInstance::Get_VoxelID(const _uint3& vIndexPos, VOXEL_ID& eID, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_VoxelID(vIndexPos, eID, eLayer);
}

HRESULT CGameInstance::Set_VoxelID(const _uint3& vIndexPos, VOXEL_ID eID, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Set_VoxelID(vIndexPos, eID, eLayer);
}

HRESULT CGameInstance::Get_VoxelID(const _uint iVoxelIndex, VOXEL_ID& eID, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_VoxelID(iVoxelIndex, eID, eLayer);
}

HRESULT CGameInstance::Set_VoxelID(const _uint iVoxelIndex, VOXEL_ID eID, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Set_VoxelID(iVoxelIndex, eID, eLayer);
}

HRESULT CGameInstance::Get_VoxelState(const _float3& vWorldPos, VOXEL_STATE& eState, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_VoxelState(vWorldPos, eState, eLayer);
}

HRESULT CGameInstance::Set_VoxelState(const _float3& vWorldPos, VOXEL_STATE eState, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Set_VoxelState(vWorldPos, eState, eLayer);
}

HRESULT CGameInstance::Get_VoxelState(const _uint3& vIndexPos, VOXEL_STATE& eState, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_VoxelState(vIndexPos, eState, eLayer);
}

HRESULT CGameInstance::Set_VoxelState(const _uint3& vIndexPos, VOXEL_STATE eState, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Set_VoxelState(vIndexPos, eState, eLayer);
}

HRESULT CGameInstance::Get_VoxelState(const _uint iVoxelIndex, VOXEL_STATE& eState, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_VoxelState(iVoxelIndex, eState, eLayer);
}

HRESULT CGameInstance::Set_VoxelState(const _uint iVoxelIndex, VOXEL_STATE eState, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Set_VoxelState(iVoxelIndex, eState, eLayer);
}

HRESULT CGameInstance::Get_VoxelOpenessScore(const _float3& vWorldPos, _byte& bOpenessScore, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_VoxelOpenessScore(vWorldPos, bOpenessScore, eLayer);
}

HRESULT CGameInstance::Get_VoxelOpenessScore(const _uint3& vIndexPos, _byte& bOpenessScore, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_VoxelOpenessScore(vIndexPos, bOpenessScore, eLayer);
}

HRESULT CGameInstance::Get_VoxelOpenessScore(const _uint iVoxelIndex, _byte& bOpenessScore, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Get_VoxelOpenessScore(iVoxelIndex, bOpenessScore, eLayer);
}

HRESULT CGameInstance::Add_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Add_Voxel(iVoxelIndex, eLayer, eID, eState);
}


HRESULT CGameInstance::Add_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Add_Voxel(vIndexPos, eLayer, eID, eState);
}

HRESULT CGameInstance::Add_Voxel(const _float3& vWorldPos, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Add_Voxel(vWorldPos, eLayer, eID, eState);
}

HRESULT CGameInstance::Erase_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Erase_Voxel(iVoxelIndex, eLayer);
}


HRESULT CGameInstance::Erase_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Erase_Voxel(vIndexPos, eLayer);
}

HRESULT CGameInstance::Erase_Voxel(const _float3& vWorldPos, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Erase_Voxel(vWorldPos, eLayer);
}

_bool CGameInstance::Is_Exist_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return false;

	return m_pVoxel_Manager->Is_Exist_Voxel(iVoxelIndex, eLayer);
}
_bool CGameInstance::Is_Exist_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return false;

	return m_pVoxel_Manager->Is_Exist_Voxel(vIndexPos, eLayer);
}

_uint CGameInstance::Get_CurPicked_VoxelIndex(const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return 0;

	return m_pVoxel_Manager->Get_CurPicked_VoxelIndex(eLayer);
}

_uint CGameInstance::Get_PrePicked_VoxelIndex()
{
	if (nullptr == m_pVoxel_Manager)
		return 0;

	return m_pVoxel_Manager->Get_PrePicked_VoxelIndex();
}

_float3 CGameInstance::Get_CurPicked_VoxelWorldPos(const VOXEL_LAYER eLayer)
{
	if (nullptr == m_pVoxel_Manager)
		return _float3();

	return m_pVoxel_Manager->Get_CurPicked_VoxelWorldPos(eLayer);
}

_float3 CGameInstance::Get_PrePicked_VoxelWorldPos()
{
	if (nullptr == m_pVoxel_Manager)
		return _float3();

	return m_pVoxel_Manager->Get_PrePicked_VoxelWorldPos();
}

HRESULT CGameInstance::Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _uint iVoxelIndex, _float3& vNearestPosFloat3)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Compute_Nearest_Position_VoxelBounding(vStartPos, iVoxelIndex, vNearestPosFloat3);
}
HRESULT CGameInstance::Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _uint3& vVoxelIndexPos, _float3& vNearestPosFloat3)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Compute_Nearest_Position_VoxelBounding(vStartPos, vVoxelIndexPos, vNearestPosFloat3);
}
HRESULT CGameInstance::Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _float3& vVoxelWorldPos, _float3& vNearestPosFloat3)
{
	if (nullptr == m_pVoxel_Manager)
		return E_FAIL;

	return m_pVoxel_Manager->Compute_Nearest_Position_VoxelBounding(vStartPos, vVoxelWorldPos, vNearestPosFloat3);
}

HRESULT CGameInstance::Add_Boid(class CBoid* pBoid)
{
	if (nullptr == m_pBoid_Manager)
		return E_FAIL;

	return m_pBoid_Manager->Add_Boid(pBoid);
}
HRESULT CGameInstance::Erase_Boid(class CBoid* pBoid)
{
	if (nullptr == m_pBoid_Manager)
		return E_FAIL;

	return m_pBoid_Manager->Erase_Boid(pBoid);
}

HRESULT CGameInstance::Get_Neighbor_Position_Boid(const _uint iNeighborIndex, _float3& vNeighborPos)
{
	if (nullptr == m_pBoid_Manager)
		return E_FAIL;

	return m_pBoid_Manager->Get_Neighbor_Position_Boid(iNeighborIndex, vNeighborPos);
}

HRESULT CGameInstance::Get_Neighbor_Indices_Boid(class CBoid* pBoid, const _float fNeighborRange, vector<_uint>& NeighborIndices)
{
	if (nullptr == m_pBoid_Manager)
		return E_FAIL;

	return m_pBoid_Manager->Get_Neighbor_Indices_Boid(pBoid, fNeighborRange, NeighborIndices);
}

HRESULT CGameInstance::Get_Neighbor_FlowFieldIndex_Boid(const _uint iNeighborIndex, _uint& iFlowFieldIndexTag)
{
	if (nullptr == m_pBoid_Manager)
		return E_FAIL;

	return m_pBoid_Manager->Get_Neighbor_FlowFieldIndex_Boid(iNeighborIndex, iFlowFieldIndexTag);
}

HRESULT CGameInstance::Add_FormationGroup(_uint& iGroupIndex)
{
	if (nullptr == m_pFormation_Manager)
		return E_FAIL;

	return m_pFormation_Manager->Add_Group(iGroupIndex);
}

HRESULT CGameInstance::Erase_FormationGroup(const _uint iGroupIndex)
{
	if (nullptr == m_pFormation_Manager)
		return E_FAIL;

	return m_pFormation_Manager->Erase_Group(iGroupIndex);
}

HRESULT CGameInstance::Add_Formation(class CFormation* pFormation, const _uint iGroupIndex)
{
	if (nullptr == m_pFormation_Manager)
		return E_FAIL;

	return m_pFormation_Manager->Add_Formation(pFormation, iGroupIndex);
}

HRESULT CGameInstance::Erase_Formation(class CFormation* pFormation, const _uint iGroupIndex)
{
	if (nullptr == m_pFormation_Manager)
		return E_FAIL;

	return m_pFormation_Manager->Erase_Formation(pFormation, iGroupIndex);
}

HRESULT CGameInstance::Initiate_Formation(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex)
{
	if (nullptr == m_pFormation_Manager)
		return E_FAIL;

	return m_pFormation_Manager->Initiate_Formation(iGroupIndexTag, iStartIndex, iGoalIndex);
}


FORMATION_TYPE CGameInstance::Get_Formation_Type()
{
	if (nullptr == m_pFormation_Manager)
		return FORMATION_TYPE::_END;

	return m_pFormation_Manager->Get_Formation_Type();
}

void CGameInstance::Set_Formation_Type(const FORMATION_TYPE eType)
{
	if (nullptr == m_pFormation_Manager)
		return;

	return m_pFormation_Manager->Set_Formation_Type(eType);
}

HRESULT CGameInstance::Add_Render_Path(class CNavigation* pNavigation)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Add_Render_Path(pNavigation);
}

HRESULT CGameInstance::Add_Render_Path(const list<_uint>& PathIndices)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Add_Render_Path(PathIndices);
}

HRESULT CGameInstance::Set_FlowFieldRenderIndex(const _uint iIndexTag)
{
	if (nullptr == m_pPath_Manager)
		return E_FAIL;

	return m_pPath_Manager->Set_FlowFieldRenderIndex(iIndexTag);
}


void CGameInstance::Transform_PickingToLocalSpace_Perspective(const CTransform* pTransform, _Out_ _float3* pRayDir, _Out_ _float4* pRayPos)
{
	if (nullptr == m_pPicking ||
		nullptr == pRayDir ||
		nullptr == pRayPos)
		return;

	m_pPicking->Transform_PickingToLocalSpace_Perspective(pTransform, pRayDir, pRayPos);
}

void CGameInstance::Transform_PickingToLocalSpace_Perspective(_fmatrix WorldMatrix, _Out_ _float3* pRayDir, _Out_ _float4* pRayPos)
{
	if (nullptr == m_pPicking ||
		nullptr == pRayDir ||
		nullptr == pRayPos)
		return;

	m_pPicking->Transform_PickingToLocalSpace_Perspective(WorldMatrix, pRayDir, pRayPos);
}

_float4 CGameInstance::Get_MouseWorldPos()
{
	if (nullptr == m_pPicking)
		return _float4(0.f, 0.f, 0.f, 1.f);

	return m_pPicking->Get_MouseWorldPos();
}

_float3 CGameInstance::Get_MouseWorldDirection()
{
	if (nullptr == m_pPicking)
		return _float3(0.f, 0.f, 0.f);

	return m_pPicking->Get_MouseWorldDirection();
}

IMGUI_IMPL_API LRESULT CGameInstance::ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return m_pImGUI_Manager->ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

HRESULT CGameInstance::Insert_GuiObject(const wstring& strTag, CGui_Object* pObject)
{
	if (nullptr == m_pImGUI_Manager)
		return E_FAIL;

	return m_pImGUI_Manager->Insert_GuiObject(strTag, pObject);
}

HRESULT CGameInstance::Erase_GuiObject(const wstring& strTag)
{
	if (nullptr == m_pImGUI_Manager)
		return E_FAIL;

	return m_pImGUI_Manager->Erase_GuiObject(strTag);
}

HRESULT CGameInstance::Clear_GuiObjects()
{
	if (nullptr == m_pImGUI_Manager)
		return E_FAIL;

	return m_pImGUI_Manager->Clear_GuiObjects();
}

HRESULT CGameInstance::Ready_RTVDebug(const wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	return m_pTarget_Manager->Ready_Debug(strRenderTargetTag, fX, fY, fSizeX, fSizeY);
}
HRESULT CGameInstance::Draw_RTVDebug(const wstring& strMRTTag, CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pTarget_Manager->Render_Debug(strMRTTag, pShader, pVIBuffer);
}

HRESULT CGameInstance::Ready_Fonts(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// MakeSpriteFont "넥슨lv1고딕 Bold" /FontSize:30 /FastPack /CharacterRegion:0x0020-0x00FF /CharacterRegion:0x3131-0x3163 /CharacterRegion:0xAC00-0xD800 /DefaultCharacter:0xAC00 142.spritefont
	if (FAILED(Add_Font(pDevice, pContext, TEXT("Font_Debug"), TEXT("../../Client/Bin/Resources/Fonts/141ex.spriteFont"))))
		return E_FAIL;

	return S_OK;
}

void CGameInstance::Release_Engine()
{
	CGameInstance::Get_Instance()->Free();

	Destroy_Instance();
}

void CGameInstance::Free()
{
	Safe_Release(m_pFrustum);
	Safe_Release(m_pExtractor);
	Safe_Release(m_pTarget_Manager);
	Safe_Release(m_pFont_Manager);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pTimer_Manager);
	Safe_Release(m_pRenderer);
	Safe_Release(m_pObject_Manager);
	Safe_Release(m_pComponent_Manager);
	Safe_Release(m_pLevel_Manager);
	Safe_Release(m_pInput_Device);
	Safe_Release(m_pGraphic_Device);
	Safe_Release(m_pImGUI_Manager);
	Safe_Release(m_pVoxel_Manager);
	Safe_Release(m_pPath_Manager);
	Safe_Release(m_pBoid_Manager);
	Safe_Release(m_pFormation_Manager);
	Safe_Release(m_pPicking);
}


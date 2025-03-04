#pragma once

/* 클라이언트개발자가 엔진의 기능을 사용하기위해서 항상 접근해야하는 클래스. */
#include "Renderer.h"
#include "Component_Manager.h"
#include "PipeLine.h"

#include "Formation_Includes.h"

BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)

private:
	typedef struct tagWinSize
	{
		_uint			iSizeX;
		_uint			iSizeY;
} WIN_SIZE;

private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public:
	HRESULT Initialize_Engine(HINSTANCE hInstance, _uint iNumLevels, const ENGINE_DESC & EngineDesc, _Inout_ ID3D11Device * *ppDevice, _Inout_ ID3D11DeviceContext * *ppContext);
	void Tick_Engine(_float fTimeDelta);
	HRESULT Begin_Draw(const _float4 & vClearColor);
	HRESULT End_Draw();
	HRESULT Draw();
	HRESULT Clear(_uint iClearLevelIndex);

public: /* For.Input_Device */
	_uint Get_KeyState(_int iKey);
	_float Get_Key_Press_Time(_int iKey);
	void Update_WHEEL(_int iDelta);
	_bool Check_Wheel_Down();
	_bool Check_Wheel_Up();
	_int Check_Wheel_Delta();
	void Wheel_Clear();
	POINT Get_MouseCurPos();
	POINT Get_MouseDeltaPos();
	void Set_MouseCurPos(POINT ptPos);

public: /* For.Renderer */
	HRESULT Add_RenderGroup(CRenderer::RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);

#ifdef _DEBUG
public:
	HRESULT Add_DebugComponents(class CComponent* pRenderComponent);
#endif

public: /* For.Level_Manager */
	HRESULT Open_Level(_uint iNewLevelID, class CLevel* pNewLevel);

public: /* For.Object_Manager */
	HRESULT Add_Prototype(const wstring & strPrototypeTag, class CGameObject* pPrototype);
	HRESULT Add_Clone(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strPrototypeTag, void* pArg = nullptr);
	class CGameObject* Clone_GameObject(const wstring & strPrototypeTag, void* pArg = nullptr);
	const CComponent* Get_Component(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strComTag, _uint iIndex = 0);

public: /* For.Component_Manager */
	HRESULT Add_Prototype(_uint iLevelIndex, const wstring & strPrototypeTag, class CComponent* pPrototype);
	class CComponent* Clone_Component(_uint iLevelIndex, const wstring & strPrototypeTag, void* pArg = nullptr);


public: /* For.Timer_Manager */
	HRESULT Add_Timer(const wstring & strTimerTag);
	_float Compute_TimeDelta(const wstring & strTimerTag);

public: /* For.PipeLine */
	void Set_Transform(CPipeLine::TRANSFORMSTATE eState, _fmatrix TransformMatrix);
	_matrix Get_Transform_Matrix(CPipeLine::TRANSFORMSTATE eState) const;
	_float4x4 Get_Transform_Float4x4(CPipeLine::TRANSFORMSTATE eState) const;
	_matrix Get_Transform_Matrix_Inverse(CPipeLine::TRANSFORMSTATE eState) const;
	_float4x4 Get_Transform_Float4x4_Inverse(CPipeLine::TRANSFORMSTATE eState) const;
	_vector Get_CamPosition_Vector() const;
	_float4 Get_CamPosition_Float4() const;

public: /* For.Light_Manager */
	const LIGHT_DESC* Get_LightDesc(_uint iIndex);
	HRESULT Add_Light(const LIGHT_DESC & LightDesc);
	HRESULT Render_Lights(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);


public: /* For.Font_Manager */
	HRESULT Add_Font(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strFontTag, const wstring & strFontFilePath);
	HRESULT Render_Font(const wstring & strFontTag, const wstring & strText, const _float2 & vPosition, _fvector vColor, _float fRadian);
	HRESULT Render_Font_Scaled(const wstring & strFontTag, const wstring & strText, const _float2 & vPosition, _fvector vColor, _float fScale);


public: /* For.Target_Manager */
	HRESULT Add_RenderTarget(const wstring & strRenderTargetTag, _uint iSizeX, _uint iSizeY, DXGI_FORMAT ePixelFormat, const _float4 & vClearColor);
	HRESULT Add_MRT(const wstring & strMRTTag, const wstring & strRenderTargetTag);
	HRESULT Begin_MRT(const wstring & strMRTTag, ID3D11DepthStencilView * pDSV = nullptr);
	HRESULT End_MRT();
	HRESULT Bind_RTShaderResource(class CShader* pShader, const wstring & strRenderTargetTag, const _char * pConstantName);
	HRESULT Copy_Resource(const wstring & strRenderTargetTag, ID3D11Texture2D * *ppTextureHub);

public: /* For.Frustum */
	_bool isInFrustum_WorldSpace(_fvector vWorldPos, _float fRange = 0.f);
	_bool isInFrustum_LocalSpace(_fvector vLocalPos, _float fRange = 0.f);
	void TransformFrustum_LocalSpace(_fmatrix WorldMatrixInv);

public: /* For.Extractor */
	_vector Compute_WorldPos(const _float2 & vViewportPos, const wstring & strZRenderTargetTag, _uint iOffset = 0);

public:	/* For.Path_Manager */
	HRESULT Make_Path_Voxel(list<_uint>& PathIndices, NODE_INFOS& NodeInfos, _uint iStartIndex, const vector<_uint>& GoalIndices, const _bool isDetourStaticObstacle, const _bool isDetourDynamicObstacle, const vector<_uint>& MyReserveIndices);
	HRESULT Make_Path_Inverse_Voxel(NODE_INFOS& NodeInfos, const vector<_uint>& StartIndices, const _uint iGoalIndex, const _bool isDetourStaticObstacle);

	list<_float3> Get_BestPath_Positions_Float_Optimized();
	const list<_uint>& Get_BestPath_Indices_Optimized();
	const vector<_float3>& Get_BestPath_Positions_Smooth();
	HRESULT Clear_PathInfos();

	const unordered_set<_uint>& Get_OpenList();
	const unordered_set<_uint>& Get_CloseList();

	ALGORITHM Get_Algorithm();
	MAIN_HEURISTIC Get_MainHeuristic();
	void Set_Algorithm(ALGORITHM eAlgorithm);
	void Set_MainHeuristic(MAIN_HEURISTIC eHeuristic);
	void Set_MaxCost(const _float fMaxCost);
	TEST_MODE Get_OptimizerTestMode();
	void Set_OptimizerTestMode(TEST_MODE eTestMode);
	_float Get_HeuristicWeights(HEURISTIC_TYPE eHeuriticType);
	void Set_HeuristicWeights(HEURISTIC_TYPE eHeuriticType, _float fWeight);

	_float Compute_Total_Dist_BestPath(const list<_uint>& PathIndices);
	_float Compute_Total_Dist_OptimizedPath();

	_llong Get_Make_Path_Time();
	_llong Get_Optimize_Path_Time();
	_llong Get_Smooth_Path_Time();

	HRESULT Make_VectorFiled_Daijkstra(const vector<_uint>& GoalIndices, const _uint iNumStartAgent, _uint& iIndexTag);
	HRESULT Make_VectorFiled_A_Star(const _uint iGoalIndex, const vector<_uint>& StartIndices, const _uint iNumStartAgent, _uint& iIndexTag);
	HRESULT Get_Direction_FlowField(const _float3& vCurPos, const _uint iIndexTag, _float3& vDirection);
	HRESULT Get_ParentIndex_FlowField(const _float3& vCurPos, const _uint iIndexTag, _uint& iParentIndex);
	_float Get_Distance_From_Goal_FlowField(const _uint iVoxelIndex, const _uint iIndexTag);

	HRESULT AddRef_FlowField(const _uint iIndexTag);
	HRESULT Release_FlowField(const _uint iIndexTag);

	vector<_uint> Get_Active_FlowField_RefCnt();
	vector<_uint> Get_Active_FlowField_Indices();

	HRESULT Convert_BestList_From_FlowField(list<_uint>& BestList, const _uint iIndexTag, const _uint iStartIndex);


public: /* For.Voxel_Manager */
	_uint Get_NumVoxelWorldDim();
	HRESULT Resize_Voxel(_float fSize);

	void Set_Render_VoxelLayer(_bool isRender, const VOXEL_LAYER eLayer);
	void Set_Render_VoxelLayer_All(_bool isRender);

	HRESULT Render_Voxels();

	HRESULT Cooking_Terrain_Static_Voxels(CVIBuffer_Terrain* pTerrain, CTransform* pTerrainTransform);
	HRESULT Cooking_Model_Static_Voxels(CModel* pModel, CTransform* pModelTransform);

	vector<_uint> Get_NeighborIndices_Voxel(const _float3 & vPos, const VOXEL_LAYER eLayer);
	vector<_uint> Get_NeighborIndices_Voxel(_uint iIndex, const VOXEL_LAYER eLayer);
	vector<_uint> Get_NeighborIndices_All_Voxel(const _uint iIndex, VOXEL_LAYER eLayer);
	vector<_uint> Get_NeighborIndices_All_Voxel(const _float3& vPos, VOXEL_LAYER eLayer);

	HRESULT Get_Index_Voxel(const _float3 & vPos, _uint & iIndex);
	HRESULT Get_Index_Voxel(const _uint3 & vIndexPos, _uint & iIndex);
	HRESULT Get_WorldPosition_Voxel(_uint iIndex, _float3 & vPos);
	HRESULT Get_WorldPosition_Voxel(const _uint3& vIndexPos, _float3 & vPos);
	HRESULT Get_IndexPosition_Voxel(_uint iIndex, _uint3 & vIndexPos);
	HRESULT Get_IndexPosition_Voxel(const _float3& vPos, _uint3 & vIndexPos);

	HRESULT Compute_Picking_Voxel(_uint & iPickIndex, const VOXEL_LAYER eLayer);
	_bool Is_Reach_TargetIndex(const _uint iStartIndex, const _uint iTargetIndex, const VOXEL_LAYER eLayer);

	_uint Get_NumVoxel_Active_In_Layer(const VOXEL_LAYER eLayer);
	_uint Get_NumVoxel_Active_All_Layer();
	_uint Get_NumVoxel_In_Layer(const VOXEL_LAYER eLayer);
	_uint Get_NumVoxel_All_Layer();
	_uint Get_NumSector(const VOXEL_LAYER eLayer);
	_uint Get_NumSector_Active();

	_float Get_WorldSize_Voxel();

	HRESULT Get_VoxelID(const _float3 & vWorldPos, VOXEL_ID& eID, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelID(const _float3 & vWorldPos, VOXEL_ID eID, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelID(const _uint3 & vIndexPos, VOXEL_ID & eID, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelID(const _uint3 & vIndexPos, VOXEL_ID eID, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelID(const _uint iVoxelIndex, VOXEL_ID & eID, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelID(const _uint iVoxelIndex, VOXEL_ID eID, const VOXEL_LAYER eLayer);

	HRESULT Get_VoxelState(const _float3& vWorldPos, VOXEL_STATE& eState, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelState(const _float3& vWorldPos, VOXEL_STATE eState, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelState(const _uint3& vIndexPos, VOXEL_STATE& eState, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelState(const _uint3& vIndexPos, VOXEL_STATE eState, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelState(const _uint iVoxelIndex, VOXEL_STATE& eState, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelState(const _uint iVoxelIndex, VOXEL_STATE eState, const VOXEL_LAYER eLayer);

	HRESULT Get_VoxelOpenessScore(const _float3& vWorldPos, _byte& bOpenessScore, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelOpenessScore(const _uint3& vIndexPos, _byte& bOpenessScore, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelOpenessScore(const _uint iVoxelIndex, _byte& bOpenessScore, const VOXEL_LAYER eLayer);

	HRESULT Add_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Add_Voxel(const _uint3 & vIndexPos, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Add_Voxel(const _float3 & vWorldPos, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Erase_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer);
	HRESULT Erase_Voxel(const _uint3 & vIndexPos, const VOXEL_LAYER eLayer);
	HRESULT Erase_Voxel(const _float3 & vWorldPos, const VOXEL_LAYER eLayer);

	_bool Is_Exist_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer);
	_bool Is_Exist_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer);

	_uint Get_CurPicked_VoxelIndex(const VOXEL_LAYER eLayer);
	_uint Get_PrePicked_VoxelIndex();
	_float3 Get_CurPicked_VoxelWorldPos(const VOXEL_LAYER eLayer);
	_float3 Get_PrePicked_VoxelWorldPos();

	HRESULT Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _uint iVoxelIndex, _float3& vNearestPosFloat3);
	HRESULT Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _uint3& vVoxelIndexPos, _float3& vNearestPosFloat3);
	HRESULT Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _float3& vVoxelWorldPos, _float3& vNearestPosFloat3);

public:	/* For.Boid_Manager*/
	HRESULT Add_Boid(class CBoid* pBoid);
	HRESULT Erase_Boid(class CBoid* pBoid);

	HRESULT Get_Neighbor_Position_Boid(const _uint iNeighborIndex, _float3& vNeighborPos);
	HRESULT Get_Neighbor_Indices_Boid(class CBoid* pBoid, const _float fNeighborRange, vector<_uint>& NeighborIndices);
	HRESULT Get_Neighbor_FlowFieldIndex_Boid(const _uint iNeighborIndex, _uint& iFlowFieldIndexTag);

public:	/* For.Formation_Manager */
	HRESULT Add_FormationGroup(_uint& iGroupIndex);
	HRESULT Erase_FormationGroup(const _uint iGroupIndex);

	HRESULT Add_Formation(class CFormation* pFormation, const _uint iGroupIndex);
	HRESULT Erase_Formation(class CFormation* pFormation, const _uint iGroupIndex);

	HRESULT Initiate_Formation(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex);

	FORMATION_TYPE Get_Formation_Type();
	void Set_Formation_Type(const FORMATION_TYPE eType);

public:
	HRESULT Add_Render_Path(class CNavigation* pNavigation);
	HRESULT Add_Render_Path(const list<_uint>& PathIndices);
	HRESULT Set_FlowFieldRenderIndex(const _uint iIndexTag);


public: /* For.Picking */
	void Transform_PickingToLocalSpace_Perspective(const class CTransform* pTransform, _Out_ _float3 * pRayDir, _Out_ _float4 * pRayPos);
	void Transform_PickingToLocalSpace_Perspective(_fmatrix WorldMatrix, _Out_ _float3 * pRayDir, _Out_ _float4 * pRayPos);
	_float4 Get_MouseWorldPos();
	_float3 Get_MouseWorldDirection();

public: /* For.ImGui Manager*/
	IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HRESULT Insert_GuiObject(const wstring & strTag, class CGui_Object* pObject);
	HRESULT Erase_GuiObject(const wstring & strTag);
	HRESULT Clear_GuiObjects();

	HRESULT Ready_RTVDebug(const wstring & strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Draw_RTVDebug(const wstring & strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

	class CPathFinder_Voxel* Get_ToolsPathFinder();

public:
	HRESULT Ready_Fonts(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);

public:
	_uint Get_WinSizeX() { return m_WinSize.iSizeX; }
	_uint Get_WinSizeY() { return m_WinSize.iSizeY; }

private:
	class CGraphic_Device*			m_pGraphic_Device = { nullptr };
	class CInput_Device*			m_pInput_Device = { nullptr };
	class CLevel_Manager*			m_pLevel_Manager = { nullptr };
	class CObject_Manager*			m_pObject_Manager = { nullptr };
	class CComponent_Manager*		m_pComponent_Manager = { nullptr };
	class CRenderer*				m_pRenderer = { nullptr };
	class CTimer_Manager*			m_pTimer_Manager = { nullptr };
	class CPipeLine*				m_pPipeLine = { nullptr };
	class CLight_Manager*			m_pLight_Manager = { nullptr };
	class CFont_Manager*			m_pFont_Manager = { nullptr };
	class CTarget_Manager*			m_pTarget_Manager = { nullptr };
	class CFrustum*					m_pFrustum = { nullptr };
	class CExtractor*				m_pExtractor = { nullptr };
	class CVoxel_Manager*			m_pVoxel_Manager = { nullptr };
	class CPath_Manager*			m_pPath_Manager = { nullptr };
	class CBoid_Manager*			m_pBoid_Manager = { nullptr };
	class CFormation_Manager*		m_pFormation_Manager = { nullptr };
	class CPicking*					m_pPicking = { nullptr };
	class CImGUI_Manager*			m_pImGUI_Manager = { nullptr };

private:
	WIN_SIZE						m_WinSize = {};

public:
	static void Release_Engine();
	virtual void Free() override;
};

END
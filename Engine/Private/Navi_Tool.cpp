#include "Navi_Tool.h"
#include "GameInstance.h"
#include "PathFinder_Voxel.h"
#include "Navigation.h"
#include "Path_Manager.h"
#include "Boid.h"
#include "Formation.h"

CNavi_Tool::CNavi_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGui_Object{ pDevice, pContext }
{
}

HRESULT CNavi_Tool::Initialize(class CPath_Manager* pPath_Manager, void* pArg)
{
	if (nullptr == pPath_Manager ||
		nullptr == pArg)
		return E_FAIL;

	NAVI_TOOL_DESC*				pDesc = { static_cast<NAVI_TOOL_DESC*>(pArg) };
	m_pNavigations = pDesc->pNavigations;
	m_pTransforms = pDesc->pTransforms;
	m_pCurObjectIndex = pDesc->pCurObjectIndex;

	if (nullptr == m_pNavigations ||
		nullptr == m_pTransforms ||
		nullptr == m_pCurObjectIndex)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pPath_Manager = pPath_Manager;
	Safe_AddRef(m_pPath_Manager);

	return S_OK;
}

void CNavi_Tool::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	ImGui::Begin("Path Tool");

	if (ImGui::IsWindowFocused())
		*m_pFocus = true;
	if (ImGui::IsWindowHovered())
		*m_pHovered = true;

	//////////////////////////////////
	//////////////////////////////////
	//////////////////////////////////

	Choice_Formation_Type();
		
	//	Make_Formation_Initiate();
	Make_Formation_Initiate2();
	//	Make_Formation_Initiate_Flocking();
	//////////////////////////////////
	//////////////////////////////////
	//////////////////////////////////



	Set_GoalIndex_VectorField();
	Shuffle_All_Agents_FlowField();

	if (ImGui::Button("Clear GoalIndices "))
	{
		m_GoalIndices.clear();
	}

	ImGui::Text("Multi Agent Path Find");
	Make_Move_VectorField_Daijkstra();
	Make_Move_VectorField_A_Star();
	ImGui::Text("Single Agent Path Find");
	Make_Path_Move_A_Star();
	Clear_VectorField();

	Show_VectorFieldInfos();


	Pick_Voxel_Change_State();

	Choice_Mode();
	Choice_HeuristicWeights();
	Choice_Heuristic();
	Choice_Algorithm();
	Choice_Test_Mode_Optimizer();
	Choice_Can_Throw_Path();

	Set_AgentParams();
	Set_BoidParams();

	Make_Path();

	Show_CurObjectPathLists();

	if (ImGui::Button("Start Move Cur Agent ##Path Tool"))
	{
		CNavigation* pNavigation = { Get_CurObjectNavigation() };
		if (nullptr != pNavigation)
		{
			pNavigation->Start_Move();
		}
	}

	if (ImGui::Button("Start Move All Agent ##Path Tool"))
	{
		for (auto pComponnet : (*m_pNavigations))
		{
			if (nullptr == pComponnet)
				continue;

			CNavigation*		pNavigation = { static_cast<CNavigation*>(pComponnet) };
			pNavigation->Start_Move();
		}
	}

	if (ImGui::Button("Stop Move All Agent ##Path Tool"))
	{
		for (auto pComponnet : (*m_pNavigations))
		{
			if (nullptr == pComponnet)
				continue;

			CNavigation*		pNavigation = { static_cast<CNavigation*>(pComponnet) };
			pNavigation->Stop_Move();
		}
	}

	Show_Current_PathInfos();

	for (auto pComponent : (*m_pNavigations))
	{
		if (nullptr == pComponent)
			continue;

		CNavigation* pNavigation = { static_cast<CNavigation*>(pComponent) };
		pNavigation->Priority_Tick(fTimeDelta);
	}

	for (auto pComponent : (*m_pNavigations))
	{
		if (nullptr == pComponent)
			continue;

		CNavigation* pNavigation = { static_cast<CNavigation*>(pComponent) };
		pNavigation->Tick(fTimeDelta);
		m_pGameInstance->Add_Render_Path(pNavigation);
	}

	for (auto pComponent : (*m_pNavigations))
	{
		if (nullptr == pComponent)
			continue;

		CNavigation* pNavigation = { static_cast<CNavigation*>(pComponent) };
		pNavigation->Late_Tick(fTimeDelta);
	}

	CNavigation*	pNavigation = { static_cast<CNavigation*>(Get_CurObjectNavigation()) };
	_uint			iFlowFieldIndexTag;
	if (nullptr != pNavigation &&
		!(FAILED(pNavigation->Get_Cur_FlowFieldIndexTag(iFlowFieldIndexTag))))
	{
		m_pGameInstance->Set_FlowFieldRenderIndex(iFlowFieldIndexTag);
	}

	ImGui::End();
}

void CNavi_Tool::Shuffle_All_Agents_FlowField()
{
	unordered_set<_uint>		Used;
	if (ImGui::Button("Rand All Agent FlowField"))
	{
		srand(static_cast<_uint>(time(NULL)));

		for (_uint i = 0; i < m_pNavigations->size(); ++i)
		{
			if (nullptr == (*m_pNavigations)[i] ||
				nullptr == (*m_pTransforms)[i])
				continue;

			CNavigation*		pNavigation = { static_cast<CNavigation*>((*m_pNavigations)[i]) };
			CTransform*			pTransform = { static_cast<CTransform*>((*m_pTransforms)[i]) };


			const auto&			FlowField = m_pPath_Manager->Get_FlowField(0);

			if(true == FlowField.empty())
				continue;

			_uint		iRand = { static_cast<_uint>(rand()) % static_cast<_uint>(FlowField.size()) };
			if (Used.end() != Used.find(iRand))
				continue;

			auto iter{ FlowField.begin() };
			advance(iter, iRand); 

			Used.emplace(iRand);

			_uint		iVoxelIndex = { iter->first };
			_float3		vVoxelWorldPos;
			m_pGameInstance->Get_WorldPosition_Voxel(iVoxelIndex, vVoxelWorldPos);

			pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(XMLoadFloat3(&vVoxelWorldPos), 1.f));

			pNavigation->Stop_Move();
		}
	}
}

void CNavi_Tool::Shuffle_All_Agents_Floor()
{
	unordered_set<_uint>		Used;
	if (ImGui::Button("Rand All Agent Floor"))
	{
		srand(static_cast<_uint>(time(NULL)));

		for (_uint i = 0; i < m_pNavigations->size(); ++i)
		{
			if (nullptr == (*m_pNavigations)[i] ||
				nullptr == (*m_pTransforms)[i])
				continue;

			CNavigation*		pNavigation = { static_cast<CNavigation*>((*m_pNavigations)[i]) };
			CTransform*			pTransform = { static_cast<CTransform*>((*m_pTransforms)[i]) };

			_uint				iFlowFieldIndexTag;
			if (FAILED(pNavigation->Get_Cur_FlowFieldIndexTag(iFlowFieldIndexTag)))
				continue;

			const auto& FlowField = m_pPath_Manager->Get_FlowField(iFlowFieldIndexTag);

			if (true == FlowField.empty())
				continue;

			_uint		iRand = { static_cast<_uint>(rand()) % static_cast<_uint>(FlowField.size()) };
			if (Used.end() != Used.find(iRand))
				continue;

			auto iter{ FlowField.begin() };
			advance(iter, iRand);

			Used.emplace(iRand);

			_uint		iVoxelIndex = { iter->first };
			_float3		vVoxelWorldPos;
			m_pGameInstance->Get_WorldPosition_Voxel(iVoxelIndex, vVoxelWorldPos);

			pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(XMLoadFloat3(&vVoxelWorldPos), 1.f));
		}
	}
}

void CNavi_Tool::Choice_Mode()
{
	if (ImGui::CollapsingHeader("Select Mode ##PathTool"))
	{
		if (ImGui::RadioButton("Start ##PathTool", m_eMode == MODE::_START))
			m_eMode = MODE::_START;
		ImGui::SameLine();

		if (ImGui::RadioButton("Goal ##PathTool", m_eMode == MODE::_GOAL))
			m_eMode = MODE::_GOAL;
		ImGui::SameLine();
	}

	ImGui::NewLine();
}

void CNavi_Tool::Choice_HeuristicWeights()
{
	CNavigation* pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	if (ImGui::CollapsingHeader("Setup HeuristicWeights ##PathTool"))
	{
		_float			Weights[static_cast<_uint>(HEURISTIC_TYPE::_END)] = {};

		for (_uint i = 0; i < static_cast<_uint>(HEURISTIC_TYPE::_END); ++i)
		{
			Weights[i] = pNavigation->Get_HeuristicWeights(static_cast<HEURISTIC_TYPE>(i));
			if (static_cast<HEURISTIC_TYPE>(i) == HEURISTIC_TYPE::_OPENNESS)
				ImGui::SliderFloat("Openess Weight", &Weights[i], 0.f, 10.f);
			else if(static_cast<HEURISTIC_TYPE>(i) == HEURISTIC_TYPE::_DISTANCE)
				ImGui::SliderFloat("Distance Weight", &Weights[i], 0.f, 10.f);
			else if (static_cast<HEURISTIC_TYPE>(i) == HEURISTIC_TYPE::_HEIGHT)
				ImGui::SliderFloat("Height Weight", &Weights[i], 0.f, 10.f);


			pNavigation->Set_HeuristicWeights(static_cast<HEURISTIC_TYPE>(i), Weights[i]);
		}
	}

	ImGui::NewLine();
}

void CNavi_Tool::Choice_Algorithm()
{
	CNavigation*		pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	if (ImGui::CollapsingHeader("Algorithm ##PathTool"))
	{
		ALGORITHM			eAlgorithm = { pNavigation->Get_Algorithm() };
		if (ImGui::RadioButton("A* ##PathTool", eAlgorithm == ALGORITHM::_A_STAR))
		{
			eAlgorithm = ALGORITHM::_A_STAR;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("GBFS ##PathTool", eAlgorithm == ALGORITHM::_GBFS))
		{
			eAlgorithm = ALGORITHM::_GBFS;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("Daijkstra ##PathTool", eAlgorithm == ALGORITHM::_DAIJKSTRA))
		{
			eAlgorithm = ALGORITHM::_DAIJKSTRA;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("BFS ##PathTool", eAlgorithm == ALGORITHM::_BFS))
		{
			eAlgorithm = ALGORITHM::_BFS;
		}
		ImGui::SameLine();


		pNavigation->Set_Algorithm(eAlgorithm);

	}

	ImGui::NewLine();
}

void CNavi_Tool::Choice_Heuristic()
{
	CNavigation*		pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	if (ImGui::CollapsingHeader("Heuristic ##PathTool"))
	{
		MAIN_HEURISTIC		eHeuristic = { pNavigation->Get_MainHeuristic() };
		if (ImGui::RadioButton("Euclidean ##PathTool", eHeuristic == MAIN_HEURISTIC::_EUCLIDEAN))
		{
			eHeuristic = MAIN_HEURISTIC::_EUCLIDEAN;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("Manhattan ##PathTool", eHeuristic == MAIN_HEURISTIC::_MANHATTAN))
		{
			eHeuristic = MAIN_HEURISTIC::_MANHATTAN;
		}
		ImGui::SameLine();
		pNavigation->Set_MainHeuristic(eHeuristic);
	}

	ImGui::NewLine();
}

void CNavi_Tool::Choice_Can_Throw_Path()
{
}

void CNavi_Tool::Choice_Test_Mode_Optimizer()
{
	CNavigation*		pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	if (ImGui::CollapsingHeader("Optimizer Test Mode ##PathTool"))
	{
		TEST_MODE		eTestMode = { pNavigation->Get_OptimizerTestMode() };
		if (ImGui::RadioButton("Default ##PathToolasd123asdfzxcv", eTestMode == TEST_MODE::_DEFAULT))
		{
			eTestMode = TEST_MODE::_DEFAULT;
		}
		ImGui::SameLine();

		if (ImGui::RadioButton("Inifinite ##PathToolasd123asdfzxcv", eTestMode == TEST_MODE::_INFITINTE))
		{
			eTestMode = TEST_MODE::_INFITINTE;
		}
		ImGui::SameLine();
		pNavigation->Set_OptimizerTestMode(eTestMode);
	}

	ImGui::NewLine();
}

void CNavi_Tool::Move_AllAgent_Make_Flow_Field()
{

}

void CNavi_Tool::Set_AgentParams()
{
	_float			fReserveSensortive = { CNavigation::Get_ReserveSensortive() };
	if (ImGui::InputFloat("Reserve Sensortive ##1232asdeasdasdasdczxc", &fReserveSensortive))
	{
		CNavigation::Set_ReserveSensortive(fReserveSensortive);
	}

	CNavigation*		pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	if (ImGui::CollapsingHeader("Agent Params ##PathTool"))
	{
		_bool				isPatrol = { pNavigation->Is_Patrol() };
		if (ImGui::RadioButton("Patrol ##1232asdeasdasdasdczxc", isPatrol))
		{
			isPatrol = !isPatrol;
			pNavigation->Set_Patrol(isPatrol);
		}


		_bool				isMove = { pNavigation->Is_Move() };
		if (ImGui::RadioButton("Move ##1232asdeasdasdasdczxc", isMove))
		{
			if (true == isMove)
			{
				isMove = !isMove;
				if (false == isMove)
					pNavigation->Stop_Move();
				else
					pNavigation->Start_Move();
			}
		}
		
		_float				fSpeed = { pNavigation->Get_Speed() };
		if (ImGui::InputFloat("Speed ##1232asdeasdasdasdczxc", &fSpeed))
		{
			pNavigation->Set_Speed(fSpeed);
		}

		_float				fRadius = { pNavigation->Get_Radius() };
		if (ImGui::InputFloat("Radius ##1232asdeasdasdasdczxc", &fRadius))
		{
			pNavigation->Set_Radius(fRadius);
		}
		_float				fHeight = { pNavigation->Get_Height() };
		if (ImGui::InputFloat("Height ##1232asdeasdasdasdczxc", &fHeight))
		{
			pNavigation->Set_Height(fHeight);
		}
		_float				fMaxWaitTime = { pNavigation->Get_MaxWaitTime() };
		if (ImGui::InputFloat("Max Wait Time ##1232asdeasdasdasdczxc", &fMaxWaitTime))
		{
			pNavigation->Set_MaxWaitTime(fMaxWaitTime);
		}

		_float				fRecognizeDist = { pNavigation->Get_ObstacleRecognizeDist() };
		if (ImGui::InputFloat("Obstacle Recognize Dist ##1232asdeasdasdasdczxc", &fRecognizeDist))
		{
			pNavigation->Set_ObstacleRecognizeDist(fRecognizeDist);
		}
	}

	ImGui::NewLine();
}

void CNavi_Tool::Set_BoidParams()
{
	if (ImGui::CollapsingHeader("Boid Params ##PathTool"))
	{
		_float				fFlowfieldRatio = { CBoid::Get_FlowField_Ratio() };
		if (ImGui::SliderFloat("FlowfieldRatio ##1232asdeasdasdasdczxc", &fFlowfieldRatio, 0.f, 10.f))
		{
			CBoid::Set_FlowField_Ratio(fFlowfieldRatio);
		}

		_float				fAlignmentRatio = { CBoid::Get_Alignment_Ratio() };
		if (ImGui::SliderFloat("AlignmentRatio ##1232asdeasdasdasdczxc", &fAlignmentRatio, 0.f, 10.f))
		{
			CBoid::Set_Alignment_Ratio(fAlignmentRatio);
		}

		_float				fCohesionRatio = { CBoid::Get_Cohesion_Ratio() };
		if (ImGui::SliderFloat("CohesionRatio ##1232asdeasdasdasdczxc", &fCohesionRatio, 0.f, 10.f))
		{
			CBoid::Set_Cohesion_Ratio(fCohesionRatio);
		}

		_float				fSeparationRatio = { CBoid::Get_Separation_Ratio() };
		if (ImGui::SliderFloat("SeparationRatio ##1232asdeasdasdasdczxc", &fSeparationRatio, 0.f, 10.f))
		{
			CBoid::Set_Separation_Ratio(fSeparationRatio);
		}

		_float				fAvoidenceRatio = { CBoid::Get_Avoidence_Ratio() };
		if (ImGui::SliderFloat("AvoidenceRatio ##1232asdeasdasdasdczxc", &fAvoidenceRatio, 0.f, 10.f))
		{
			CBoid::Set_Avoidence_Ratio(fAvoidenceRatio);
		}
	}

	ImGui::NewLine();
}

void CNavi_Tool::Set_GoalIndex_VectorField()
{
	_uint			iCurPickedVoxelIndex = { m_pGameInstance->Get_CurPicked_VoxelIndex(VOXEL_LAYER::_STATIC) };

	if (m_pGameInstance->Get_KeyState('Y') == DOWN)
	{
		m_GoalIndices.push_back(iCurPickedVoxelIndex);
	}
}

void CNavi_Tool::Choice_Formation_Type()
{
	if (ImGui::CollapsingHeader("Formation Type ##PathTool"))
	{
		FORMATION_TYPE			eType = { m_pGameInstance->Get_Formation_Type() };
		if (ImGui::RadioButton("2_Line ##asd", eType == FORMATION_TYPE::_TWO_LINE))
			m_pGameInstance->Set_Formation_Type(FORMATION_TYPE::_TWO_LINE);

		ImGui::SameLine();

		if (ImGui::RadioButton("4_Line ##asd", eType == FORMATION_TYPE::_FOUR_LINE))
			m_pGameInstance->Set_Formation_Type(FORMATION_TYPE::_FOUR_LINE);

		ImGui::SameLine();

		if (ImGui::RadioButton("8_Line ##asd", eType == FORMATION_TYPE::_EIGHT_LINE))
			m_pGameInstance->Set_Formation_Type(FORMATION_TYPE::_EIGHT_LINE);

		ImGui::SameLine();

		if (ImGui::RadioButton("Circle ##asd", eType == FORMATION_TYPE::_CIRCLE))
			m_pGameInstance->Set_Formation_Type(FORMATION_TYPE::_CIRCLE);
	}

	ImGui::NewLine();
}

void CNavi_Tool::Make_Formation_Initiate()
{
	if (ImGui::Button("Make Formation & Initiate ##DFSDFDVXCvxcvzxcvzxcvzxcv"))
	{
		if (2 > m_GoalIndices.size())
			return;

		const _uint		iStartIndex = { m_GoalIndices[0] };
		const _uint		iGoalIndex = { m_GoalIndices[1] };

		_uint			iGroupIndex;
		m_pGameInstance->Add_FormationGroup(iGroupIndex);

		_uint			iNumAddFormation{};
		vector<CNavigation*>		AddedNavigations;
		for (auto pComponent : (*m_pNavigations))
		{
			if (nullptr == pComponent)
				continue;

			CNavigation* pNavigation = { static_cast<CNavigation*>(pComponent) };
			pNavigation->Add_FormationGroup(iGroupIndex);

			AddedNavigations.push_back(pNavigation);
			++iNumAddFormation;
			if (32 <= iNumAddFormation)
				break;
		}

		auto start = std::chrono::high_resolution_clock::now();


		if (FAILED(m_pGameInstance->Initiate_Formation(iGroupIndex, iStartIndex, iGoalIndex)))
			return;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		cout << "Make ParentMap Time " << duration.count() << endl;

		for (auto pNavigation : AddedNavigations)
		{
			if (FAILED(pNavigation->SetUp_Formation_Path()))
				return;

			if(false == pNavigation->Is_Move())
				pNavigation->Start_Move();
		}

		m_GoalIndices.clear();
	}
}

void CNavi_Tool::Make_Formation_Initiate2()
{
	if (ImGui::Button("Make Formation & Initiate22 ##DFSDFDVXCvxcvzxcvzxcvzxcv"))
	{
		if (2 > m_GoalIndices.size())
			return;

		const _uint		iStartIndex = { m_GoalIndices[0] };
		const _uint		iGoalIndex = { m_GoalIndices[1] };

		_uint			iGroupIndex;
		m_pGameInstance->Add_FormationGroup(iGroupIndex);

		_uint			iNumAddFormation{};
		vector<CNavigation*>		AddedNavigations;
		for (auto pComponent : (*m_pNavigations))
		{
			if (nullptr == pComponent)
				continue;

			CNavigation* pNavigation = { static_cast<CNavigation*>(pComponent) };
			pNavigation->Add_FormationGroup(iGroupIndex);

			AddedNavigations.push_back(pNavigation);
			++iNumAddFormation;
			if (32 <= iNumAddFormation)
				break;
		}

		auto start = std::chrono::high_resolution_clock::now();


		if (FAILED(m_pGameInstance->Initiate_Formation(iGroupIndex, iStartIndex, iGoalIndex)))
			return;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		cout << "Make ParentMap Time " << duration.count() << endl;

		for (auto pNavigation : AddedNavigations)
		{
			if (FAILED(pNavigation->SetUp_Formation_PathTest(iGroupIndex)))
				return;

			if(false == pNavigation->Is_Move())
				pNavigation->Start_Move();
		}

		m_GoalIndices.clear();
	}
}

void CNavi_Tool::Make_Formation_Initiate_Flocking()
{
	if (ImGui::Button("Make Formation & Initiate _ Flocking ##DFSDFDVXCvxcvzxcvzxcvzxcv"))
	{
		if (2 > m_GoalIndices.size())
			return;

		const _uint		iStartIndex = { m_GoalIndices[0] };
		const _uint		iGoalIndex = { m_GoalIndices[1] };

		_uint			iGroupIndex;
		m_pGameInstance->Add_FormationGroup(iGroupIndex);

		_uint			iNumAddFormation{};
		vector<CNavigation*>		AddedNavigations;
		for (auto pComponent : (*m_pNavigations))
		{
			if (nullptr == pComponent)
				continue;

			CNavigation* pNavigation = { static_cast<CNavigation*>(pComponent) };
			pNavigation->Add_FormationGroup(iGroupIndex);

			AddedNavigations.push_back(pNavigation);
			++iNumAddFormation;
			if (16 <= iNumAddFormation)
				break;
		}

		if (FAILED(m_pGameInstance->Initiate_Formation(iGroupIndex, iStartIndex, iGoalIndex)))
			return;

		for (auto pNavigation : AddedNavigations)
		{
			if (false == pNavigation->Is_Move())
				pNavigation->Start_Move();
		}

		if (true == m_GoalIndices.empty())
			return;

		vector<_uint>					StartIndices;
		unordered_map<_uint, _uint>		ParentMap;

		_uint							iNumAgent = {};
		_uint							iIndexTag{};

		for (auto pComponent : (*m_pNavigations))
		{
			if (nullptr == pComponent)
				continue;

			CNavigation* pNavigation = { static_cast<CNavigation*>(pComponent) };
			_uint				iStartIndex;
			if (FAILED(pNavigation->Get_LastGoalIndex(iStartIndex)) &&
				FAILED(pNavigation->Get_CurPos_Index(iStartIndex)))
				continue;

			StartIndices.push_back(iStartIndex);
			++iNumAgent;
		}

		auto start = std::chrono::high_resolution_clock::now();

		auto iter = m_GoalIndices.begin();
		++iter;

		if (FAILED(m_pGameInstance->Make_VectorFiled_Daijkstra({*iter}, iNumAgent, iIndexTag)))
		{
			MSG_BOX(TEXT("Failed To Make VectorField :: Call NaviTool"));
			return;
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		cout << "Make ParentMap Time " << duration.count() << endl;


		for (auto pComponent : (*m_pNavigations))
		{
			if (nullptr == pComponent)
				continue;

			CNavigation* pNavigation = { static_cast<CNavigation*>(pComponent) };
			_uint				iStartIndex;
			if (FAILED(pNavigation->Get_LastGoalIndex(iStartIndex)) &&
				FAILED(pNavigation->Get_CurPos_Index(iStartIndex)))
				continue;

			list<_uint>			BestList;
			m_pGameInstance->Convert_BestList_From_FlowField(BestList, iIndexTag, iStartIndex);

			COMMAND_INFO_MAPF			CommandInfo;
			CommandInfo.BestPathIndices = BestList;
			CommandInfo.ePathMode = PATH_MODE::_MULTI_AGENT;
			CommandInfo.iStartIndex = iStartIndex;
			CommandInfo.iGoalIndex = BestList.back();
			CommandInfo.iFlowIndex = iIndexTag;


			pNavigation->Add_CommandInfo(CommandInfo);
			if (false == pNavigation->Is_Move())
				pNavigation->Start_Move();
		}
		m_GoalIndices.clear();
	}
}

void CNavi_Tool::Make_Move_VectorField_Daijkstra()
{
	if (ImGui::Button("Make Vector Field Daijkstra ##DFSDFDVXCvxcvzxcvzxcvzxcv"))
	{
		if (true == m_GoalIndices.empty())
			return;

		vector<_uint>					StartIndices;
		unordered_map<_uint, _uint>		ParentMap;

		_uint							iNumAgent = {};
		_uint							iIndexTag{};

		for (auto pComponent : (*m_pNavigations))
		{
			if (nullptr == pComponent)
				continue;

			CNavigation*		pNavigation = { static_cast<CNavigation*>(pComponent) };
			_uint				iStartIndex;
			if (FAILED(pNavigation->Get_LastGoalIndex(iStartIndex)) &&
				FAILED(pNavigation->Get_CurPos_Index(iStartIndex)))
				continue;

			StartIndices.push_back(iStartIndex);
			++iNumAgent;
		}

		auto start = std::chrono::high_resolution_clock::now();

		
		if (FAILED(m_pGameInstance->Make_VectorFiled_Daijkstra(m_GoalIndices, iNumAgent, iIndexTag)))
		{
			MSG_BOX(TEXT("Failed To Make VectorField :: Call NaviTool"));
			return;
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		
		cout << "Make ParentMap Time " << duration.count() << endl;


		for (auto pComponent : (*m_pNavigations))
		{
			if (nullptr == pComponent)
				continue;

			CNavigation*		pNavigation = { static_cast<CNavigation*>(pComponent) };
			_uint				iStartIndex;
			if (FAILED(pNavigation->Get_LastGoalIndex(iStartIndex)) &&
				FAILED(pNavigation->Get_CurPos_Index(iStartIndex)))
				continue;

			list<_uint>			BestList;
			m_pGameInstance->Convert_BestList_From_FlowField(BestList, iIndexTag, iStartIndex);

			COMMAND_INFO_MAPF			CommandInfo;
			CommandInfo.BestPathIndices = BestList;
			CommandInfo.ePathMode = PATH_MODE::_MULTI_AGENT;
			CommandInfo.iStartIndex = iStartIndex;
			CommandInfo.iGoalIndex = BestList.back();
			CommandInfo.iFlowIndex = iIndexTag;


			pNavigation->Add_CommandInfo(CommandInfo);
			if (false == pNavigation->Is_Move())
				pNavigation->Start_Move();
		}
		m_GoalIndices.clear();
	}
}

void CNavi_Tool::Make_Move_VectorField_A_Star()
{
	if (ImGui::Button("Make Vector Field A_Star ##DFSDFDVXCvxcvzxcvzxcvzxcv"))
	{
		unordered_set<_uint>			StartIndicesSet;
		vector<_uint>					StartIndices;
		unordered_map<_uint, _uint>		ParentMap;

		_uint							iNumAgent = {};
		_uint							iIndexTag{};

		if (true == m_GoalIndices.empty())
			return;

		for (_uint i = 0; i < m_pNavigations->size(); ++i)
		{
			if (nullptr == (*m_pNavigations)[i] ||
				nullptr == (*m_pTransforms)[i])
				continue;

			CNavigation*		pNavigation = { static_cast<CNavigation*>((*m_pNavigations)[i]) };
			CTransform*			pTransform = { static_cast<CTransform*>((*m_pTransforms)[i]) };

			_vector				vPos = { pTransform->Get_State_Vector(CTransform::STATE_POSITION) };
			_float3				vPosFloat3;

			XMStoreFloat3(&vPosFloat3, vPos);

			_uint			iStartIndex{};

			if (FAILED(pNavigation->Get_LastGoalIndex(iStartIndex)) &&
				FAILED(pNavigation->Get_CurPos_Index(iStartIndex)))
				continue;

			StartIndicesSet.insert(iStartIndex);
			++iNumAgent;
		}

		for (auto iStartIndex : StartIndicesSet)
			StartIndices.push_back(iStartIndex);

		auto start = std::chrono::high_resolution_clock::now();


		if (FAILED(m_pGameInstance->Make_VectorFiled_A_Star(m_GoalIndices.front(), StartIndices, iNumAgent, iIndexTag)))
		{
			MSG_BOX(TEXT("Failed To Make VectorField :: Call NaviTool"));
			return;
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		cout << "Make ParentMap Time " << duration.count() << endl;


		for (auto pComponent : (*m_pNavigations))
		{
			if (nullptr == pComponent)
				continue;

			CNavigation*		pNavigation = { static_cast<CNavigation*>(pComponent) };
			_uint				iStartIndex;
			if (FAILED(pNavigation->Get_LastGoalIndex(iStartIndex)) &&
				FAILED(pNavigation->Get_CurPos_Index(iStartIndex)))
				continue;

			list<_uint>			BestList;
			m_pGameInstance->Convert_BestList_From_FlowField(BestList, iIndexTag, iStartIndex);

			if (BestList.empty())
				int iA = 0;

			COMMAND_INFO_MAPF			CommandInfo;
			CommandInfo.BestPathIndices = BestList; 
			CommandInfo.ePathMode = PATH_MODE::_MULTI_AGENT;
			CommandInfo.iStartIndex = iStartIndex;
			CommandInfo.iGoalIndex = BestList.back();
			CommandInfo.iFlowIndex = iIndexTag;

			pNavigation->Add_CommandInfo(CommandInfo);
			if (false == pNavigation->Is_Move())
				pNavigation->Start_Move();
		}

		m_GoalIndices.clear();
	}
}

void CNavi_Tool::Make_Path_Move_A_Star()
{
	if (ImGui::Button("Make Path A_Star ##DFSDFDVXCvxcvzxcvzxcvzxcv"))
	{
		if (true == m_GoalIndices.empty())
			return;

		CNavigation*		pNavigation = { Get_CurObjectNavigation() };
		if (nullptr == pNavigation)
			return;

		_uint				iStartIndex{};
		if (FAILED(pNavigation->Get_LastGoalIndex(iStartIndex)) &&
			FAILED(pNavigation->Get_CurPos_Index(iStartIndex)))
			return;

		auto start = std::chrono::high_resolution_clock::now();

		NODE_INFOS			NodeInfos;
		list<_uint>			BestList;

		if (FAILED(m_pGameInstance->Make_Path_Voxel(BestList, NodeInfos, iStartIndex, { m_GoalIndices.front() }, false, false, {})))
		{
			MSG_BOX(TEXT("Failed To Make VectorField :: Call NaviTool"));
			return;
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		cout << "Make ParentMap Time " << duration.count() << endl;

		COMMAND_INFO_SAPF			CommandInfo;
		CommandInfo.BestPathIndices = BestList;
		CommandInfo.ePathMode = PATH_MODE::_SINGLE_AGENT;
		CommandInfo.iStartIndex = iStartIndex;
		CommandInfo.iGoalIndex = BestList.back();

		pNavigation->Add_CommandInfo(CommandInfo);
		if (false == pNavigation->Is_Move())
			pNavigation->Start_Move();

		m_GoalIndices.clear();
	}
}

void CNavi_Tool::Clear_VectorField()
{
	if (ImGui::Button("Clear All VectorField"))
	{
		for (auto pComponent : (*m_pNavigations))
		{
			if (nullptr == pComponent)
				continue;

			CNavigation* pNavigation = { static_cast<CNavigation*>(pComponent) };
			pNavigation->Clear_CommandQueue();
		}
	}
}

void CNavi_Tool::Show_VectorFieldInfos()
{
	if (ImGui::CollapsingHeader("FlowField Infos ##dkfujzxoipcvujiosdkfas"))
	{
		vector<_uint>			FlowFieldIndices = { move(m_pGameInstance->Get_Active_FlowField_Indices()) };
		vector<_uint>			FlowFieldRefCnts = { move(m_pGameInstance->Get_Active_FlowField_RefCnt()) };

		for (_uint i = 0; i < static_cast<_uint>(FlowFieldIndices.size()); ++i)
		{
			ImGui::Text("Index : %d", FlowFieldIndices[i]);
			ImGui::Text("RefCnt : %d", FlowFieldRefCnts[i]);
		}
	}
}

void CNavi_Tool::Show_CurObjectPathLists()
{
	CNavigation*		pNavigation = { Get_CurObjectNavigation() };

	ImGui::Text("Show Path List Option");

	if (ImGui::RadioButton("Open List ##1232asdeasda123124sdasdczxc", m_isShowOpenList))
		m_isShowOpenList = !m_isShowOpenList;

	ImGui::SameLine();

	if (ImGui::RadioButton("Close List ##1232asdeasda123124sdasdczxc", m_isShowCloseList))
		m_isShowCloseList = !m_isShowCloseList;

	ImGui::SameLine();

	if (ImGui::RadioButton("Best List ##1232asdeasda123124sdasdczxc", m_isShowBestList))
		m_isShowBestList = !m_isShowBestList;

	ImGui::NewLine();

	Show_OpenList(pNavigation);
	Show_CloseList(pNavigation);
	Show_BestList(pNavigation);
}

void CNavi_Tool::Show_OpenList(class CNavigation* pNavigaion)
{
	for (auto iVoxelIndex : m_PreOpenList)
		m_pGameInstance->Set_VoxelState(iVoxelIndex, VOXEL_STATE::_DEFAULT, VOXEL_LAYER::_STATIC);
	m_PreOpenList.clear();

	if (nullptr == pNavigaion)
		return;

	auto		CurOpenList{ pNavigaion->Get_OpenList() };

	for (auto iVoxelIndex : CurOpenList)
		m_PreOpenList.push_back(iVoxelIndex);

	if (true == m_isShowOpenList)
	{
		for (auto iVoxelIndex : m_PreOpenList)
			m_pGameInstance->Set_VoxelState(iVoxelIndex, VOXEL_STATE::_IS_OPEN, VOXEL_LAYER::_STATIC);
	}
}

void CNavi_Tool::Show_CloseList(class CNavigation* pNavigaion)
{
	for (auto iVoxelIndex : m_PreCloseList)
		m_pGameInstance->Set_VoxelState(iVoxelIndex, VOXEL_STATE::_DEFAULT, VOXEL_LAYER::_STATIC);
	m_PreCloseList.clear();

	if (nullptr == pNavigaion)
		return;

	auto		CurCloseList{ pNavigaion->Get_CloseList() };

	for (auto iVoxelIndex : CurCloseList)
		m_PreCloseList.push_back(iVoxelIndex);

	if (true == m_isShowCloseList)
	{
		for (auto iVoxelIndex : m_PreCloseList)
			m_pGameInstance->Set_VoxelState(iVoxelIndex, VOXEL_STATE::_IS_CLOSE, VOXEL_LAYER::_STATIC);
	}
}

void CNavi_Tool::Show_BestList(class CNavigation* pNavigaion)
{
	for (auto iVoxelIndex : m_PreBestList)
		m_pGameInstance->Set_VoxelState(iVoxelIndex, VOXEL_STATE::_DEFAULT, VOXEL_LAYER::_STATIC);
	m_PreBestList.clear();

	if (nullptr == pNavigaion)
		return;

	auto		CurBestList{ pNavigaion->Get_BestPath_Indices() };

	for (auto iVoxelIndex : CurBestList)
		m_PreBestList.push_back(iVoxelIndex);

	if (true == m_isShowBestList)
	{
		for (auto iVoxelIndex : m_PreBestList)
			m_pGameInstance->Set_VoxelState(iVoxelIndex, VOXEL_STATE::_IS_BEST, VOXEL_LAYER::_STATIC);
	}
}

CNavigation* CNavi_Tool::Get_CurObjectNavigation()
{
	_int		iCurObjectIndex = { *m_pCurObjectIndex };
	if (0 <= iCurObjectIndex &&
		static_cast<_int>(m_pNavigations->size()) > iCurObjectIndex)
		return static_cast<CNavigation*>((*m_pNavigations)[static_cast<_uint>(iCurObjectIndex)]);

	return nullptr;
}

void CNavi_Tool::Show_Current_PathInfos()
{
	ImGui::Text("================= Path Infos ======================");

	Show_Current_OpenListInfo();
	Show_Current_CloseListInfo();
	Show_Current_BestListInfo();
	Show_Current_OptimizedListInfo();
	Show_TotalDistance();
	Show_TotalTime();
}

void CNavi_Tool::Show_Current_OpenListInfo()
{
	CNavigation* pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	ImGui::Text("Open List Size: %d ea", pNavigation->Get_OpenList().size());
}

void CNavi_Tool::Show_Current_CloseListInfo()
{
	CNavigation* pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	ImGui::Text("Close List Size: %d ea", pNavigation->Get_CloseList().size());
}

void CNavi_Tool::Show_Current_BestListInfo()
{
	CNavigation* pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	ImGui::Text("Best List Size: %d ea", pNavigation->Get_BestPath_Indices().size());
}

void CNavi_Tool::Show_Current_OptimizedListInfo()
{
	CNavigation* pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	ImGui::Text("Optimized List Size: %d ea", pNavigation->Get_BestPath_Indices_Optimized().size());
}

void CNavi_Tool::Show_TotalDistance()
{
	CNavigation*		pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	ImGui::Text("BestList Total Dist: %f ea", pNavigation->Compute_Total_Dist_BestPath());
	ImGui::Text("Optimized Total Dist: %f ea", pNavigation->Compute_Total_Dist_OptimizedPath());
}

void CNavi_Tool::Show_TotalTime()
{
	CNavigation*		pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	ImGui::Text("Make Path Time: %d ms", pNavigation->Get_Make_Path_Time());
	ImGui::Text("Optimize Path Time: %d ms", pNavigation->Get_Optimize_Path_Time());
	ImGui::Text("Smooth Path Time: %d ms", pNavigation->Get_Smooth_Path_Time());
}

void CNavi_Tool::Make_Path()
{
	CNavigation*		pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	if (ImGui::Button("Make Path ##CNavi_Tool::Make_Path()"))
	{
		pNavigation->Set_MakeNewPass();
	}
}

void CNavi_Tool::Pick_Voxel_Change_State()
{
	CNavigation*		pNavigation = { Get_CurObjectNavigation() };
	if (nullptr == pNavigation)
		return;

	//if (DOWN == m_pGameInstance->Get_KeyState(VK_RBUTTON))
	//{
	//	_uint		iPickedVoxelIndex = { m_pGameInstance->Get_CurPicked_VoxelIndex(VOXEL_LAYER::_STATIC) };

	//	if (MODE::_START == m_eMode)
	//	{
	//		_uint			iPreGoalIndex;
	//		if (!FAILED(pNavigation->Get_StartIndex(iPreGoalIndex)))
	//		{
	//			//	m_pGameInstance->Set_VoxelState(iPreGoalIndex, VOXEL_STATE::_DEFAULT, VOXEL_LAYER::_STATIC);
	//		}

	//		//	pNavigation->Set_StartIndex(iPickedVoxelIndex);
	//		//	m_pGameInstance->Set_VoxelState(iPickedVoxelIndex, VOXEL_STATE::_START, VOXEL_LAYER::_STATIC);
	//	}
	//	else if (MODE::_GOAL == m_eMode)
	//	{
	//		_uint			iPreGoalIndex;
	//		if (!FAILED(pNavigation->Get_GoalIndex(iPreGoalIndex)))
	//		{
	//			//	m_pGameInstance->Set_VoxelState(iPreGoalIndex, VOXEL_STATE::_DEFAULT, VOXEL_LAYER::_STATIC);
	//		}

	//		//	pNavigation->Set_GoalIndex(iPickedVoxelIndex);
	//		//	m_pGameInstance->Set_VoxelState(iPickedVoxelIndex, VOXEL_STATE::_GOAL, VOXEL_LAYER::_STATIC);
	//	}
	//}
}

HRESULT CNavi_Tool::Add_Components()
{
	return S_OK;
}

CNavi_Tool* CNavi_Tool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, class CPath_Manager* pPath_Manager, void* pArg)
{
	CNavi_Tool*			pInstance = { new CNavi_Tool(pDevice, pContext) };
	if (FAILED(pInstance->Initialize(pPath_Manager, pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CNavi_Tool"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNavi_Tool::Free()
{
	__super::Free();

	Safe_Release(m_pPath_Manager);
}

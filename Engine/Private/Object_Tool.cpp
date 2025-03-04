#include "Object_Tool.h"
#include "GameInstance.h"
#include "Dummy_Object.h"

#include "Navigation.h"
#include "Obstacle.h"
#include "Obstacle_Cube.h"
#include "Obstacle_Cylinder.h"
#include "Obstacle_Mesh.h"

#include "Model.h"

#include "ImGuizmo.h"

CObject_Tool::CObject_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGui_Object{ pDevice, pContext }
{
}

HRESULT CObject_Tool::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    OBJECT_TOOL_DESC*       pDesc = { static_cast<OBJECT_TOOL_DESC*>(pArg) };
    m_pComponents = pDesc->pComponents;
    m_pCurObjectIdx = pDesc->pCurObjectIndex;
    m_pObjects = pDesc->pDummyObjects;

    if (nullptr == m_pComponents ||
        nullptr == m_pCurObjectIdx ||
        nullptr == m_pObjects)
        return E_FAIL;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Add_Components()))
        return E_FAIL;

    return S_OK;
}

void CObject_Tool::Tick(_float fTimeDelta)
{
    __super::Tick(fTimeDelta);

    ImGui::Begin("Object Tool");

    if (ImGui::IsWindowFocused())
        *m_pFocus = true;
    if (ImGui::IsWindowHovered())
        *m_pHovered = true;

    enum MODEL_TYPE { _AGENT, _OBSTACLE, _END };
    static MODEL_TYPE         eType = { _AGENT };

    if (ImGui::RadioButton("Prototype : Agent", eType == _AGENT))
        eType = _AGENT;

	if (ImGui::RadioButton("Prototype : _OBSTACLE", eType == _OBSTACLE))
		eType = _OBSTACLE;

    if (ImGui::Button("Create Object ## void CObject_Tool::Tick(_float fTimeDelta)"))
    {
        Create_Object();

		Add_Component_To_Object(m_pObjects->size() - 1, COMPONENT_TYPE::_MODEL_STATIC);

		if (eType == _AGENT)
			Add_Component_To_Object(m_pObjects->size() - 1, COMPONENT_TYPE::_NAVIGATION);
		else if (eType == _OBSTACLE)
			Add_Component_To_Object(m_pObjects->size() - 1, COMPONENT_TYPE::_OBSTACLE);

		static_cast<CTransform*>((*m_pComponents)[static_cast<_uint>(COMPONENT_TYPE::_TRANSFORM)].back())->Set_Scaled(0.3f, 0.3f, 0.3f);
    }

	if (ImGui::Button("Create Object 100 Agent## void CObject_Tool::Tick(_float fTimeDelta)"))
	{
        for (_uint i = 0; i < 100; ++i)
        {
		    Create_Object();
			Add_Component_To_Object(m_pObjects->size() - 1, COMPONENT_TYPE::_MODEL_STATIC);

            if (eType == _AGENT)
                Add_Component_To_Object(m_pObjects->size() - 1, COMPONENT_TYPE::_NAVIGATION);
            else if (eType == _OBSTACLE)
                Add_Component_To_Object(m_pObjects->size() - 1, COMPONENT_TYPE::_OBSTACLE);

            static_cast<CTransform*>((*m_pComponents)[static_cast<_uint>(COMPONENT_TYPE::_TRANSFORM)].back())->Set_Scaled(0.3f, 0.3f, 0.3f);
        }
	}

    if (ImGui::Button("Erase Object ## void CObject_Tool::Tick(_float fTimeDelta)"))
    {
        Erase_Objest(*m_pCurObjectIdx);
    }

	Select_ObjectIndex();
    Select_ComponentType();
    Select_ObstacleType();

	if (ImGui::Button("Add_Componet ## void CObject_Tool::Tick(_float fTimeDelta)"))
	{
        Add_Component_To_Object(*m_pCurObjectIdx, m_eComponentType);
	}

	Show_CurObject_ComponentInfos();

    for (auto pObject : *m_pObjects)
    {
        pObject->Tick(fTimeDelta);
    }

    ImGui::End();
}

void CObject_Tool::Select_ComponentType()
{
    if (ImGui::CollapsingHeader("Component Type ##ObjectTool"))
    {
        if (ImGui::RadioButton("Navigation ##ObjectTool", m_eComponentType == COMPONENT_TYPE::_NAVIGATION))
            m_eComponentType = COMPONENT_TYPE::_NAVIGATION;
        ImGui::SameLine();

        if (ImGui::RadioButton("Obstacle ##ObjectTool", m_eComponentType == COMPONENT_TYPE::_OBSTACLE))
            m_eComponentType = COMPONENT_TYPE::_OBSTACLE;
        ImGui::SameLine();

		if (ImGui::RadioButton("Model Static ##ObjectTool", m_eComponentType == COMPONENT_TYPE::_MODEL_STATIC))
			m_eComponentType = COMPONENT_TYPE::_MODEL_STATIC;
		ImGui::SameLine();

		if (ImGui::RadioButton("Model Anim ##ObjectTool", m_eComponentType == COMPONENT_TYPE::_MODEL_ANIM))
			m_eComponentType = COMPONENT_TYPE::_MODEL_ANIM;
		ImGui::SameLine();
    }

    ImGui::NewLine();
}

void CObject_Tool::Select_ObstacleType()
{
    if (COMPONENT_TYPE::_OBSTACLE != m_eComponentType)
        return;

	if (ImGui::CollapsingHeader("Obstacle Type ##ObjectTool"))
	{
		if (ImGui::RadioButton("Mesh ##ObjectTool", m_eObstacleType == OBSTACLE_TYPE::_MESH))
            m_eObstacleType = OBSTACLE_TYPE::_MESH;
		ImGui::SameLine();

		if (ImGui::RadioButton("Cube ##ObjectTool", m_eObstacleType == OBSTACLE_TYPE::_CUBE))
            m_eObstacleType = OBSTACLE_TYPE::_CUBE;
		ImGui::SameLine();

		if (ImGui::RadioButton("Cylinder ##ObjectTool", m_eObstacleType == OBSTACLE_TYPE::_CYLINDER))
            m_eObstacleType = OBSTACLE_TYPE::_CYLINDER;
		ImGui::SameLine();
	}

	ImGui::NewLine();
}

void CObject_Tool::Select_ObjectIndex()
{
    _int            iCurIndex = { max(*m_pCurObjectIdx, 0) };
    vector<const _char*>  ObjectsTags;
    
    string          strBase = { "Object_" };
    for (_uint i = 0; i < static_cast<_uint>(m_pObjects->size()); ++i)
    {
        string              strTag = { strBase + to_string(i + 1) };
        _char*              pObjectTag = { new _char[strTag.size() + 1] };
        strcpy_s(pObjectTag, strTag.size() + 1, strTag.c_str());
        ObjectsTags.push_back(pObjectTag);
    }

    if (false == m_pObjects->empty() &&
        ImGui::ListBox("Object List", &iCurIndex, &ObjectsTags.front(), static_cast<_int>(m_pObjects->size())))
    {
        *m_pCurObjectIdx = iCurIndex;
    }

    for (auto& pTag : ObjectsTags)
    {
        Safe_Delete_Array(pTag);
    }
}

void CObject_Tool::Show_CurObject_ComponentInfos()
{
    Show_NavigationInfo(*m_pCurObjectIdx);
    Show_ObstacleInfo(*m_pCurObjectIdx);
    Show_TransformInfo(*m_pCurObjectIdx);
}

void CObject_Tool::Show_NavigationInfo(const _int iObjectIdx)
{
    if (true == Is_OutOfRange_Objects(iObjectIdx))
        return;

    CNavigation*        pNavi = { static_cast<CNavigation*>((*m_pObjects)[iObjectIdx]->Get_Component(g_strNavigationTag)) };
    if (nullptr == pNavi)
        return;


    //`TODO: Show Info
}

void CObject_Tool::Show_ObstacleInfo(const _int iObjectIdx)
{
    if (true == Is_OutOfRange_Objects(iObjectIdx))
        return;

    CObstacle*          pObstacle = { static_cast<CObstacle*>((*m_pObjects)[iObjectIdx]->Get_Component(g_strObstacleTag)) };
    if (nullptr == pObstacle)
        return;

    //`TODO: Show Info

}

void CObject_Tool::Show_TransformInfo(const _int iObjectIdx)
{
    if (true == Is_OutOfRange_Objects(iObjectIdx))
        return;

    CTransform*         pTransform = { static_cast<CTransform*>((*m_pObjects)[iObjectIdx]->Get_Component(g_strTransformTag)) };
    if (nullptr == pTransform)
        return;

    _vector             vPosition = { pTransform->Get_State_Vector(CTransform::STATE::STATE_POSITION) };
    _float4x4           WorldFloat4x4 = { pTransform->Get_WorldFloat4x4() };

    ImGuizmo::BeginFrame();

    static ImGuizmo::OPERATION		CurrentGizmoOperation(ImGuizmo::ROTATE);
    static ImGuizmo::MODE			CurrentGizmoMode(ImGuizmo::WORLD);


    if (ImGui::RadioButton("Scale", CurrentGizmoOperation == ImGuizmo::SCALE))
        CurrentGizmoOperation = ImGuizmo::SCALE;
    ImGui::SameLine();

    if (ImGui::RadioButton("Rotation", CurrentGizmoOperation == ImGuizmo::ROTATE))
        CurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();

    if (ImGui::RadioButton(u8"Translation", CurrentGizmoOperation == ImGuizmo::TRANSLATE))
        CurrentGizmoOperation = ImGuizmo::TRANSLATE;


    _float3         vTranslation, vRotate, vScale;
    _bool			isChanged = { false };

    ImGuizmo::DecomposeMatrixToComponents((_float*)&WorldFloat4x4, (_float*)&vTranslation, (_float*)&vRotate, (_float*)&vScale);

    if (ImGui::InputFloat3("Scale", (_float*)&vScale))
        isChanged = true;
    if (ImGui::InputFloat3("Rotation", (_float*)&vRotate))
        isChanged = true;
    if (ImGui::InputFloat3("Translation", (_float*)&vTranslation))
        isChanged = true;

    ImGuizmo::RecomposeMatrixFromComponents((_float*)&vTranslation, (_float*)&vRotate, (_float*)&vScale, (_float*)&WorldFloat4x4);

    if (CurrentGizmoOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("LocalSpace", CurrentGizmoMode == ImGuizmo::LOCAL))
            CurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("WorldSpace", CurrentGizmoMode == ImGuizmo::WORLD))
            CurrentGizmoMode = ImGuizmo::WORLD;
    }

    static _bool isUseSnap = { false };
    ImGui::Checkbox("Grab", &isUseSnap);
    ImGui::SameLine();

    _float3 vSnap = { 0.f, 0.f, 0.f };
    switch (CurrentGizmoOperation)
    {
    case ImGuizmo::TRANSLATE:
        ImGui::InputFloat3("Translate Grab", &vSnap.x);
        break;
    case ImGuizmo::ROTATE:
        ImGui::InputFloat("Rotate Grab", &vSnap.x);
        break;
    case ImGuizmo::SCALE:
        ImGui::InputFloat("Scale Grab", &vSnap.x);
        break;
    }

    ImGuiIO& io = { ImGui::GetIO() };
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    _float4x4	ProjMatrix, ViewMatrix;

    ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);
    ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);

    ImGuizmo::Manipulate(&ViewMatrix.m[0][0], &ProjMatrix.m[0][0], CurrentGizmoOperation, CurrentGizmoMode, &WorldFloat4x4.m[0][0], NULL, isUseSnap ? &vSnap.x : NULL);

    pTransform->Set_WorldMatrix(WorldFloat4x4);
}

void CObject_Tool::Show_AnimationInfo(const _int iObjectIdx)
{
	if (true == Is_OutOfRange_Objects(iObjectIdx))
		return;

	CModel*         pModel = { static_cast<CModel*>((*m_pObjects)[iObjectIdx]->Get_Component(g_strObstacleTag)) };
	if (nullptr == pModel)
		return;

    const _uint     iNumAnims = { pModel->Get_NumAnims() };
    _bool           isLoop = { pModel->isLoop() };
}

HRESULT CObject_Tool::Create_Object()
{
    CDummy_Object*      pDummyObject = { CDummy_Object::Create(m_pDevice, m_pContext, nullptr) };
    if (nullptr == pDummyObject)
        return E_FAIL;

    m_pObjects->push_back(pDummyObject);
    for (auto& Components : *m_pComponents)
    {
        Components.push_back(nullptr);
    }

    (*m_pComponents)[static_cast<_uint>(COMPONENT_TYPE::_TRANSFORM)].back() = static_cast<CTransform*>(pDummyObject->Get_Component(g_strTransformTag));

    return S_OK;
}

HRESULT CObject_Tool::Erase_Objest(const _int iObjectIdx)
{
    if (true == Is_OutOfRange_Objects(iObjectIdx))
        return E_FAIL;

    auto iterObject{ m_pObjects->begin() + iObjectIdx };

    Safe_Release(*iterObject);
    *iterObject = nullptr;
    m_pObjects->erase(iterObject);

    for (auto& Components : *m_pComponents)
    {
        auto iterComponent{ Components.begin() + iObjectIdx };
        *iterComponent = nullptr;
        Components.erase(iterComponent);
    }

    *m_pCurObjectIdx = -1;

    return S_OK;
}

HRESULT CObject_Tool::Add_Component_To_Object(const _int iObjectIdx, const COMPONENT_TYPE eComoponnent)
{
    if(true == Is_OutOfRange_Objects(iObjectIdx))
        return E_FAIL;

    CDummy_Object*          pCurObject = { (*m_pObjects)[iObjectIdx] };
    CComponent*             pComponent = { nullptr };
    wstring                 strComponentTag = { TEXT("") };

    switch (eComoponnent)
    {
    case COMPONENT_TYPE::_NAVIGATION:
    {
        CNavigation::NAVIGATION_DESC            NaviDesc;
        NaviDesc.pTransform = static_cast<CTransform*>(pCurObject->Get_Component(g_strTransformTag));

        pComponent = CNavigation::Create(m_pDevice, m_pContext);
        if (nullptr == pComponent)
            return E_FAIL;
        if (FAILED(pComponent->Initialize(&NaviDesc)))
            return E_FAIL;

        strComponentTag = g_strNavigationTag;
        break;
    }
	case COMPONENT_TYPE::_OBSTACLE:
	{
        switch (m_eObstacleType)
        {
        case Engine::OBSTACLE_TYPE::_MESH:
        {
            CComponent* pComModel;
            pComModel = (*m_pComponents)[static_cast<_uint>(COMPONENT_TYPE::_MODEL_STATIC)][iObjectIdx];
            if (nullptr == pComModel)
                return E_FAIL;

            CObstacle_Mesh::OBSTACLE_DESC           ObstacleDesc;
            ObstacleDesc.pTransform = static_cast<CTransform*>(pCurObject->Get_Component(g_strTransformTag));

            //  pComponent = CObstacle_Cube::Create(m_pDevice, m_pContext);
            pComponent = CObstacle_Mesh::Create(m_pDevice, m_pContext, static_cast<CModel*>(pComModel));
            if (nullptr == pComponent)
                return E_FAIL;
            if (FAILED(pComponent->Initialize(&ObstacleDesc)))
                return E_FAIL;

            strComponentTag = g_strObstacleTag;
            break;
        }
        case Engine::OBSTACLE_TYPE::_CUBE:
        {
            CObstacle_Cube::OBSTACLE_DESC           ObstacleDesc;
            ObstacleDesc.pTransform = static_cast<CTransform*>(pCurObject->Get_Component(g_strTransformTag));

            //  pComponent = CObstacle_Cube::Create(m_pDevice, m_pContext);
            pComponent = CObstacle_Cube::Create(m_pDevice, m_pContext);
            if (nullptr == pComponent)
                return E_FAIL;
            if (FAILED(pComponent->Initialize(&ObstacleDesc)))
                return E_FAIL;

            strComponentTag = g_strObstacleTag;
            break;
        }
        case Engine::OBSTACLE_TYPE::_CYLINDER:
        {
            CObstacle_Cylinder::OBSTACLE_DESC           ObstacleDesc;
            ObstacleDesc.pTransform = static_cast<CTransform*>(pCurObject->Get_Component(g_strTransformTag));

            //  pComponent = CObstacle_Cube::Create(m_pDevice, m_pContext);
            pComponent = CObstacle_Cylinder::Create(m_pDevice, m_pContext);
            if (nullptr == pComponent)
                return E_FAIL;
            if (FAILED(pComponent->Initialize(&ObstacleDesc)))
                return E_FAIL;

            strComponentTag = g_strObstacleTag;
            break;
        }
        }
		break;
	}
	case COMPONENT_TYPE::_MODEL_STATIC:
	{
		pComponent = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements);
		if (nullptr == pComponent)
			return E_FAIL;
		if (FAILED(pComponent->Initialize(nullptr)))
			return E_FAIL;

		strComponentTag = g_strShaderModelTag;
		if (FAILED(pCurObject->Add_Component(pComponent, strComponentTag)))
			return E_FAIL;

        pComponent = m_pPrototypeModel->Clone(nullptr);
        
		if (nullptr == pComponent)
			return E_FAIL;
		if (FAILED(pComponent->Initialize(nullptr)))
			return E_FAIL;

		strComponentTag = g_strModelTag;
		break;
	}
	case COMPONENT_TYPE::_MODEL_ANIM:
	{
		pComponent = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimModel.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements);
		if (nullptr == pComponent)
			return E_FAIL;
		if (FAILED(pComponent->Initialize(nullptr)))
			return E_FAIL;

		strComponentTag = g_strShaderAnimModelTag;
		if (FAILED(pCurObject->Add_Component(pComponent, strComponentTag)))
			return E_FAIL;

		_matrix			TransformMatrix = { XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.0f)) };
		//  pComponent = CModel::Create(m_pDevice, m_pContext, CModel::TYPE_ANIM, "../Bin/Resources/Models/Fiona/Fiona.fbx", TransformMatrix);
		pComponent = CModel::Create(m_pDevice, m_pContext, CModel::TYPE_ANIM, "../Bin/Resources/Models/Remy/Remy.fbx", TransformMatrix);
		//  pComponent = CModel::Create(m_pDevice, m_pContext, CModel::TYPE_ANIM, "../Bin/Resources/Models/Thresher_Shark/Thresher_Shark.fbx", TransformMatrix);

		if (nullptr == pComponent)
			return E_FAIL;
		if (FAILED(pComponent->Initialize(nullptr)))
			return E_FAIL;

        static_cast<CModel*>(pComponent)->Set_Animation(0, true);

		strComponentTag = g_strAnimModelTag;
		break;
	}
    default:

        break;
    }

    if (FAILED(pCurObject->Add_Component(pComponent, strComponentTag)))
        return E_FAIL;

    (*m_pComponents)[static_cast<_uint>(eComoponnent)][iObjectIdx] = pComponent;

    return S_OK;
}

_bool CObject_Tool::Is_OutOfRange_Objects(const _int iObjectIdx)
{
    return (0 > iObjectIdx) || (static_cast<_int>(m_pObjects->size()) <= iObjectIdx);
}

HRESULT CObject_Tool::Add_Components()
{
	_matrix			TransformMatrix = { XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.0f)) };
    m_pPrototypeModel = CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, "../Bin/Resources/Models/ForkLift/ForkLift.fbx", TransformMatrix);

    if (nullptr == m_pPrototypeModel)
        return E_FAIL;

    return S_OK;
}

CObject_Tool* CObject_Tool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
    CObject_Tool*       pInstance = new CObject_Tool(pDevice, pContext);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Created : CObject_Tool"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

void CObject_Tool::Free()
{
    __super::Free();

    for (auto& pObject : (*m_pObjects))
    {
        Safe_Release(pObject);
        pObject = nullptr;
    }

    m_pObjects->clear();

    Safe_Release(m_pPrototypeModel);
}

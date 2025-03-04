#include "Path_Rounder.h"
#include "GameInstance.h"

CPath_Rounder::CPath_Rounder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:m_pDevice{pDevice}
	,m_pContext{pContext}
	,m_pGameInstance{CGameInstance::Get_Instance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPath_Rounder::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CPath_Rounder::Render()
{
	return S_OK;
}

void CPath_Rounder::Compute_Smooth_Path(const list<_float3>& PathList)
{
	if (PathList.empty())
		return;

	vector<_float3>		PathPoses, SmoothPositions;
	for (const auto& vPosition : PathList)
		PathPoses.push_back(vPosition);
	

	_uint		iCurIndex = {};
	_vector		vCurPos = { XMLoadFloat3(&PathPoses[iCurIndex]) };
	_vector		vDir = {};

	vector<_float3>			SmoothPathTemp;
	for (_uint i = 1; i < PathPoses.size(); ++i)
	{

		if (SmoothPositions.empty())
		{
			vDir = XMLoadFloat3(&PathPoses[i]) - XMLoadFloat3(&PathPoses[i - 1]);
			SmoothPathTemp = Make_SmoothPath(XMLoadFloat3(&PathPoses[i - 1]), XMLoadFloat3(&PathPoses[i]), vDir, 0.25f, m_iNumSample);
		}
		else
		{
			SmoothPositions.pop_back();
			vDir = XMLoadFloat3(&PathPoses[i - 1]) - XMLoadFloat3(&SmoothPositions.back());
			SmoothPathTemp = Make_SmoothPath(XMLoadFloat3(&PathPoses[i - 1]), XMLoadFloat3(&PathPoses[i]), vDir, 0.25f, m_iNumSample);
		}

		for (auto& vPos : SmoothPathTemp)
			SmoothPositions.push_back(vPos);
	}

	m_SmoothPath = SmoothPositions;
}

void CPath_Rounder::Compute_Smooth_Path_Bazier(const list<_float3>& PathList)
{
	if (PathList.empty())
		return;

	vector<_float3>		PathPoses, SmoothPositions;
	for (const auto& vPosition : PathList)
		PathPoses.push_back(vPosition);


	_uint		iCurIndex = {};
	_vector		vCurPos = { XMLoadFloat3(&PathPoses[iCurIndex]) };
	_vector		vDir = {};

	vector<_float3>			SmoothPathTemp;
	for (_uint i = 1; i < PathPoses.size(); ++i)
	{
		_float		fControlLength = { XMVectorGetX(XMVector3Length(XMLoadFloat3(&PathPoses[i]) - XMLoadFloat3(&PathPoses[i - 1]))) * 0.5f };
		_float		fRemainLength = { fControlLength * 0.1f };

		if (SmoothPositions.empty())
		{

			while (0.f < fControlLength)
			{
				_bool		isFailed = {};

				vDir = XMLoadFloat3(&PathPoses[i]) - XMLoadFloat3(&PathPoses[i - 1]);
				vDir = XMVector3Normalize(vDir) * fControlLength;
				_vector			vControll = { vDir + XMLoadFloat3(&PathPoses[i - 1]) };
				SmoothPathTemp = Make_SmoothPath_Bezier(XMLoadFloat3(&PathPoses[i - 1]), XMLoadFloat3(&PathPoses[i]), vControll, m_iNumSample);

				for (_uint iSmoothIndex = 0; iSmoothIndex < SmoothPathTemp.size() - 1; ++iSmoothIndex)
				{
					_uint		iStartIndex, iEndIndex;
					if (FAILED(m_pGameInstance->Get_Index_Voxel(SmoothPathTemp[iSmoothIndex], iStartIndex)) ||
						FAILED(m_pGameInstance->Get_Index_Voxel(SmoothPathTemp[iSmoothIndex + 1], iEndIndex)) ||
						false == m_pGameInstance->Is_Reach_TargetIndex(iStartIndex, iEndIndex, VOXEL_LAYER::_STATIC))
					{
						isFailed = true;
						break;
					}
				}

				//	¼º°ø½Ã
			if (false == isFailed)
					break;

				fControlLength -= fRemainLength;
				SmoothPathTemp.clear();
			}
			
		}
		else
		{
			SmoothPositions.pop_back();

			while (0.f < fControlLength)
			{
				_bool		isFailed = {};

				vDir = XMLoadFloat3(&PathPoses[i - 1]) - XMLoadFloat3(&SmoothPositions.back());
				vDir = XMVector3Normalize(vDir) * fControlLength;
				_vector			vControll = { vDir + XMLoadFloat3(&PathPoses[i - 1]) };
				SmoothPathTemp = Make_SmoothPath_Bezier(XMLoadFloat3(&PathPoses[i - 1]), XMLoadFloat3(&PathPoses[i]), vControll, m_iNumSample);

				for (_uint iSmoothIndex = 0; iSmoothIndex < SmoothPathTemp.size() - 1; ++iSmoothIndex)
				{
					_uint		iStartIndex, iEndIndex;
					if (FAILED(m_pGameInstance->Get_Index_Voxel(SmoothPathTemp[iSmoothIndex], iStartIndex)) ||
						FAILED(m_pGameInstance->Get_Index_Voxel(SmoothPathTemp[iSmoothIndex + 1], iEndIndex)) ||
						false == m_pGameInstance->Is_Reach_TargetIndex(iStartIndex, iEndIndex, VOXEL_LAYER::_STATIC))
					{
						isFailed = true;
						break;
					}
				}

				if (false == isFailed)
					break;

				fControlLength -= fRemainLength;
				SmoothPathTemp.clear();
			}
		}

		for (auto& vPos : SmoothPathTemp)
			SmoothPositions.push_back(vPos);
	}

	SmoothPositions.push_back(PathPoses.back());

	m_SmoothPath = SmoothPositions;
}

vector<_float3> CPath_Rounder::Make_SmoothPath(_fvector vStart, _fvector vEnd, _fvector vDir, _float fRadius, _uint iNumSample)
{
	vector<_float3> SmoothPath;

	_vector			vDirToEnd = { vEnd - vStart };
	_float			fDot = { XMVectorGetX(XMVector3Dot(XMVector3Normalize(vDirToEnd), XMVector3Normalize(vDir))) };

	_float3				vTemp;
	XMStoreFloat3(&vTemp, vStart);
	SmoothPath.push_back(vTemp);

	if (fDot <= 1.f - EPSILON)
	{
		_vector			vAxis = { XMVector3Normalize(XMVector3Cross(XMVector3Normalize(vDir), XMVector3Normalize(vDirToEnd))) };
		_vector			vToCircleCenterDir = { XMVector3Normalize(XMVector3Cross(XMVector3Normalize(vAxis), XMVector3Normalize(vDir))) };
		_vector			vCircleCenterPos = { vStart + (vToCircleCenterDir * fRadius) };

		//	//	_float			fDist = { XMVectorGetX(XMVector3Length(vDirToEnd)) };
		_vector			vNextPos = { vStart };
		_vector			vDirToNext = { vNextPos - vCircleCenterPos };
		_float			fDot = { XMVectorGetX(XMVector3Dot(XMVector3Normalize(vDirToNext), XMVector3Normalize(vEnd - vNextPos))) };
		_float			fAngle = { 0.f };
		_float			fAnlgeStep = { XMConvertToRadians(5.f) };
		_matrix			RotateMatrix = { XMMatrixRotationAxis(vAxis, fAnlgeStep) };
		_matrix			RotateMatrix2 = { XMMatrixRotationAxis(vAxis, XMConvertToRadians(90.f)) };

		_float			fPreDot = { fDot };

		while (true)
		{
			vDirToNext = XMVector3TransformNormal(vDirToNext, RotateMatrix);
			vNextPos = vCircleCenterPos + vDirToNext;

			_vector			vCurDir = { XMVector3TransformNormal(vDirToNext, RotateMatrix2) };

			fPreDot = fDot;
			fDot = { XMVectorGetX(XMVector3Dot(XMVector3Normalize(vCurDir), XMVector3Normalize(vEnd - vNextPos))) };

			if (fPreDot > fDot)
				break;

			XMStoreFloat3(&vTemp, vNextPos);
			SmoothPath.push_back(vTemp);
		}
	}

	XMStoreFloat3(&vTemp, vEnd);
	SmoothPath.push_back(vTemp);

	return SmoothPath;
}

vector<_float3> CPath_Rounder::Make_SmoothPath_Bezier(const _uint3& vStartIndexPos, const _uint3& vEndIndexPos)
{
	vector<_float3>			SmoothPoses;
	_uint			iDistX = { vStartIndexPos.x - vEndIndexPos.x };
	_uint			iDistY = { vStartIndexPos.y - vEndIndexPos.y };
	_uint			iDistZ = { vStartIndexPos.z - vEndIndexPos.z };


	_float3			vStartWorldPos, vEndWorldPos;
	_uint			iStartIndex, iEndIndex;

	if (FAILED(m_pGameInstance->Get_Index_Voxel(vStartIndexPos, iStartIndex)) ||
		FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iStartIndex, vStartWorldPos)) ||
		FAILED(m_pGameInstance->Get_Index_Voxel(vEndIndexPos, iEndIndex)) ||
		FAILED(m_pGameInstance->Get_WorldPosition_Voxel(iEndIndex, vEndWorldPos)))
		return SmoothPoses;

	/////////////////////////////////
	/////////////////////////////////
	/////////////////////////////////
	/////////////////////////////////

	return SmoothPoses;
}

vector<_float3> CPath_Rounder::Make_SmoothPath_Bezier(_fvector vStart, _fvector vEnd, _fvector vControllPos, _uint iNumSample)
{
	vector<_float3>			SmoothPoses;

	_float					fRatio = {};
	_float					fRatioStep = { 1.f / static_cast<_float>(iNumSample) };

	for (_uint iSampleCnt = 0; iSampleCnt < iNumSample; ++iSampleCnt)
	{
		fRatio = fRatioStep * static_cast<_float>(iSampleCnt);

		_vector			vFirst = { XMVectorLerp(vStart, vControllPos, fRatio) };
		_vector			vSecond = { XMVectorLerp(vControllPos, vEnd, fRatio) };
		_vector			vResult = { XMVectorLerp(vFirst, vSecond, fRatio) };

		_float3			vTemp = {};
		XMStoreFloat3(&vTemp, vResult);

		SmoothPoses.push_back(vTemp);
	}

	return SmoothPoses;
}

CPath_Rounder* CPath_Rounder::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CPath_Rounder*		pInstance = { new CPath_Rounder(pDevice, pContext) };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CPath_Rounder"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPath_Rounder::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}

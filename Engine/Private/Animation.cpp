#include "..\Public\Animation.h"
#include "Channel.h"
#include "Bone.h"

CAnimation::CAnimation()
{

}

CAnimation::CAnimation(const CAnimation & rhs)
	: m_fDuration{ rhs.m_fDuration }
	, m_fTickPerSecond{ rhs.m_fTickPerSecond }
	, m_fTrackPosition{ rhs.m_fTrackPosition }
	, m_iNumChannels{ rhs.m_iNumChannels }
	, m_Channels{ rhs.m_Channels }
	, m_CurrentKeyFrameIndices{ rhs.m_CurrentKeyFrameIndices }
	, m_isFinished{ rhs.m_isFinished }

{
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);
}

HRESULT CAnimation::Initialize(const aiAnimation * pAIAnimation, const vector<CBone*>& Bones)
{
	strcpy_s(m_szName, pAIAnimation->mName.data);

	m_fDuration = static_cast<_float>(pAIAnimation->mDuration);

	m_fTickPerSecond = static_cast<_float>(pAIAnimation->mTicksPerSecond);	

	/* �� �ִϸ��̼��� ��� ���� ��Ʈ���ؾ��ϴ°�? */
	m_iNumChannels = pAIAnimation->mNumChannels;

	m_CurrentKeyFrameIndices.resize(m_iNumChannels);

	for (size_t i = 0; i < m_iNumChannels; i++)
	{
		CChannel*		pChannel = CChannel::Create(pAIAnimation->mChannels[i], Bones);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}

	return S_OK;
}

void CAnimation::Invalidate_TransformationMatrix(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop)
{
	m_isFinished = false;

	m_fTrackPosition += m_fTickPerSecond * fTimeDelta;

	if (m_fDuration <= m_fTrackPosition)
	{
		if (false == isLoop)
		{
			m_isFinished = true;			
			return;
		}			

		m_fTrackPosition = 0.f;
	}

	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		/* �� ���� ��������� ���� CBone�� TransformationMatrix�� �ٲ��. */
		m_Channels[i]->Invalidate_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);
	}

}

CAnimation * CAnimation::Create(const aiAnimation * pAIAnimation, const vector<CBone*>& Bones)
{
	CAnimation*		pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(pAIAnimation, Bones)))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimation"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation * CAnimation::Clone()
{
	return new CAnimation(*this);
}


void CAnimation::Free()
{
	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);

	m_Channels.clear();
}


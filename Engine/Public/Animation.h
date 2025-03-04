#pragma once

#include "Base.h"

/* �ϳ��� �ִϸ��̼��� ��� ���� ��Ʈ���ؾ��ϴ���. �� ������ ��������.  */
/* �� �ִϸ��̼��� ����ϴµ� �ɸ��� �� �Ÿ�.. */
/* �� �ִϸ��̼��� �����ϴ� �ӵ�. */

BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& rhs);
	virtual ~CAnimation() = default;

public:
	_bool isFinished() const {
		return m_isFinished;
	}

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	void Invalidate_TransformationMatrix(_float fTimeDelta, const vector<class CBone*>& Bones, _bool isLoop);

private:
	_char								m_szName[MAX_PATH] = { "" };
	
	_float								m_fDuration = { 0.0f };		 /* ��ü ��� ����. */
	_float								m_fTickPerSecond = { 0.0f }; /* �ʴ� �󸶳� ����� �ؾ��ϴ°� (�ӵ�) */
	_float								m_fTrackPosition = { 0.0f }; /* ���� �ִϸ��̼��� ������ ����Ǿ�����"? */

	_uint								m_iNumChannels = { 0 };
	vector<class CChannel*>				m_Channels;
	vector<_uint>						m_CurrentKeyFrameIndices;

	_bool								m_isFinished = { false };	

public:
	static CAnimation* Create(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	CAnimation* Clone();
	virtual void Free() override;
};

END
#pragma once

#include "Base.h"

/* Ư�� �ִϸ��̼��� ����ϰ� �ִ� ���� ��, �ϳ��� ������ ǥ���Ѵ�.. */
/* �� ���� �ð��� ���� � ����(KeyFrame)�� ���ؾ��ϴ°�? */

BEGIN(Engine)

class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	void Invalidate_TransformationMatrix(const vector<class CBone*>& Bones, _float fTrackPosition, _uint* pCurrentKeyFrameIndex);

private:
	_char				m_szName[MAX_PATH] = { "" };
	_int				m_iBoneIndex = { -1 };
	_uint				m_iNumKeyFrames = { 0 };
	vector<KEYFRAME>	m_KeyFrames;

	
	

public:
	static CChannel* Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	virtual void Free() override;
};

END